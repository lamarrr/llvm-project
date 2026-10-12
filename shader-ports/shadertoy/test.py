#!/usr/bin/env python3
"""Attempt all pinned Shadertoy ports at O0/O1/O2/O3; retain every failure."""

import argparse
import collections
import hashlib
import json
import os
import re
import signal
import struct
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
HERE = SCRIPT_DIR
ROOT = SCRIPT_DIR.parents[1]
BIN = ROOT / "build/bin"
ARTIFACTS = HERE.parent / "artifacts/shadertoy"
COMPILE_TIMEOUT = 45
OPTIMIZATIONS = ("-O0", "-O1", "-O2", "-O3")


def compile_bounded(command):
    # Terminate the whole compiler/linker tree if a shader exceeds the limit.
    with subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        start_new_session=True,
    ) as process:
        try:
            stdout, stderr = process.communicate(timeout=COMPILE_TIMEOUT)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.communicate()
            raise
        return subprocess.CompletedProcess(command, process.returncode, stdout, stderr)


def attempt(task):
    item, opt = task
    result = {"id": item["id"], "optimization": opt, "status": "compile_failed"}
    source_stem = Path(item["file"]).stem
    source = HERE / "ports" / (source_stem + ".cpp")
    stem = ARTIFACTS / (source_stem + "-" + opt[1:])
    binary = stem.with_suffix(".spv")
    for p in [
        binary,
        Path(str(binary) + ".json"),
        stem.with_suffix(".spvasm"),
        stem.with_suffix(".log"),
    ]:
        p.unlink(missing_ok=True)
    command = [
        str(BIN / "clang++"),
        "-std=c++20",
        "-fspirv-shader",
        "-fspirv-entry=shadertoy",
        opt,
        "-fno-crash-diagnostics",
        str(source),
        "-o",
        str(binary),
    ]
    result["command"] = command
    result["source_sha256"] = hashlib.sha256(source.read_bytes()).hexdigest()
    try:
        built = compile_bounded(command)
        stem.with_suffix(".log").write_text(built.stdout + built.stderr)
        if built.returncode:
            result["diagnostic"] = built.stderr[:16000]
            result["crashed"] = (
                built.returncode < 0
                or "PLEASE submit" in built.stderr
                or "UNREACHABLE" in built.stderr
            )
            return result
        result["status"] = "validation_failed"
        if struct.unpack("<II", binary.read_bytes()[:8]) != (0x07230203, 0x00010400):
            raise ValueError("Wrong SPIR-V version")
        subprocess.run(
            [
                str(BIN / "spirv-val"),
                "--target-env",
                "vulkan1.3",
                "--scalar-block-layout",
                str(binary),
            ],
            check=True,
            capture_output=True,
            text=True,
            timeout=30,
        )
        asm = subprocess.run(
            [str(BIN / "spirv-dis"), str(binary)],
            check=True,
            capture_output=True,
            text=True,
            timeout=30,
        ).stdout
        stem.with_suffix(".spvasm").write_text(asm)
        reflection = json.loads(Path(str(binary) + ".json").read_text())
        if (
            reflection["stage"] != "fragment"
            or reflection["source_entry"] != "shadertoy"
            or not reflection["entry"].startswith("_Z")
        ):
            raise ValueError("Incorrect entry/stage/linkage")
        if "OpFunctionCall" in asm:
            raise ValueError("Uninlined helper")
        output = [i for i in reflection["interfaces"] if i["kind"] == "output"]
        if (
            len(output) != 1
            or output[0].get("location") != 0
            or output[0]["size"] != 16
        ):
            raise ValueError("Incorrect color output")
        pushes = [i for i in reflection["interfaces"] if i["kind"] == "push_constant"]
        if pushes and pushes[0]["size"] != 128:
            raise ValueError("Incorrect Shadertoy uniform layout")
        result.update(
            status="pass",
            entry=reflection["entry"],
            bytes=binary.stat().st_size,
            image_operations=dict(
                collections.Counter(re.findall(r"\b(OpImage\w+)\b", asm))
            ),
            required_features=reflection["required_features"],
        )
    except subprocess.TimeoutExpired:
        result.update(status="timeout", diagnostic="Compilation or validation timeout")
        stem.with_suffix(".log").write_text(result["diagnostic"] + "\n")
    except Exception as e:
        result["diagnostic"] = str(e)
    return result


def main():
    global HERE, ARTIFACTS
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--corpus-dir", type=Path, default=SCRIPT_DIR)
    parser.add_argument("--expected-count", type=int, default=256)
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Fail if any port does not pass every optimization level",
    )
    args = parser.parse_args()
    HERE = args.corpus_dir.resolve()
    artifact_name = "shadertoy" if HERE == SCRIPT_DIR else f"shadertoy_{HERE.name}"
    ARTIFACTS = ROOT / "shader-ports/artifacts" / artifact_name
    if args.jobs < 1:
        raise ValueError("--jobs must be positive")
    ARTIFACTS.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((HERE / "upstream/manifest.json").read_text())
    stems = [Path(item["file"]).stem.casefold() for item in manifest["files"]]
    if len(stems) != len(set(stems)):
        raise ValueError("Shader filenames must be unique ignoring case")
    if (
        len(manifest["files"]) != args.expected_count
        or len({i["id"] for i in manifest["files"]}) != args.expected_count
    ):
        raise ValueError(f"Expected {args.expected_count} distinct shaders")
    for item in manifest["files"]:
        if (
            hashlib.sha256((HERE / "upstream" / item["file"]).read_bytes()).hexdigest()
            != item["sha256"]
        ):
            raise ValueError("Original shader changed: " + item["id"])
    if isinstance(manifest.get("selection"), dict):
        old = json.loads((SCRIPT_DIR / "upstream/manifest.json").read_text())
        old_ids = {item["id"] for item in old["files"]}
        old_code = set()
        for item in old["files"]:
            record = json.loads((SCRIPT_DIR / "upstream" / item["file"]).read_text())
            source = (record.get("common_code") or "") + "\n" + record["image_code"]
            old_code.add(hashlib.sha256(source.encode()).hexdigest())
        if old_ids.intersection(item["id"] for item in manifest["files"]):
            raise ValueError("Complex corpus reuses an original shader ID")
        if old_code.intersection(item["code_sha256"] for item in manifest["files"]):
            raise ValueError("Complex corpus reuses original shader code")
        if len({item["code_sha256"] for item in manifest["files"]}) != args.expected_count:
            raise ValueError("Complex corpus contains duplicate shader code")
        selection = manifest["selection"]
        dataset_files = {item["file"] for item in manifest["dataset_files"]}
        for item in manifest["files"]:
            record = json.loads((HERE / "upstream" / item["file"]).read_text())
            source = (record.get("common_code") or "") + "\n" + record["image_code"]
            functions = len(re.findall(
                r"\b(?:void|float|vec[234]|mat[234]|int|bool)"
                r"\s+\w+\s*\([^;{}]*\)\s*\{", source))
            loops = len(re.findall(r"\b(?:for|while)\s*\(", source))
            if (record["id"] != item["id"] or
                    hashlib.sha256(source.encode()).hexdigest() !=
                    item["code_sha256"] or
                    len(source) != item["source_chars"] or
                    functions != item["functions"] or
                    loops != item["loops"] or
                    functions < selection["min_function_definitions"] or
                    loops < selection["min_loop_constructs"] or
                    len(source) < selection["min_source_chars"] or
                    item["dataset_file"] not in dataset_files or
                    item["license"] in selection["excluded_license_values"] or
                    any(record.get(name) for name in (
                        "buffer_a_code", "buffer_b_code", "buffer_c_code",
                        "buffer_d_code", "sound_code", "cube_a_code"))):
                raise ValueError("Complex source selection mismatch: " + item["id"])
        screened = [json.loads(line) for line in
                    (HERE / "screening.jsonl").read_text().splitlines()]
        screened_by_id = {item["id"]: item for item in screened}
        passing = {item["id"] for item in screened if item["status"] == "pass"}
        eligible_passing = [item for item in screened
                            if item["status"] == "pass" and
                            item["license"] not in
                            selection["excluded_license_values"]]
        if (len(screened) != selection["screened"] or
                len(screened_by_id) != len(screened) or
                len(passing) != selection["passed_both"] or
                len(eligible_passing) != selection["eligible_passed_both"] or
                any(screened_by_id[item["id"]]["sha256"] != item["sha256"]
                    for item in manifest["files"]) or
                [item["id"] for item in manifest["files"]] !=
                [item["id"] for item in sorted(
                    eligible_passing,
                    key=lambda item: (-item["source_chars"], item["id"]))
                 [:args.expected_count]]):
            raise ValueError("Complex screening provenance mismatch")
    subprocess.run(["python3", str(SCRIPT_DIR / "port.py"),
                    "--corpus-dir", str(HERE)], check=True)
    translation = json.loads((HERE / "ports/translation.json").read_text())
    tasks = [
        (item, opt)
        for item in translation
        if item["status"] == "translated"
        for opt in OPTIMIZATIONS
    ]
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results = list(pool.map(attempt, tasks))
    for item in translation:
        if item["status"] != "translated":
            for opt in OPTIMIZATIONS:
                results.append(
                    {
                        "id": item["id"],
                        "optimization": opt,
                        "status": item["status"],
                        "diagnostic": item["error"],
                    }
                )
    results.sort(key=lambda x: (x["id"], x["optimization"]))
    counts = dict(collections.Counter(r["status"] for r in results))
    per_shader = collections.defaultdict(dict)
    for result in results:
        per_shader[result["id"]][result["optimization"]] = result["status"]
    successful = sum(
        all(statuses.get(opt) == "pass" for opt in OPTIMIZATIONS)
        for statuses in per_shader.values()
    )
    passed_both = sum(
        statuses.get("-O0") == statuses.get("-O2") == "pass"
        for statuses in per_shader.values()
    )
    report = {
        "dataset_commit": manifest["commit"],
        "support_sha256": hashlib.sha256((SCRIPT_DIR / "support.h").read_bytes()).hexdigest(),
        "manifest_sha256": hashlib.sha256(
            (HERE / "upstream/manifest.json").read_bytes()
        ).hexdigest(),
        "compiler_version": subprocess.run(
            [str(BIN / "clang++"), "--version"],
            capture_output=True,
            text=True,
            check=True,
        ).stdout,
        "shaders": args.expected_count,
        "cases": len(results),
        "optimizations": list(OPTIMIZATIONS),
        "passed_all": successful,
        "passed_both": passed_both,
        "counts_by_optimization": {
            opt: dict(collections.Counter(
                result["status"] for result in results
                if result["optimization"] == opt
            )) for opt in OPTIMIZATIONS
        },
        "counts": counts,
        "results": results,
    }
    (ARTIFACTS / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    categories = collections.Counter()
    for result in results:
        if result["status"] != "pass":
            diagnostic = result.get("diagnostic", "")
            match = re.search(r"(?:fatal )?error: ([^\n]+)", diagnostic)
            categories[
                match[1]
                if match
                else diagnostic.splitlines()[0]
                if diagnostic
                else result["status"]
            ] += 1
    print(
        json.dumps(
            {
                k: v
                for k, v in report.items()
                if k not in ["results", "compiler_version"]
            },
            indent=2,
        )
    )
    print("First diagnostic categories:", categories.most_common(20))
    print("Report:", ARTIFACTS / "report.json")
    return int(args.strict and successful != args.expected_count)


if __name__ == "__main__":
    raise SystemExit(main())
