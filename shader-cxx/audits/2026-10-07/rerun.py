#!/usr/bin/env python3
"""Reproduce the exploratory compile audit; nonzero exits are findings, not CI tests."""

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import resource
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--output", type=Path, required=True)
parser.add_argument("--jobs", type=int, default=8)
parser.add_argument("--case", help="Run only the named case")
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
cases = json.loads((HERE / "cases.json").read_text())
if args.case:
    cases = [case for case in cases if case["name"] == args.case]
    if not cases:
        parser.error("unknown case")


def check(case):
    path = args.output / (case["name"] + ".cpp")
    path.write_text(case["source"])
    results = []
    for opt in ("O0", "O2"):
        command = [
            str(ROOT / "build/bin/clang++"),
            "-std=c++20",
            "-fspirv-shader",
            "-" + opt,
            str(path),
            "-o",
            str(args.output / (case["name"] + "-" + opt + ".spv")),
        ]
        try:
            process = subprocess.run(
                command, capture_output=True, text=True, timeout=30
            )
            code, diagnostic = process.returncode, process.stderr
        except subprocess.TimeoutExpired:
            code, diagnostic = 124, "TIMEOUT"
        results.append(
            {"opt": opt, "code": code, "diagnostic": diagnostic, "command": command}
        )
    return {"name": case["name"], "expected": case["expected"], "results": results}


with ThreadPoolExecutor(max_workers=args.jobs) as pool:
    results = list(pool.map(check, cases))
(args.output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
for result in results:
    print(result["name"], *(r["code"] for r in result["results"]))
