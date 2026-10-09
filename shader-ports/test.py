#!/usr/bin/env python3
"""Offline compile, validation, reflection and codegen checks for engine ports."""

import argparse
import hashlib
import json
import re
import struct
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
OPTIMIZATIONS = ("-O0", "-O1", "-O2", "-O3")

# Expectations come from the selected upstream shader paths, not generated output.
CASES = [
    (
        "copy_simple",
        "copy",
        "COPY_SIMPLE",
        {"OpImageFetch": 1, "OpImageSampleExplicitLod": 1},
    ),
    ("copy_depth", "copy", "COPY_DEPTH", {"OpImageFetch": 1}),
    ("copy_cube", "copy", "COPY_CUBE", {"OpImageSampleExplicitLod": 1}),
    ("copy_cube_array", "copy", "COPY_CUBE_ARRAY", {"OpImageSampleExplicitLod": 1}),
    ("luminance_read", "luminance_reduce", "READ_TEXTURE", {"OpImageFetch": 1}),
    ("luminance_intermediate", "luminance_reduce", "INTERMEDIATE", {"OpImageRead": 1}),
    (
        "luminance_final",
        "luminance_reduce",
        "WRITE_LUMINANCE",
        {"OpImageRead": 1, "OpImageFetch": 1},
    ),
    ("blur_gaussian", "blur", "BLUR_GAUSSIAN", {"OpImageSampleImplicitLod": 13}),
    ("blur_mipmap", "blur", "BLUR_MIPMAP", {"OpImageSampleImplicitLod": 4}),
    ("tonemap_linear", "tonemap", "TONEMAP_LINEAR", {"OpImageSampleExplicitLod": 1}),
    (
        "tonemap_reinhard",
        "tonemap",
        "TONEMAP_REINHARD",
        {"OpImageSampleExplicitLod": 1},
    ),
    ("tonemap_filmic", "tonemap", "TONEMAP_FILMIC", {"OpImageSampleExplicitLod": 1}),
    ("roughness", "roughness_limiter", "ROUGHNESS", {"OpImageFetch": 1}),
]

# Each case is a selected path from a pinned upstream project. Counts are
# lower bounds because optimized builds may duplicate operations when unrolling.
EXTERNAL_CASES = [
    ("vulkan_sampler_fragment", "vulkan_samples", "separate_sampler.cpp",
     "vulkan_separate_sampler", "fragment", {"OpImageSampleImplicitLod": 1},
     {(0, 1), (1, 0)}, None),
    ("vulkan_sampler_vertex", "vulkan_samples", "separate_sampler_vertex.cpp",
     "vulkan_separate_sampler_vertex", "vertex", {"OpDot": 8}, {(0, 0)}, None),
    ("vulkan_particle_integrate", "vulkan_samples", "particle_integrate.cpp",
     "vulkan_particle_integrate", "compute", {"OpFMul": 1}, set(), [256, 1, 1]),
    ("vulkan_nonuniform_quads", "vulkan_samples", "nonuniform_quads.cpp",
     "vulkan_nonuniform_quads", "fragment", {"OpImageSampleImplicitLod": 1},
     {(0, 0), (1, 0)}, None),
    ("vulkan_ray_shadow", "vulkan_samples", "ray_shadow.cpp",
     "vulkan_ray_shadow", "fragment",
     {"OpRayQueryInitializeKHR": 2, "OpRayQueryProceedKHR": 2,
      "OpRayQueryGetIntersectionTypeKHR": 2,
      "OpRayQueryGetIntersectionTKHR": 1}, {(0, 0), (0, 1)}, None),
    ("filament_gaussian", "filament", "gaussian.cpp",
     "filament_gaussian", "fragment", {"OpImageFetch": 31}, {(0, 0)}, None),
    ("filament_pbr_standard", "filament", "pbr.cpp",
     "filament_pbr_standard", "fragment",
     {"OpImageSampleImplicitLod": 3, "OpImageSampleExplicitLod": 3,
      "OpDot": 19}, {(0, 0), (0, 1), (0, 2), (0, 3), (0, 4),
                    (0, 5), (1, 0)}, None),
    ("filament_pbr_clearcoat", "filament", "pbr.cpp",
     "filament_pbr_clearcoat", "fragment",
     {"OpImageSampleImplicitLod": 3, "OpImageSampleExplicitLod": 4,
      "OpDot": 25}, {(0, 0), (0, 1), (0, 2), (0, 3), (0, 4),
                    (0, 5), (1, 0)}, None),
    ("fidelityfx_spd_quad", "fidelityfx", "spd_quad.cpp",
     "fidelityfx_spd_quad", "compute",
     {"OpImageFetch": 1, "OpGroupNonUniformQuadSwap": 3, "OpImageWrite": 1},
     {(0, 0), (0, 1)}, [64, 1, 1]),
    ("fidelityfx_spd_two_mips", "fidelityfx", "spd_two_mips.cpp",
     "fidelityfx_spd_two_mips", "compute",
     {"OpImageFetch": 16, "OpGroupNonUniformQuadSwap": 12,
      "OpImageWrite": 8}, {(0, 0), (0, 1), (0, 2)}, [256, 1, 1]),
    ("slang_shared_atomics", "slang", "shared_atomics.cpp",
     "slang_shared_atomics", "compute",
     {"OpAtomicIAdd": 3, "OpControlBarrier": 2}, set(), [4, 1, 1]),
]

IMAGE_OPS = [
    "OpImageFetch",
    "OpImageRead",
    "OpImageWrite",
    "OpImageSampleExplicitLod",
    "OpImageSampleImplicitLod",
]

OFFSETS = {
    "copy": [0, 16, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 80],
    "luminance_reduce": [0, 8, 12, 16, 20],
    "blur": [0, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60],
    "tonemap": [0, 12, 16, 24, 28, 32, 40, 44, 48, 52, 80, 84, 88, 92],
    "roughness_limiter": [0, 8, 12],
}
PUSH_SIZES = {
    "copy": 96,
    "luminance_reduce": 32,
    "blur": 64,
    "tonemap": 96,
    "roughness_limiter": 16,
}


def require(condition, message):
    # Deliberately active under python -O.
    if not condition:
        raise AssertionError(message)


def run(command):
    result = subprocess.run(
        [str(part) for part in command], capture_output=True, text=True, timeout=60
    )
    if result.returncode:
        raise RuntimeError(
            f"Command failed: {' '.join(map(str, command))}\n{result.stderr}"
        )
    return result.stdout


def instructions(assembly):
    definitions = {}
    operations = []
    for line in assembly.splitlines():
        line = line.split(";", 1)[0].strip()
        match = re.match(r"(%\S+)\s*=\s*(Op\w+.*)", line)
        if match:
            tokens = match[2].split()
            definitions[match[1]] = tokens
            operations.append(tokens)
        elif line.startswith("Op"):
            operations.append(line.split())
    return definitions, operations


def verify(case, optimization, bin_dir, output_dir, unroll=False):
    name, entry, define, expected_images = case
    if unroll:
        name += "_unrolled"
    result = {"name": name, "optimization": optimization}
    binary = output_dir / f"{name}-{optimization[1:]}.spv"
    try:
        command = [
            bin_dir / "clang++",
            "-std=c++20",
            "-fspirv-shader",
            f"-fspirv-entry={entry}",
            f"-D{define}",
            optimization,
            "-fno-crash-diagnostics",
            HERE / "godot" / f"{entry}.cpp",
            "-o",
            binary,
        ]
        if unroll:
            command.insert(1, "-DUNROLL_REDUCTION")
        result["command"] = [str(part) for part in command]
        # Stale output must never make a failed build look successful.
        binary.unlink(missing_ok=True)
        Path(str(binary) + ".json").unlink(missing_ok=True)
        binary.with_suffix(".spvasm").unlink(missing_ok=True)
        process = subprocess.run(
            [str(part) for part in command], text=True, capture_output=True, timeout=60
        )
        require(process.returncode == 0, f"Compilation failed:\n{process.stderr}")
        require(
            struct.unpack("<II", binary.read_bytes()[:8]) == (0x07230203, 0x00010400),
            "Wrong SPIR-V magic or version",
        )
        run(
            [
                bin_dir / "spirv-val",
                "--target-env",
                "vulkan1.3",
                "--scalar-block-layout",
                binary,
            ]
        )
        assembly = run([bin_dir / "spirv-dis", binary])
        binary.with_suffix(".spvasm").write_text(assembly)
        reflection = json.loads(Path(str(binary) + ".json").read_text())
        definitions, operations = instructions(assembly)
        counts = {
            op: sum(tokens[0] == op for tokens in operations)
            for op in IMAGE_OPS + ["OpControlBarrier", "OpFDiv", "OpLoopMerge"]
        }
        result["operations"] = counts
        expected_images = dict(expected_images)
        compute = entry in ("copy", "luminance_reduce", "roughness_limiter")
        if entry == "roughness_limiter" and optimization in ("-O2", "-O3"):
            expected_images["OpImageFetch"] = 4
        expected_images["OpImageWrite"] = int(compute)
        for op in IMAGE_OPS:
            require(
                counts[op] == expected_images.get(op, 0),
                f"{op}: expected {expected_images.get(op, 0)}, got {counts[op]}",
            )
        require(
            not any(t[0] == "OpFunctionCall" for t in operations),
            "Uninlined helper call",
        )
        require(
            not any("OpAsm" in t[0] for t in operations), "Unexpected inline assembly"
        )
        stage = "compute" if compute else "fragment"
        require(
            reflection["stage"] == stage and reflection["source_entry"] == entry,
            "Wrong reflected entry/stage",
        )
        require(reflection["target"] == "spirv1.4-unknown-vulkan1.3", "Wrong target")
        require(reflection["entry"].startswith("_Z"), "Lost normal C++ mangling")
        require(
            any(
                t[0] == "OpEntryPoint"
                and t[1] == ("GLCompute" if compute else "Fragment")
                and f'"{reflection["entry"]}"' in t
                for t in operations
            ),
            "Binary/reflection entry mismatch",
        )
        interfaces = reflection["interfaces"]
        push = [i for i in interfaces if i["kind"] == "push_constant"]
        require(
            len(push) == 1 and push[0]["size"] == PUSH_SIZES[entry],
            "Wrong push block size",
        )
        require(
            [m["offset"] for m in push[0]["members"]] == OFFSETS[entry],
            "Push member layout differs from upstream std430",
        )
        require(
            reflection["required_limits"]["maxPushConstantsSize"] == PUSH_SIZES[entry],
            "Wrong reflected push-constant limit",
        )
        bindings = {
            (i["set"], i["binding"]): i for i in interfaces if i["kind"] == "descriptor"
        }
        if entry == "copy":
            expected_bindings = {(0, 0), (3, 0)}
            if define != "COPY_DEPTH":
                expected_bindings.add((0, 1))
        elif entry in ("luminance_reduce", "roughness_limiter"):
            expected_bindings = {(0, 0), (1, 0)}
            if define == "WRITE_LUMINANCE":
                expected_bindings.add((2, 0))
        else:
            expected_bindings = {(0, 0), (0, 1)}
        require(set(bindings) == expected_bindings, "Wrong descriptor sets/bindings")
        require(
            {
                "bufferDeviceAddress",
                "descriptorIndexing",
                "scalarBlockLayout",
                "vulkanMemoryModel",
            }
            <= set(reflection["required_features"]),
            "Missing profile feature requirements",
        )
        image_types = [t for t in operations if t[0] == "OpTypeImage"]
        if compute:
            require(reflection["workgroup_size"] == [8, 8, 1], "Wrong workgroup size")
            require(
                reflection["required_limits"]["maxComputeWorkGroupInvocations"] == 64,
                "Wrong invocation limit",
            )
            require(
                any(
                    t[0] == "OpExecutionMode" and t[2:] == ["LocalSize", "8", "8", "1"]
                    for t in operations
                ),
                "Wrong binary workgroup dimensions",
            )
            require(
                any(t[0] == "OpBranchConditional" for t in operations),
                "Missing bounds/control flow",
            )
            image_format = (
                "R32f"
                if entry == "luminance_reduce" or define == "COPY_DEPTH"
                else "R8"
                if entry == "roughness_limiter"
                else "Rgba16f"
            )
            require(
                any(
                    t[2] == "2D" and t[6:8] == ["2", image_format] for t in image_types
                ),
                "Wrong storage image type/format",
            )
        else:
            require(
                any(
                    t[0] == "OpExecutionMode" and t[2:] == ["OriginUpperLeft"]
                    for t in operations
                ),
                "Wrong fragment origin",
            )
            require(
                [(i["location"], i["size"]) for i in interfaces if i["kind"] == "input"]
                == [(0, 8)],
                "Wrong UV varying",
            )
            require(
                [
                    (i["location"], i["size"])
                    for i in interfaces
                    if i["kind"] == "output"
                ]
                == [(0, 16)],
                "Wrong color output",
            )

            def storage(pointer):
                seen = set()
                while pointer in definitions and pointer not in seen:
                    seen.add(pointer)
                    tokens = definitions[pointer]
                    if tokens[0] == "OpVariable":
                        return tokens[2]
                    if tokens[0] in ("OpAccessChain", "OpInBoundsAccessChain"):
                        pointer = tokens[2]
                    else:
                        return None
                return None

            require(
                sum(t[0] == "OpStore" and storage(t[1]) == "Output" for t in operations)
                == 4,
                "Expected four component stores to stage output",
            )
        if entry == "luminance_reduce":
            if unroll or optimization in ("-O2", "-O3"):
                require(
                    counts["OpControlBarrier"] == 7,
                    "Expected initial barrier and six reduction-level barriers",
                )
                require(
                    counts["OpLoopMerge"] == 0,
                    "Template-expanded reduction acquired a loop",
                )
            else:
                require(
                    counts["OpControlBarrier"] == 2 and counts["OpLoopMerge"] >= 1,
                    "Missing synchronized reduction loop",
                )
                require(
                    any(t[0] == "OpShiftRightLogical" for t in operations),
                    "Missing halving reduction stride",
                )
            for tokens in operations:
                if tokens[0] == "OpControlBarrier":
                    values = [int(definitions[arg][2]) for arg in tokens[1:]]
                    require(
                        values == [2, 2, 0x108], "Wrong Workgroup barrier semantics"
                    )
            require(
                reflection["required_limits"]["maxComputeSharedMemorySize"] == 256,
                "Expected 64 shared floats",
            )
            require(
                any(t[0] == "OpVariable" and t[2] == "Workgroup" for t in operations),
                "Reduction scratch is not shared",
            )
        if define.startswith("COPY_CUBE"):
            require(
                any(
                    t[2] == "Cube" and t[4] == str(int(define == "COPY_CUBE_ARRAY"))
                    for t in image_types
                ),
                "Wrong cube image/layer type",
            )
            for intrinsic in ("Sin", "Cos"):
                require(
                    any(t[0] == "OpExtInst" and t[3] == intrinsic for t in operations),
                    f"Missing panorama {intrinsic}",
                )
        if define == "COPY_DEPTH" or define in ("TONEMAP_REINHARD", "TONEMAP_FILMIC"):
            require(counts["OpFDiv"] > 0, "Missing depth/tonemap division")
        if entry == "roughness_limiter":
            for intrinsic in ("Acos", "Pow", "Sqrt"):
                require(
                    any(t[0] == "OpExtInst" and t[3] == intrinsic for t in operations),
                    f"Missing roughness {intrinsic}",
                )
        if entry == "tonemap":
            require(
                sum(t[0] == "OpExtInst" and t[3] == "Pow" for t in operations) == 3,
                "Expected three sRGB power operations",
            )
        if define == "BLUR_GAUSSIAN":
            constants = {t[2] for t in operations if t[0] == "OpConstant"}
            require({"0.125", "0.03125"} <= constants, "Lost original blur weights")
        result.update(
            status="pass",
            entry=reflection["entry"],
            binary=str(binary),
            push_constant_bytes=push[0]["size"],
            bindings=sorted(expected_bindings),
        )
    except Exception as error:
        result.update(status="fail", error=str(error))
    return result


def verify_record_output(optimization, bin_dir, output_dir):
    name = "record_output_assignment"
    result = {"name": name, "optimization": optimization}
    binary = output_dir / f"{name}-{optimization[1:]}.spv"
    try:
        command = [
            bin_dir / "clang++",
            "-std=c++20",
            "-fspirv-shader",
            "-fspirv-entry=record_output",
            optimization,
            HERE / "probes/record-output.cpp",
            "-o",
            binary,
        ]
        result["command"] = [str(part) for part in command]
        binary.unlink(missing_ok=True)
        Path(str(binary) + ".json").unlink(missing_ok=True)
        binary.with_suffix(".spvasm").unlink(missing_ok=True)
        run(command)
        require(
            struct.unpack("<II", binary.read_bytes()[:8]) == (0x07230203, 0x00010400),
            "Wrong record-assignment SPIR-V version",
        )
        run(
            [
                bin_dir / "spirv-val",
                "--target-env",
                "vulkan1.3",
                "--scalar-block-layout",
                binary,
            ]
        )
        assembly = run([bin_dir / "spirv-dis", binary])
        binary.with_suffix(".spvasm").write_text(assembly)
        reflection = json.loads(Path(str(binary) + ".json").read_text())
        require(
            reflection["stage"] == "fragment"
            and reflection["source_entry"] == "record_output",
            "Wrong record-assignment entry",
        )
        require(
            [
                (i["kind"], i.get("location"), i["size"])
                for i in reflection["interfaces"]
            ]
            == [("input", 0, 16), ("output", 0, 16)],
            "Wrong record interfaces",
        )
        require(
            "OpFunctionCall" not in assembly and "OpPtrCastToGeneric" not in assembly,
            "Trivial assignment introduced a method call or generic pointer conversion",
        )
        require(
            "shaderInt8" not in reflection["required_features"],
            "Record copy introduced byte storage",
        )
        result.update(status="pass", entry=reflection["entry"], binary=str(binary))
    except Exception as error:
        result.update(status="fail", error=str(error))
    return result


def verify_imgui(stage, optimization, bin_dir, output_dir):
    name = f"imgui_{stage}"
    result = {"name": name, "optimization": optimization}
    binary = output_dir / f"{name}-{optimization[1:]}.spv"
    try:
        command = [
            bin_dir / "clang++",
            "-std=c++20",
            "-fspirv-shader",
            f"-fspirv-entry={name}",
            optimization,
            HERE / "imgui" / f"{stage}.cpp",
            "-o",
            binary,
        ]
        result["command"] = [str(part) for part in command]
        binary.unlink(missing_ok=True)
        Path(str(binary) + ".json").unlink(missing_ok=True)
        binary.with_suffix(".spvasm").unlink(missing_ok=True)
        run(command)
        require(
            struct.unpack("<II", binary.read_bytes()[:8]) == (0x07230203, 0x00010400),
            "Wrong ImGui SPIR-V version",
        )
        run(
            [
                bin_dir / "spirv-val",
                "--target-env",
                "vulkan1.3",
                "--scalar-block-layout",
                binary,
            ]
        )
        assembly = run([bin_dir / "spirv-dis", binary])
        binary.with_suffix(".spvasm").write_text(assembly)
        reflection = json.loads(Path(str(binary) + ".json").read_text())
        definitions, operations = instructions(assembly)
        counts = {
            op: sum(t[0] == op for t in operations)
            for op in IMAGE_OPS + ["OpFMul", "OpFAdd", "OpStore"]
        }
        require(
            reflection["stage"] == stage and reflection["source_entry"] == name,
            "Wrong ImGui stage or entry",
        )
        require(reflection["entry"].startswith("_Z"), "Lost C++ entry mangling")
        require(
            not any(
                t[0] in ("OpFunctionCall", "OpPtrCastToGeneric") for t in operations
            ),
            "Unlowered ImGui helper or pointer cast",
        )
        interfaces = reflection["interfaces"]
        located = {
            (i["kind"], i["location"]): i["size"] for i in interfaces if "location" in i
        }
        expected = (
            {("input", 0): 8, ("input", 1): 8, ("input", 2): 16, ("output", 0): 24}
            if stage == "vertex"
            else {("input", 0): 24, ("output", 0): 16}
        )
        require(located == expected, "ImGui input/output locations or sizes changed")
        binary_interfaces = {}
        for tokens in operations:
            if tokens[0] == "OpDecorate" and tokens[2] == "Location":
                variable = definitions[tokens[1]]
                pointer = definitions[variable[1]]
                binary_interfaces[(pointer[1], int(tokens[3]))] = pointer[2]

        def float_vector(type_id, width):
            vector = definitions[type_id]
            return (
                vector[0] == "OpTypeVector"
                and vector[2] == str(width)
                and definitions[vector[1]] == ["OpTypeFloat", "32"]
            )

        varying_kind = "Output" if stage == "vertex" else "Input"
        varying_type = definitions[binary_interfaces[(varying_kind, 0)]]
        require(
            varying_type[0] == "OpTypeStruct"
            and len(varying_type) == 3
            and float_vector(varying_type[1], 4)
            and float_vector(varying_type[2], 2),
            "ImGui color/UV must occupy vector locations 0 and 1",
        )
        vectors = (
            {("Input", 0): 2, ("Input", 1): 2, ("Input", 2): 4}
            if stage == "vertex"
            else {("Output", 0): 4}
        )
        require(
            all(
                float_vector(binary_interfaces[key], width)
                for key, width in vectors.items()
            ),
            "ImGui vertex inputs or fragment output have incorrect vector types",
        )
        varying = next(
            i
            for i in interfaces
            if i["kind"] == ("output" if stage == "vertex" else "input")
            and i.get("location") == 0
        )
        require(
            [m["offset"] for m in varying["members"]] == [0, 16],
            "ImGui color/UV varying layout changed",
        )
        bindings = {
            (i["set"], i["binding"]): i for i in interfaces if i["kind"] == "descriptor"
        }
        push = [i for i in interfaces if i["kind"] == "push_constant"]
        if stage == "vertex":
            require(
                not bindings
                and len(push) == 1
                and push[0]["size"] == 16
                and [m["offset"] for m in push[0]["members"]] == [0, 8],
                "ImGui vertex push layout changed",
            )
            positions = [i for i in interfaces if i.get("builtin") == 0]
            require(
                len(positions) == 1
                and positions[0]["kind"] == "output"
                and positions[0]["size"] == 16
                and "location" not in positions[0],
                "Missing built-in Position output",
            )
            position = next(
                t[1]
                for t in operations
                if t[0] == "OpDecorate" and t[2:] == ["BuiltIn", "Position"]
            )
            pointer = definitions[definitions[position][1]]
            require(
                pointer[0:2] == ["OpTypePointer", "Output"]
                and definitions[pointer[2]][0::2] == ["OpTypeVector", "4"],
                "Position must be an Output float4 vector",
            )
            fused = sum(t[0] == "OpExtInst" and t[3] == "Fma" for t in operations)
            require(
                counts["OpFMul"] + fused == 2 and counts["OpFAdd"] + fused == 2,
                "Lost ImGui scale/translation operations",
            )
            constants = {t[2] for t in operations if t[0] == "OpConstant"}
            require({"0", "1"} <= constants, "Lost clip-space z/w constants")
        else:
            require(
                not push and set(bindings) == {(0, 0), (1, 0)},
                "ImGui texture/sampler bindings changed",
            )
            require(
                bindings[(0, 0)]["resource_type"] == "texture_2d"
                and bindings[(1, 0)]["resource_type"] == "sampler",
                "Wrong ImGui resource types",
            )
            products = [t for t in operations if t[0] == "OpFMul"]
            require(
                len(products) == 1
                and definitions[products[0][1]][0::2] == ["OpTypeVector", "4"],
                "Lost component-wise color modulation",
            )
            require(
                any(
                    t[0] == "OpExecutionMode" and t[2:] == ["OriginUpperLeft"]
                    for t in operations
                ),
                "Wrong fragment origin",
            )
        for op in IMAGE_OPS:
            require(
                counts[op]
                == int(stage == "fragment" and op == "OpImageSampleImplicitLod"),
                f"Wrong ImGui {op} count",
            )
        result.update(
            status="pass",
            entry=reflection["entry"],
            binary=str(binary),
            operations=counts,
        )
    except Exception as error:
        result.update(status="fail", error=str(error))
    return result


def verify_external_port(case, optimization, bin_dir, output_dir):
    name, project, source, entry, stage, expected_ops, bindings, workgroup = case
    result = {"name": name, "optimization": optimization, "project": project}
    binary = output_dir / f"{name}-{optimization[1:]}.spv"
    try:
        command = [
            bin_dir / "clang++", "-std=c++20", "-fspirv-shader",
            f"-fspirv-entry={entry}", optimization,
            HERE / project / source, "-o", binary,
        ]
        result["command"] = [str(part) for part in command]
        binary.unlink(missing_ok=True)
        Path(str(binary) + ".json").unlink(missing_ok=True)
        binary.with_suffix(".spvasm").unlink(missing_ok=True)
        run(command)
        require(
            struct.unpack("<II", binary.read_bytes()[:8]) == (0x07230203, 0x00010400),
            "Wrong SPIR-V magic or version",
        )
        run([bin_dir / "spirv-val", "--target-env", "vulkan1.3",
             "--scalar-block-layout", binary])
        assembly = run([bin_dir / "spirv-dis", binary])
        binary.with_suffix(".spvasm").write_text(assembly)
        reflection = json.loads(Path(str(binary) + ".json").read_text())
        _, operations = instructions(assembly)
        require(reflection["stage"] == stage and reflection["source_entry"] == entry,
                "Wrong reflected stage or entry")
        require(reflection["entry"].startswith("_Z"), "Lost C++ entry mangling")
        require(reflection["target"] == "spirv1.4-unknown-vulkan1.3",
                "Wrong target")
        observed = {op: sum(t[0] == op for t in operations) for op in expected_ops}
        for op, minimum in expected_ops.items():
            require(observed[op] >= minimum,
                    f"{op}: expected at least {minimum}, got {observed[op]}")
        actual_bindings = {
            (i["set"], i["binding"])
            for i in reflection["interfaces"]
            if i["kind"] in ("descriptor", "uniform")
        }
        require(actual_bindings == bindings, "Wrong descriptor set/binding map")
        require(reflection.get("workgroup_size") == workgroup,
                "Wrong workgroup dimensions")
        interfaces = reflection["interfaces"]
        push = [i for i in interfaces if i["kind"] == "push_constant"]
        push_size = {
            "vulkan_particle_integrate": 16,
            "filament_gaussian": 88,
            "filament_pbr_standard": 48,
            "filament_pbr_clearcoat": 48,
            "slang_shared_atomics": 8,
        }.get(name)
        require([i["size"] for i in push] == ([] if push_size is None else [push_size]),
                "Wrong push-constant C layout")
        if push_size is not None:
            require(reflection["required_limits"]["maxPushConstantsSize"] == push_size,
                    "Wrong reflected push-constant limit")
        if name == "vulkan_sampler_vertex":
            require(any(i["kind"] == "uniform" and i["size"] == 144
                        and i["set"] == 0 and i["binding"] == 0
                        for i in interfaces), "Wrong vertex camera record")
            require(any(i["kind"] == "output" and i.get("builtin") == 0
                        for i in interfaces), "Missing vertex Position")
        if name == "vulkan_nonuniform_quads":
            require("SampledImageArrayNonUniformIndexing" in assembly,
                    "Missing nonuniform sampled-image capability")
            require("OpTypeRuntimeArray" in assembly,
                    "Missing runtime descriptor array")
            require(re.search(r"OpDecorate %\S+ NonUniform\b", assembly),
                    "Missing NonUniform descriptor decoration")
            require(re.search(r"OpDecorate %\S+ Flat\b", assembly),
                    "Missing Flat index decoration")
        if name == "vulkan_ray_shadow":
            require(any(i["kind"] == "uniform" and i["size"] == 160
                        and i["set"] == 0 and i["binding"] == 1
                        for i in interfaces), "Wrong ray camera/light record")
            if optimization != "-O3":
                require(sum(op[0] == "OpLoopMerge" for op in operations) >= 2,
                        "Missing nested ambient-occlusion loops")
            require(" Sin " in assembly and " Cos " in assembly,
                    "Missing trigonometric ray directions")
        if name == "filament_gaussian":
            require(any(i["kind"] == "descriptor" and i["arrayed"]
                        and i["resource_type"] == "texture_2d_layers"
                        for i in interfaces), "Lost layered Gaussian texture")
        if name.startswith("filament_pbr_"):
            require(observed["OpImageSampleImplicitLod"] == 3 and
                    observed["OpImageSampleExplicitLod"] ==
                    (4 if name.endswith("clearcoat") else 3),
                    "Wrong PBR material or IBL sampling path")
            require("OpConstant %float 453.5" in assembly and
                    "OpConstant %float 0.318309" in assembly and
                    " Sqrt " in assembly,
                    "Missing GGX or Smith BRDF math")
            require(any(i["kind"] == "uniform" and i["size"] == 96
                        and [m["offset"] for m in i["members"]] ==
                        [0, 16, 32, 48, 64, 80]
                        and i["set"] == 1 and i["binding"] == 0
                        for i in interfaces), "Wrong PBR frame record")
            require(any(i["kind"] == "push_constant" and i["size"] == 48
                        and [m["offset"] for m in i["members"]] ==
                        [0, 16, 32] for i in interfaces),
                    "Wrong PBR material record")
            require(sorted(i["binding"] for i in interfaces
                           if i["kind"] == "descriptor" and
                           i["resource_type"] == "texture_cube") == [4],
                    "Missing PBR environment map")
            require(sorted(i["location"] for i in interfaces
                           if i["kind"] == "input" and "location" in i) ==
                    [0, 1, 2, 3], "Wrong PBR stage inputs")
            require(any(i["kind"] == "output" and i.get("location") == 0
                        for i in interfaces), "Missing PBR color output")
            if name == "filament_pbr_standard":
                combined = output_dir / f"filament_pbr_both-{optimization[1:]}.spv"
                combined.unlink(missing_ok=True)
                Path(str(combined) + ".json").unlink(missing_ok=True)
                combined_command = [
                    bin_dir / "clang++", "-std=c++20", "-fspirv-shader",
                    optimization, HERE / "filament/pbr.cpp", "-o", combined,
                ]
                run(combined_command)
                run([bin_dir / "spirv-val", "--target-env", "vulkan1.3",
                     "--scalar-block-layout", combined])
                combined_assembly = run([bin_dir / "spirv-dis", combined])
                combined_reflection = json.loads(
                    Path(str(combined) + ".json").read_text())
                require({e["source_entry"] for e in
                         combined_reflection["entries"]} ==
                        {"filament_pbr_standard", "filament_pbr_clearcoat"}
                        and combined_assembly.count("OpEntryPoint Fragment") == 2,
                        "Default PBR module lost an entry point")
                result["combined_command"] = [str(part) for part in
                                              combined_command]
                result["combined_binary"] = str(combined)
        if name == "fidelityfx_spd_quad":
            require(any(i["kind"] == "descriptor" and i["binding"] == 1
                        and i["image_format"] == 1
                        for i in interfaces), "Wrong SPD output image format")
        if name == "fidelityfx_spd_two_mips":
            require(observed == {"OpImageFetch": 16,
                                 "OpGroupNonUniformQuadSwap": 12,
                                 "OpImageWrite": 8},
                    "Wrong two-mip reduction operation counts")
            require(sorted(i["binding"] for i in interfaces
                           if i["kind"] == "descriptor" and
                           i["resource_type"] == "storage_texture_2d" and
                           i["image_format"] == 1) == [1, 2],
                    "Wrong two-mip storage image layout")
        if name == "slang_shared_atomics":
            require(reflection["required_limits"]["maxComputeSharedMemorySize"] == 16,
                    "Wrong shared atomic array size")
        result.update(status="pass", entry=reflection["entry"],
                      operations=observed, binary=str(binary))
    except Exception as error:
        result.update(status="fail", error=str(error))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin-dir", type=Path, default=ROOT / "build/bin")
    parser.add_argument("--output-dir", type=Path, default=HERE / "artifacts")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Require all cases to compile and validate (also the default)",
    )
    args = parser.parse_args()
    args.bin_dir = args.bin_dir.resolve()
    args.output_dir = args.output_dir.resolve()
    require(args.jobs > 0, "--jobs must be positive")
    args.output_dir.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((HERE / "godot/upstream/manifest.json").read_text())
    imgui_manifest = json.loads((HERE / "imgui/upstream/manifest.json").read_text())
    external_manifests = {
        project: json.loads((HERE / project / "upstream/manifest.json").read_text())
        for project in ("vulkan_samples", "filament", "fidelityfx", "slang")
    }
    for project, snapshot in (("godot", manifest), ("imgui", imgui_manifest),
                              *external_manifests.items()):
        for item in snapshot["files"]:
            source = HERE / project / "upstream" / item["file"]
            require(
                hashlib.sha256(source.read_bytes()).hexdigest() == item["sha256"],
                f"Upstream snapshot changed: {source}",
            )
    tasks = [(case, optimization) for case in CASES for optimization in OPTIMIZATIONS]
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results = list(
            pool.map(lambda task: verify(*task, args.bin_dir, args.output_dir), tasks)
        )
        reduction_tasks = [
            (case, optimization)
            for case in CASES
            if case[1] == "luminance_reduce"
            for optimization in OPTIMIZATIONS
        ]
        results.extend(
            pool.map(
                lambda task: verify(*task, args.bin_dir, args.output_dir, unroll=True),
                reduction_tasks,
            )
        )
    for optimization in OPTIMIZATIONS:
        results.append(
            verify_record_output(optimization, args.bin_dir, args.output_dir)
        )
        for stage in ("vertex", "fragment"):
            results.append(
                verify_imgui(stage, optimization, args.bin_dir, args.output_dir)
            )
    external_tasks = [(case, optimization) for case in EXTERNAL_CASES
                      for optimization in OPTIMIZATIONS]
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results.extend(pool.map(
            lambda task: verify_external_port(*task, args.bin_dir, args.output_dir),
            external_tasks,
        ))
    for result in results:
        print(f"{result['status'].upper():9} {result['name']} {result['optimization']}")
        if "error" in result:
            print(result["error"], file=sys.stderr)
    counts = {
        status: sum(r["status"] == status for r in results)
        for status in ("pass", "fail")
    }
    report = {
        "upstream_commit": manifest["commit"],
        "imgui_upstream_commit": imgui_manifest["commit"],
        "external_upstream_commits": {
            project: snapshot["commit"]
            for project, snapshot in external_manifests.items()
        },
        "upstream_sources": {
            project: snapshot["files"]
            for project, snapshot in (("godot", manifest), ("imgui", imgui_manifest),
                                      *external_manifests.items())
        },
        "compiler_version": run([args.bin_dir / "clang++", "--version"]),
        "optimizations": list(OPTIMIZATIONS),
        "counts": counts,
        "results": results,
    }
    report_path = args.output_dir / "report.json"
    report_path.write_text(json.dumps(report, indent=2) + "\n")
    print(
        f"{counts['pass']} validated binaries, "
        f"{counts['fail']} failures. Report: {report_path}"
    )
    return int(counts["fail"] != 0)


if __name__ == "__main__":
    sys.exit(main())
