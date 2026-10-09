#!/usr/bin/env python3
"""Build the native compiler and its pinned validator. Needs CMake, Ninja, Git."""

import argparse
import json
import os
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument("--jobs", default=min(os.cpu_count() or 2, 16), type=int)
parser.add_argument("--cc", default="clang")
parser.add_argument("--cxx", default="clang++")
args = parser.parse_args()


def run(*command):
    subprocess.run([str(arg) for arg in command], cwd=root, check=True)


pins = json.loads((root / "shader-cxx/dependencies.json").read_text())
deps = root / ".shader-deps"
deps.mkdir(exist_ok=True)
for name in ("SPIRV-Tools", "SPIRV-Headers", "Vulkan-Headers"):
    checkout = deps / name
    pin = pins[name]
    if not checkout.exists():
        run("git", "init", checkout)
        run("git", "-C", checkout, "remote", "add", "origin", pin["url"])
    head = subprocess.run(
        ["git", "-C", str(checkout), "rev-parse", "HEAD"],
        capture_output=True,
        text=True,
    ).stdout.strip()
    if head != pin["revision"]:
        if subprocess.check_output(
            ["git", "-C", str(checkout), "status", "--porcelain"]
        ):
            raise SystemExit(
                f"{checkout} has local changes; preserve them before updating"
            )
        run("git", "-C", checkout, "fetch", "--depth=1", "origin", pin["revision"])
        run("git", "-C", checkout, "checkout", "--detach", pin["revision"])
run(
    "cmake",
    "-S",
    "llvm",
    "-B",
    "build",
    "-G",
    "Ninja",
    "-DCMAKE_BUILD_TYPE=Release",
    f"-DCMAKE_C_COMPILER={args.cc}",
    f"-DCMAKE_CXX_COMPILER={args.cxx}",
    "-DLLVM_ENABLE_PROJECTS=clang",
    "-DLLVM_TARGETS_TO_BUILD=X86;SPIRV",
    "-DLLVM_ENABLE_ASSERTIONS=ON",
    "-DLLVM_INCLUDE_BENCHMARKS=OFF",
    "-DCLANG_ENABLE_STATIC_ANALYZER=OFF",
    "-DCLANG_ENABLE_ARCMT=OFF",
    "-DLLVM_PARALLEL_LINK_JOBS=2",
)
run(
    "cmake",
    "--build",
    "build",
    "--parallel",
    args.jobs,
    "--target",
    "clang",
    "clang-spirv-link",
    "FileCheck",
)
run(
    "cmake",
    "-S",
    deps / "SPIRV-Tools",
    "-B",
    "shader-build/spirv-tools",
    "-G",
    "Ninja",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DSPIRV_SKIP_TESTS=ON",
    f'-DSPIRV-Headers_SOURCE_DIR={deps / "SPIRV-Headers"}',
)
run(
    "cmake",
    "--build",
    "shader-build/spirv-tools",
    "--parallel",
    args.jobs,
    "--target",
    "spirv-val",
    "spirv-as",
    "spirv-dis",
    "spirv-link",
)
for tool in ("spirv-val", "spirv-as", "spirv-dis", "spirv-link"):
    link = root / "build/bin" / tool
    target = root / "shader-build/spirv-tools/tools" / tool
    if link.exists() and not link.is_symlink():
        raise SystemExit(f"{link} is an existing regular file")
    if link.is_symlink():
        link.unlink()
    link.symlink_to(os.path.relpath(target, link.parent))
