#!/usr/bin/env python3
"""Select 512 substantial Image shaders that compile at O0 and O2.

The full deterministic candidate screen is retained, including failures.
"""

import argparse
import hashlib
import importlib.util
import json
import re
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
CORPUS = HERE / "complex"
UPSTREAM = CORPUS / "upstream"
PORTS = CORPUS / "ports"
SCREEN = CORPUS / "screening.jsonl"
MANIFEST = UPSTREAM / "manifest.json"
MIN_SOURCE_CHARS = 3000
MIN_FUNCTIONS = 3
MIN_LOOPS = 1
TARGET = 512
EXCLUDED_LICENSES = {"proprietary-license", "unknown-license-reference"}
FUNCTION = re.compile(
    r"\b(?:void|float|vec[234]|mat[234]|int|bool)\s+\w+\s*\([^;{}]*\)\s*\{"
)
LOOP = re.compile(r"\b(?:for|while)\s*\(")
OTHER_PASSES = (
    "buffer_a_code", "buffer_b_code", "buffer_c_code", "buffer_d_code",
    "sound_code", "cube_a_code",
)


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def candidates(dataset_dir):
    old_manifest = json.loads((HERE / "upstream/manifest.json").read_text())
    old_ids = {item["id"] for item in old_manifest["files"]}
    dataset_files = sorted(dataset_dir.glob("api_2022-*.jsonl")) + sorted(
        dataset_dir.glob("api_2023-*.jsonl"))
    if len(dataset_files) != 24:
        raise ValueError("Expected all 24 pinned 2022-2023 dataset files")
    first = {}
    sources = []
    for path in dataset_files:
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        sources.append({"file": path.name, "sha256": digest})
        for line in path.read_text().splitlines():
            record = json.loads(line)
            shader_id = record.get("id")
            if shader_id in old_ids or shader_id in first:
                continue
            if not record.get("image_code") or any(
                record.get(name) for name in OTHER_PASSES
            ):
                continue
            if record.get("license") in EXCLUDED_LICENSES:
                continue
            source = (record.get("common_code") or "") + "\n" + record["image_code"]
            functions = len(FUNCTION.findall(source))
            loops = len(LOOP.findall(source))
            if (len(source) < MIN_SOURCE_CHARS or functions < MIN_FUNCTIONS
                    or loops < MIN_LOOPS):
                continue
            first[shader_id] = {
                "record": record,
                "dataset_file": path.name,
                "source_chars": len(source),
                "functions": functions,
                "loops": loops,
                "code_sha256": hashlib.sha256(source.encode()).hexdigest(),
            }
    unique = {}
    for shader_id in sorted(first):
        item = first[shader_id]
        unique.setdefault(item["code_sha256"], item)
    # Screening shorter candidates first avoids spending the entire run on the
    # 40-100 KB tails. Final selection still prefers the longest passes.
    ranked = sorted(unique.values(), key=lambda x: (x["source_chars"],
                                                    x["record"]["id"]))
    return ranked, sources


def snapshot(item):
    record = item["record"]
    shader_id = record["id"]
    filename = shader_id + "-" + shader_id.encode("ascii").hex() + ".json"
    legacy = UPSTREAM / (shader_id + ".json")
    if legacy.is_file() and json.loads(legacy.read_text()).get("id") == shader_id:
        filename = legacy.name
    data = (json.dumps(record, indent=2, ensure_ascii=False) + "\n").encode()
    (UPSTREAM / filename).write_bytes(data)
    return {
        "id": shader_id,
        "name": record.get("name", ""),
        "author": record.get("author", ""),
        "license": record.get("license", ""),
        "file": filename,
        "sha256": hashlib.sha256(data).hexdigest(),
        "url": f"https://www.shadertoy.com/view/{shader_id}",
        "dataset_file": item["dataset_file"],
        "source_chars": item["source_chars"],
        "functions": item["functions"],
        "loops": item["loops"],
        "code_sha256": item["code_sha256"],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dataset-dir", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--batch-size", type=int, default=32)
    parser.add_argument("--max-candidates", type=int, default=0)
    args = parser.parse_args()
    if args.jobs < 1 or args.batch_size < 1:
        raise ValueError("Jobs and batch size must be positive")
    ranked, sources = candidates(args.dataset_dir)
    print(f"Eligible complex shaders: {len(ranked)}", flush=True)
    UPSTREAM.mkdir(parents=True, exist_ok=True)
    PORTS.mkdir(parents=True, exist_ok=True)
    portlib = load_module("shadertoy_port", HERE / "port.py")
    testlib = load_module("shadertoy_test", HERE / "test.py")
    portlib.HERE = CORPUS
    testlib.HERE = CORPUS
    testlib.ARTIFACTS = ROOT / "shader-ports/artifacts/shadertoy_complex_screen"
    testlib.COMPILE_TIMEOUT = 15
    testlib.ARTIFACTS.mkdir(parents=True, exist_ok=True)
    previous = {}
    if SCREEN.exists():
        for line in SCREEN.read_text().splitlines():
            result = json.loads(line)
            previous[result["id"]] = result
    if previous:
        print(f"Resuming {len(previous)} screened shaders", flush=True)
    limit = args.max_candidates or len(ranked)
    for start in range(0, min(limit, len(ranked)), args.batch_size):
        batch = ranked[start:start + args.batch_size]
        pending = []
        translated = []
        for item in batch:
            shader_id = item["record"]["id"]
            if shader_id in previous:
                continue
            entry = snapshot(item)
            pending.append((item, entry))
            try:
                portlib.port(entry, PORTS)
                translated.append(entry)
            except Exception as error:
                result = {**entry, "status": "translation_failed",
                          "diagnostic": str(error)[:2000]}
                previous[shader_id] = result
                with SCREEN.open("a") as out:
                    out.write(json.dumps(result) + "\n")
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            o2 = dict(zip((item["id"] for item in translated),
                          pool.map(testlib.attempt,
                                   ((item, "-O2") for item in translated))))
            o0_items = [item for item in translated
                        if o2[item["id"]]["status"] == "pass"]
            o0 = dict(zip((item["id"] for item in o0_items),
                          pool.map(testlib.attempt,
                                   ((item, "-O0") for item in o0_items))))
        for _, entry in pending:
            shader_id = entry["id"]
            if shader_id in previous:
                continue
            result = {**entry, "status": "pass" if
                      shader_id in o0 and o0[shader_id]["status"] == "pass"
                      else "failed",
                      "O2": o2[shader_id]["status"],
                      "O0": o0[shader_id]["status"] if shader_id in o0 else
                      "not_attempted"}
            for opt, results in (("O2", o2), ("O0", o0)):
                if shader_id in results and results[shader_id]["status"] != "pass":
                    result[f"{opt}_diagnostic"] = results[shader_id].get(
                        "diagnostic", "")[:2000]
            previous[shader_id] = result
            with SCREEN.open("a") as out:
                out.write(json.dumps(result) + "\n")
        passed = sum(previous.get(item["record"]["id"], {}).get("status") ==
                     "pass" for item in ranked)
        print(f"Screened {len(previous)}; eligible pass both: {passed}", flush=True)
        if passed >= TARGET:
            break
    selected = [item for item in ranked
                if previous.get(item["record"]["id"], {}).get("status") == "pass"]
    selected.sort(key=lambda x: (-x["source_chars"], x["record"]["id"]))
    selected = selected[:TARGET]
    if len(selected) < TARGET:
        print(f"Only {len(selected)} passed both; retain screening state", flush=True)
        return 1
    entries = [snapshot(item) for item in selected]
    selected_stems = {Path(item["file"]).stem for item in entries}
    for path in UPSTREAM.glob("*.json"):
        if path.name != "manifest.json" and path.stem not in selected_stems:
            path.unlink()
    for path in PORTS.glob("*.cpp"):
        if path.stem not in selected_stems:
            path.unlink()
    manifest = {
        "dataset_repository": "https://github.com/Vipitis/shadertoys-dataset",
        "commit": "7a5d34083b03501a4d42238bbe65183b0b531f00",
        "dataset_files": sources,
        "selection": {
            "description": "Longest distinct eligible screened Image shaders passing O2 then O0",
            "excluded_existing_ids": 256,
            "min_source_chars": MIN_SOURCE_CHARS,
            "min_function_definitions": MIN_FUNCTIONS,
            "min_loop_constructs": MIN_LOOPS,
            "no_other_passes": True,
            "target": TARGET,
            "excluded_license_values": sorted(EXCLUDED_LICENSES),
            "screened": len(previous),
            "passed_both": sum(x["status"] == "pass" for x in previous.values()),
            "eligible_passed_both": sum(previous.get(
                item["record"]["id"], {}).get("status") == "pass"
                for item in ranked),
        },
        "files": entries,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Selected {len(entries)}; manifest: {MANIFEST}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
