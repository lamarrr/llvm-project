#!/usr/bin/env python3
"""Reproduce the numerical Vulkan audit using the project's test harness."""

import argparse
import json
import os
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--output", type=Path, required=True)
parser.add_argument("--case", help="Run only the named case")
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
cases = json.loads((HERE / "runtime-cases.json").read_text())
expected = {
    r["name"]: r["expected"]
    for r in json.loads((HERE / "runtime-results.json").read_text())
}
if args.case:
    cases = [case for case in cases if case["name"] == args.case]
    if not cases:
        parser.error("unknown case")
runner = args.output / "runner"
subprocess.run(
    [
        "c++",
        "-std=c++20",
        "-I" + str(ROOT / ".shader-deps/Vulkan-Headers/include"),
        str(HERE / "runtime-runner.cpp"),
        "-l:libvulkan.so.1",
        "-o",
        str(runner),
    ],
    check=True,
)
results = []
for case in cases:
    path = args.output / (case["name"] + ".cpp")
    path.write_text(case["source"])
    for opt in ("O0", "O2"):
        spv = args.output / (case["name"] + "-" + opt + ".spv")
        process = subprocess.run(
            [
                str(ROOT / "build/bin/clang++"),
                "-std=c++20",
                "-fspirv-shader",
                "-" + opt,
                str(path),
                "-o",
                str(spv),
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        result = {
            "name": case["name"],
            "opt": opt,
            "compile_code": process.returncode,
            "compile_diagnostic": process.stderr,
            "expected": expected[case["name"]],
        }
        if process.returncode == 0:
            entry = json.loads(Path(str(spv) + ".json").read_text())["entry"]
            env = dict(
                os.environ,
                AUDIT_EXPECTED=result["expected"],
                MESA_SHADER_CACHE_DISABLE="true",
            )
            process = subprocess.run(
                [str(runner), str(spv), entry, "audit"],
                env=env,
                capture_output=True,
                text=True,
                timeout=30,
            )
            result.update(
                runtime_code=process.returncode,
                runtime_output=process.stdout + process.stderr,
            )
        results.append(result)
(args.output / "runtime-results.json").write_text(json.dumps(results, indent=2) + "\n")
for result in results:
    print(
        result["name"],
        result["opt"],
        result["compile_code"],
        result.get("runtime_code", "N/A"),
    )
