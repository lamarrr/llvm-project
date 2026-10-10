#!/usr/bin/env python3
"""Test every implemented shader feature; advanced stages have rejection tests."""

import argparse
import itertools
import json
import os
import re
import struct
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BIN = ROOT / "build/bin"
parser = argparse.ArgumentParser()
parser.add_argument("--runtime", action="store_true")
parser.add_argument("--host-cxx", default="c++")
args = parser.parse_args()
checks = 0


def expect(condition, message):
    # Checks remain active under python -O.
    if not condition:
        raise AssertionError(message)


def run(*command, **kwargs):
    return subprocess.run([str(x) for x in command], cwd=ROOT, check=True, **kwargs)


def validate(output, entry):
    global checks
    expect(
        struct.unpack("<II", output.read_bytes()[:8]) == (0x07230203, 0x00010400),
        f"{entry}: wrong SPIR-V version",
    )
    run(BIN / "spirv-val", "--target-env", "vulkan1.3", "--scalar-block-layout", output)
    reflection = json.loads(Path(str(output) + ".json").read_text())
    expect(
        reflection["source_entry"] == entry or reflection["entry"] == entry,
        f"{entry}: reflection names wrong entry",
    )
    expect(reflection["target"] == "spirv1.4-unknown-vulkan1.3", "wrong profile")
    assembly = run(BIN / "spirv-dis", output, capture_output=True, text=True).stdout
    for capability in ("PhysicalStorageBufferAddresses", "VulkanMemoryModel"):
        expect(f"OpCapability {capability}" in assembly, f"missing {capability}")
    for extension in ("SPV_KHR_physical_storage_buffer", "SPV_KHR_vulkan_memory_model"):
        expect(extension in reflection["spirv_extensions"], f"missing {extension}")
    expect(
        {
            "bufferDeviceAddress",
            "descriptorIndexing",
            "vulkanMemoryModel",
            "scalarBlockLayout",
        }
        <= set(reflection["required_features"]),
        "missing profile features",
    )
    expect("OpAsmINTEL" not in assembly, "unexpected vendor inline assembly")
    checks += 1
    return reflection, assembly


# Fail when a newly declared public builtin/intrinsic lacks independent tests.
# Expected types, values and operations remain explicitly written in fixtures.
audit_mapping = json.loads((ROOT / "shader-cxx/tests/audit-coverage.json").read_text())
audit_findings = json.loads(
    (ROOT / "shader-cxx/audits/2026-10-07/findings.json").read_text()
)
audit_cases = {
    case["name"]
    for catalog in ("audit-regressions.json", "audit-runtime.json")
    for case in json.loads((ROOT / "shader-cxx/tests" / catalog).read_text())
}
expect(
    set(audit_mapping) == {finding["id"] for finding in audit_findings},
    "Every audit finding must have a permanent regression mapping",
)
for finding, cases in audit_mapping.items():
    expect(
        cases and set(cases) <= audit_cases,
        f"{finding}: missing permanent regression cases",
    )

builtin_source = (
    ROOT / "clang/include/clang/Basic/SPIRVShaderBuiltins.def"
).read_text()
public_builtins = set(
    re.findall(r"^SPIRV_SHADER_BUILTIN\((\w+),", builtin_source, re.M)
)
catalog = (ROOT / "clang/test/SemaSPIRVShader/catalog.cpp").read_text()
tested_builtins = set(re.findall(r"^(?:SCALAR|ARRAY)\((\w+),", catalog, re.M))
expect(
    public_builtins == tested_builtins,
    f"Scope catalog coverage differs: {public_builtins ^ tested_builtins}",
)
sema_source = (ROOT / "clang/lib/Sema/SemaSPIRVShader.cpp").read_text()
math_names = set(re.findall(r"^  SHADER_MATH\((\w+)\);", sema_source, re.M))
math_fixture = (ROOT / "shader-cxx/tests/fixtures/math-all.cpp").read_text()
math_calls = set(re.findall(r"\b(\w+f)\(", math_fixture))
expect(math_names == math_calls, f"Math coverage differs: {math_names ^ math_calls}")
fixture_paths = {
    path.relative_to(ROOT / "shader-cxx").as_posix()
    for path in (ROOT / "shader-cxx/tests/fixtures").glob("*.cpp")
}


native_catalog = (
    ROOT / "clang/include/clang/Basic/SPIRVShaderIntrinsics.def"
).read_text()
native_names = set(re.findall(r"^SPIRV_SHADER_INTRINSIC\((\w+),", native_catalog, re.M))
native_tests = "\n".join(
    path.read_text() for path in (ROOT / "shader-cxx/tests/fixtures").glob("*.cpp")
)
native_tests += (ROOT / "shader-cxx/tests/math-body.h").read_text()
native_calls = set(re.findall(r"\b(\w+)\s*\(", native_tests))
native_calls |= {
    "texture_" + name
    for name in re.findall(r"(?:\.|->)\s*(\w+)\s*\(", native_tests)
    if "texture_" + name in native_names
}
native_calls |= {
    "ray_query_" + name
    for name in re.findall(r"(?:\.|->)\s*(\w+)\s*\(", native_tests)
    if "ray_query_" + name in native_names
}
if re.search(r"\btextures\s*\[", native_tests):
    native_calls.add("descriptor_subscript")
expect(
    native_names <= native_calls,
    f"Untested native intrinsics: {native_names - native_calls}",
)

namespace_test = (
    ROOT / "clang/test/SemaSPIRVShader/namespace-isolation.cpp"
).read_text()
namespace_names = set(re.findall(r"^  TEST\((\w+)\)", namespace_test, re.M))
member_names = {
    name.removeprefix("texture_")
    for name in native_names
    if name.startswith("texture_")
} | {
    name.removeprefix("ray_query_")
    for name in native_names
    if name.startswith("ray_query_")
}
expect(
    namespace_names == native_names | math_names | member_names,
    "Namespace-isolation coverage differs from the intrinsic and method catalogs",
)

global_test = (ROOT / "clang/test/SemaSPIRVShader/global-overloads.cpp").read_text()
global_names = set(re.findall(r"^  TEST\((\w+)\)", global_test, re.M))
expect(
    global_names
    == namespace_names
    - public_builtins
    - math_names
    - {"primitive_idx", "instance_custom_idx", "front_facing"},
    "Global-overload coverage differs from the intrinsic and method catalogs",
)

cc1 = [
    BIN / "clang",
    "-cc1",
    "-triple",
    "spirv1.4-unknown-vulkan1.3",
    "-std=c++20",
    "-fspirv-shader",
]
for test in sorted((ROOT / "clang/test/SemaSPIRVShader").glob("*.cpp")):
    if test.name.startswith("host-"):
        run(
            BIN / "clang",
            "-cc1",
            "-triple",
            "x86_64-unknown-linux-gnu",
            "-std=c++20",
            "-fsyntax-only",
            "-verify",
            *(["-fshader-host-math"] if test.name == "host-math-gating.cpp" else []),
            test,
        )
    else:
        run(*cc1, "-fsyntax-only", "-verify", test)
    checks += 1
    if test.name == "namespace-isolation.cpp":
        namespace_ir = run(
            *cc1, "-emit-llvm", "-o", "-", test, capture_output=True, text=True
        ).stdout
        for name in namespace_names:
            symbol = f"_ZN9functions{len(name)}{name}Ei"
            expect(
                re.search(r"\bcall[^\n]*@" + re.escape(symbol) + r"\(", namespace_ir),
                f"{name}: namespace call did not use the user's C++ function",
            )
        checks += 1
    if test.name == "global-overloads.cpp":
        global_ir = run(
            *cc1, "-emit-llvm", "-o", "-", test, capture_output=True, text=True
        ).stdout
        for name in global_names:
            symbol = f"_Z{len(name)}{name}i"
            expect(
                re.search(r"\bcall[^\n]*@" + re.escape(symbol) + r"\(", global_ir),
                f"{name}: global call did not use the user's C++ function",
            )
        checks += 1
for test in sorted((ROOT / "clang/test/CodeGenSPIRVShader").glob("*.cpp")):
    ir = run(*cc1, "-emit-llvm", "-o", "-", test, capture_output=True).stdout
    run(BIN / "FileCheck", test, input=ir)
    checks += 1

with tempfile.TemporaryDirectory(prefix="shader-cxx-") as directory:
    tmp = Path(directory)
    requirements = tmp / "reflection-requirements"
    run(
        args.host_cxx,
        "-std=c++20",
        "-I.shader-deps/Vulkan-Headers/include",
        "shader-cxx/tests/reflection-requirements.cpp",
        "-Illvm/include",
        "-Ibuild/include",
        "build/lib/libLLVMSupport.a",
        "build/lib/libLLVMDemangle.a",
        "-lz",
        "-lzstd",
        "-ldl",
        "-lpthread",
        "-o",
        requirements,
    )
    run(requirements, tmp / "mock-shader.spv")
    checks += 1
    linkage = tmp / "linkage.cpp"
    linkage.write_text(
        "__compute__ __workgroup_size__(1, 1, 1) void shade() {}\n"
        "namespace nested { __compute__ __workgroup_size__(1, 1, 1) void shade() {} }\n"
        'extern "C" { __compute__ __workgroup_size__(1, 1, 1) void plain() {} }\n'
        "__vertex__ void overloaded(__input__(0) unsigned value) {}\n"
        "__vertex__ void overloaded(__input__(0) float value) {}\n"
        "__fragment__ void fragment(__output__(0) float *color) { *color = 1; }\n"
    )
    for selector, exported, source_name in [
        ("shade", "_Z5shadev", "shade"),
        ("nested::shade", "_ZN6nested5shadeEv", "nested::shade"),
        ("plain", "plain", "plain"),
        ("_Z10overloadedj", "_Z10overloadedj", "overloaded"),
        ("_Z10overloadedf", "_Z10overloadedf", "overloaded"),
        ("fragment", "_Z8fragmentPU3AS8f", "fragment"),
    ]:
        output = tmp / "linkage.spv"
        run(
            BIN / "clang++",
            "-std=c++20",
            "-fspirv-shader",
            f"-fspirv-entry={selector}",
            linkage,
            "-o",
            output,
        )
        reflection, assembly = validate(output, exported)
        expect(reflection["source_entry"] == source_name, "wrong source linkage name")
        expect(f'"{exported}"' in assembly, "SPIR-V entry lost C++ linkage name")
    ambiguous = subprocess.run(
        [
            str(BIN / "clang++"),
            "-std=c++20",
            "-fspirv-shader",
            "-fspirv-entry=overloaded",
            str(linkage),
            "-o",
            str(tmp / "ambiguous.spv"),
        ],
        capture_output=True,
        text=True,
    )
    expect(
        ambiguous.returncode != 0 and "multiple entry points" in ambiguous.stderr,
        "overloaded source selector must be ambiguous",
    )
    checks += 1
    all_entries = tmp / "all-entries.spv"
    run(BIN / "clang++", "-std=c++20", "-fspirv-shader", linkage, "-o", all_entries)
    run(
        BIN / "spirv-val", "--target-env", "vulkan1.3", "--scalar-block-layout", all_entries
    )
    expect(
        struct.unpack("<II", all_entries.read_bytes()[:8])
        == (0x07230203, 0x00010400),
        "combined module must be SPIR-V 1.4",
    )
    all_reflection = json.loads(Path(str(all_entries) + ".json").read_text())
    all_assembly = run(
        BIN / "spirv-dis", all_entries, capture_output=True, text=True
    ).stdout
    expected_entries = {
        "_Z5shadev": "compute",
        "_ZN6nested5shadeEv": "compute",
        "plain": "compute",
        "_Z10overloadedj": "vertex",
        "_Z10overloadedf": "vertex",
        "_Z8fragmentPU3AS8f": "fragment",
    }
    expect(all_reflection["schema_version"] == 2, "wrong multi-entry schema")
    expect(all_reflection["target"] == "spirv1.4-unknown-vulkan1.3", "wrong target")
    expect(
        {entry["entry"]: entry["stage"] for entry in all_reflection["entries"]}
        == expected_entries,
        "missing or incorrectly staged entries in combined reflection",
    )
    expect(
        all_assembly.count("OpEntryPoint ") == len(expected_entries),
        "wrong entry count",
    )
    for entry in all_reflection["entries"]:
        expect(
            f'"{entry["entry"]}"' in all_assembly,
            "combined entry lost its C++ linkage name",
        )
    expect(
        set(all_reflection["required_features"])
        >= {"bufferDeviceAddress", "descriptorIndexing", "vulkanMemoryModel"},
        "combined feature reflection is incomplete",
    )
    checks += 1

    shared_entries = tmp / "shared-entries.cpp"
    shared_entries.write_text(
        "struct Push { __storage__ float *dst; };\n"
        "struct Uniform { float gain; };\n"
        "__compute__ __workgroup_size__(1, 1, 1) "
        "void first(__push_constant__ Push *p, "
        "__binding__(0, 0) __uniform__ const Uniform *u) "
        "{ p->dst[0] = u->gain; }\n"
        "__compute__ __workgroup_size__(2, 1, 1) "
        "void second(__push_constant__ Push *p, "
        "__binding__(0, 0) __uniform__ const Uniform *u) "
        "{ p->dst[0] = u->gain + 1; }\n"
    )
    shared_output = tmp / "shared-entries.spv"
    run(
        BIN / "clang++",
        "-std=c++20",
        "-fspirv-shader",
        shared_entries,
        "-o",
        shared_output,
    )
    run(
        BIN / "spirv-val",
        "--target-env",
        "vulkan1.3",
        "--scalar-block-layout",
        shared_output,
    )
    shared_reflection = json.loads(Path(str(shared_output) + ".json").read_text())
    expect(
        [entry["workgroup_size"] for entry in shared_reflection["entries"]]
        == [[1, 1, 1], [2, 1, 1]],
        "combined entries lost independent workgroup sizes",
    )
    for entry in shared_reflection["entries"]:
        expect(
            any(
                interface.get("binding") == 0 and interface.get("set") == 0
                for interface in entry["interfaces"]
            ),
            "combined entry lost its descriptor binding",
        )
    checks += 1
    for source, expected_stages in (
        ("control-extended", ["compute", "fragment"]),
        ("interpolation", ["vertex", "fragment"]),
    ):
        output = tmp / f"{source}-all.spv"
        run(
            BIN / "clang++",
            "-std=c++20",
            "-O2",
            "-fspirv-shader",
            ROOT / "shader-cxx/tests/fixtures" / f"{source}.cpp",
            "-o",
            output,
        )
        run(
            BIN / "spirv-val",
            "--target-env",
            "vulkan1.3",
            "--scalar-block-layout",
            output,
        )
        reflection = json.loads(Path(str(output) + ".json").read_text())
        expect(
            [entry["stage"] for entry in reflection["entries"]] == expected_stages,
            f"{source}: optimized combined module lost a stage",
        )
        checks += 1
    swizzle_catalog = tmp / "swizzle-catalog.cpp"
    swizzle_names = [
        "".join(components)
        for length in (2, 3, 4)
        for components in itertools.product("xyzw", repeat=length)
    ]
    scalar_widths = (
        ("u8", 8),
        ("u16", 16),
        ("u32", 32),
        ("u64", 64),
        ("i8", 8),
        ("i16", 16),
        ("i32", 32),
        ("i64", 64),
        ("f16", 16),
        ("f32", 32),
        ("f64", 64),
    )
    types_catalog = tmp / "types-catalog.cpp"
    types_catalog.write_text(
        "#include <types.hpp>\n"
        + "".join(
            f"static_assert(sizeof({name}) == {width // 8});\n"
            for name, width in scalar_widths
        )
    )
    run(BIN / "clang++", "-std=c++20", "-fspirv-shader", "-fsyntax-only", types_catalog)
    checks += 1

    removed_aliases = tuple(
        f"{prefix}vec{count}"
        for prefix in ("", "i", "u", "b")
        for count in (2, 3, 4)
    ) + tuple(f"mat{count}" for count in (2, 3, 4))
    legacy_types = tmp / "removed-glsl-type-aliases.cpp"
    legacy_types.write_text(
        "#include <types.hpp>\n"
        + "".join(f"{name} value_{name};\n" for name in removed_aliases)
    )
    result = subprocess.run(
        [
            str(BIN / "clang++"),
            "-std=c++20",
            "-fspirv-shader",
            "-fsyntax-only",
            "-ferror-limit=0",
            str(legacy_types),
        ],
        capture_output=True,
        text=True,
    )
    expect(
        result.returncode != 0
        and all(f"unknown type name '{name}'" in result.stderr for name in removed_aliases),
        "GLSL type aliases remain in the public header",
    )
    checks += 1

    for header, source in (
        (
            "vec.hpp",
            "constexpr vec<float, 3> value(1.0f, 2.0f, 3.0f);\n"
            "static_assert(value.z() == 3.0f && value.xy()[1] == 2.0f);\n",
        ),
        (
            "mat.hpp",
            "constexpr mat<2, 2> value(1.0f, 2.0f, 3.0f, 4.0f);\n"
            "static_assert(value[0][1] == 3.0f);\n",
        ),
    ):
        standalone = tmp / ("standalone-" + header + ".cpp")
        standalone.write_text(f"#include <{header}>\n" + source)
        run(BIN / "clang++", "-std=c++20", "-fspirv-shader", "-fsyntax-only", standalone)
        checks += 1

    swizzle_catalog.write_text(
        '#include <types.hpp>\n'
        "template<class V> concept has_z = requires(V v) { v.z(); };\n"
        "template<class V> concept has_w = requires(V v) { v.w(); };\n"
        "static_assert(!has_z<f32x2> && has_z<f32x3>);\n"
        "static_assert(!has_w<f32x3> && has_w<f32x4>);\n"
        "static_assert(__is_same(boolx3, vec<bool, 3>));\n"
        "static_assert(requires(const f32x4 &v) {\n"
        + "".join(f"  v.{name}();\n" for name in swizzle_names)
        + "});\n"
        + "".join(
            f"static_assert(sizeof({base}x{count}) == {count} * sizeof({base}));\n"
            for base, _ in scalar_widths
            for count in (2, 3, 4)
        )
        + "".join(
            f"static_assert(sizeof(boolx{count}) == {count} * sizeof(bool));\n"
            for count in (2, 3, 4)
        )
        + "constexpr f32x4 value(1.0f, 2.0f, 3.0f, 4.0f);\n"
        "static_assert(value.xx()[1] == 1.0f);\n"
        "static_assert(value.zyx()[0] == 3.0f);\n"
        "static_assert(value.wzyx()[3] == 1.0f);\n"
    )
    run(BIN / "clang++", "-std=c++20", "-fspirv-shader", "-fsyntax-only", swizzle_catalog)
    checks += 1

    matrix_alias_catalog = tmp / "matrix-alias-catalog.cpp"
    matrix_alias_catalog.write_text(
        "#include <types.hpp>\n"
        "static_assert(__is_same(f32x3x3, mat<3, 3>));\n"
        + "".join(
            f"static_assert(sizeof(f32x{rows}x{columns}) == {rows * columns} * sizeof(f32));\n"
            for rows in (2, 3, 4)
            for columns in (2, 3, 4)
        )
    )
    run(BIN / "clang++", "-std=c++20", "-fspirv-shader", "-fsyntax-only", matrix_alias_catalog)
    checks += 1

    for fixture in (
        "host-math",
        "host-vec-mat",
        "host-acos",
        "host-trig",
        "host-decomposition",
        "host-extended",
    ):
        for optimization in ("-O0", "-O2"):
            host = tmp / (fixture + optimization)
            run(
                BIN / "clang++",
                "-fshader-host-math",
                "-std=c++20",
                optimization,
                ROOT / f"shader-cxx/tests/{fixture}.cpp",
                "-o",
                host,
            )
            run(host)
            checks += 1
    cases = [
        ("examples/scale.cpp", "scale"),
        ("examples/alignment.cpp", "align"),
        ("examples/stages.cpp", "vertex"),
        ("examples/stages.cpp", "fragment"),
        ("examples/math.cpp", "math"),
        ("examples/uniform.cpp", "uniform"),
        ("tests/fixtures/math-all.cpp", "math_all"),
        ("tests/fixtures/acos.cpp", "acos_math"),
        ("tests/fixtures/trig.cpp", "trig"),
        ("tests/fixtures/extended-math.cpp", "extended_math"),
        ("tests/fixtures/extended-integer.cpp", "extended_integer"),
        ("tests/fixtures/inline-asm.cpp", "inline_asm"),
        ("tests/fixtures/atomic-float.cpp", "atomic_float"),
        ("tests/fixtures/atomic-policy.cpp", "atomic_policy"),
        ("tests/fixtures/decomposition.cpp", "decomposition"),
        ("tests/fixtures/image-queries.cpp", "image_queries"),
        ("tests/fixtures/image-offsets.cpp", "image_offsets"),
        ("tests/fixtures/control-extended.cpp", "control_extended"),
        ("tests/fixtures/control-extended.cpp", "demote_fragment"),
        ("tests/fixtures/bit-packing.cpp", "bit_packing"),
        ("tests/fixtures/classification.cpp", "classification"),
        ("tests/fixtures/matrix-overloads.cpp", "matrix_overloads"),
        ("tests/fixtures/subgroup-extended.cpp", "subgroup_extended"),
        ("tests/fixtures/vector-matrix.cpp", "vector_matrix"),
        ("tests/fixtures/vec-mat-headers.cpp", "vec_mat_headers"),
        ("tests/fixtures/width-aliases.cpp", "width_aliases"),
        ("tests/fixtures/value-results.cpp", "value_results"),
        ("tests/fixtures/specialization.cpp", "specialization"),
        ("tests/fixtures/specialization-types.cpp", "specialization_types"),
        ("tests/fixtures/resources.cpp", "resources"),
        ("tests/fixtures/descriptor-array.cpp", "sample_descriptors"),
        ("tests/fixtures/ray-query.cpp", "query"),
        ("tests/fixtures/operations.cpp", "operations"),
        ("tests/fixtures/atomic-types.cpp", "atomic_types"),
        ("tests/fixtures/fragment-operations.cpp", "fragment_operations"),
        ("tests/fixtures/image-types.cpp", "image_types"),
        ("tests/fixtures/pointers.cpp", "pointers"),
        ("tests/fixtures/pointer-link-and-loops.cpp", "pointer_link_and_loops"),
        ("tests/fixtures/pointer-link-and-loops.cpp", "pointer_link_and_loops_fragment"),
        ("tests/fixtures/layout.cpp", "layout"),
        ("tests/fixtures/shared.cpp", "shared"),
        ("tests/fixtures/synchronization.cpp", "synchronization"),
        ("tests/fixtures/scope.cpp", "scope"),
        ("tests/fixtures/fragment-scope.cpp", "fragment_scope"),
        ("tests/fixtures/vertex-scope.cpp", "vertex_scope"),
        ("tests/fixtures/invocation-state.cpp", "invocation_state"),
        ("tests/fixtures/cxx20.cpp", "cxx20"),
        ("tests/fixtures/interpolation.cpp", "interpolation_vertex"),
        ("tests/fixtures/interpolation.cpp", "interpolation_fragment"),
    ]
    expect(
        fixture_paths
        == {source for source, _ in cases if source.startswith("tests/fixtures/")},
        "a feature fixture is missing from the compilation matrix",
    )
    for source, entry in cases:
        output = tmp / f"{entry}.spv"
        run(
            BIN / "clang++",
            "-fspirv-shader",
            "-std=c++20",
            f"-fspirv-entry={entry}",
            ROOT / "shader-cxx" / source,
            "-o",
            output,
        )
        reflection, assembly = validate(output, entry)
        expected_operations = {
            "resources": {
                "OpTypeImage",
                "OpTypeSampler",
                "OpSampledImage",
                "OpImageSampleExplicitLod",
                "OpImageWrite",
            },
            "sample_descriptors": {
                "OpTypeRuntimeArray",
                "NonUniform",
                "RuntimeDescriptorArray",
                "SampledImageArrayNonUniformIndexing",
            },
            "query": {
                "OpTypeRayQueryKHR",
                "OpRayQueryInitializeKHR",
                "OpRayQueryProceedKHR",
                "OpRayQueryTerminateKHR",
                "OpRayQueryConfirmIntersectionKHR",
                "OpRayQueryGenerateIntersectionKHR",
                "OpRayQueryGetIntersectionTypeKHR",
                "OpRayQueryGetIntersectionTKHR",
                "OpRayQueryGetIntersectionPrimitiveIndexKHR",
                "OpRayQueryGetIntersectionInstanceCustomIndexKHR",
                "OpRayQueryGetIntersectionGeometryIndexKHR",
                "OpRayQueryGetIntersectionFrontFaceKHR",
            },
            "fragment_operations": {
                "OpDPdx",
                "OpDPdy",
                "OpFwidth",
                "OpDPdxFine",
                "OpDPdyFine",
                "OpDPdxCoarse",
                "OpDPdyCoarse",
                "OpKill",
                "OpImageSampleImplicitLod",
                "OpImageGather",
                "Grad",
            },
            "image_types": {
                "OpImageFetch",
                "OpImageRead",
                "OpImageWrite",
                "OpImageQuerySize",
                "OpImageQuerySizeLod",
            },
            "atomic_float": {"OpAtomicFAddEXT", "OpAtomicFMinEXT", "OpAtomicFMaxEXT"},
            "atomic_policy": {
                "OpAtomicLoad",
                "OpAtomicStore",
                "OpAtomicIAdd",
                "OpAtomicCompareExchange",
            },
            "classification": {"OpBitcast", "OpShiftRightLogical", "OpBitwiseAnd"},
            "extended_integer": {
                "OpIAddCarry",
                "OpISubBorrow",
                "OpUMulExtended",
                "OpSMulExtended",
            },
            "subgroup_extended": {
                "OpGroupNonUniformUMin",
                "OpGroupNonUniformUMax",
                "OpGroupNonUniformBitwiseAnd",
                "OpGroupNonUniformBitwiseOr",
                "OpGroupNonUniformBitwiseXor",
                "OpGroupNonUniformLogicalAnd",
                "OpGroupNonUniformLogicalOr",
                "OpGroupNonUniformLogicalXor",
                "ClusteredReduce",
                "InclusiveScan",
                "ExclusiveScan",
            },
            "image_queries": {
                "OpImageQueryLevels",
                "OpImageQuerySamples",
                "OpImageQueryLod",
                "OpImageFetch",
                "OpImageRead",
                "OpImageWrite",
                "Bias",
                "Sample",
            },
            "image_offsets": {
                "OpImageSampleImplicitLod",
                "OpImageSampleExplicitLod",
                "OpImageGather",
                "ConstOffset",
            },
            "control_extended": {
                "OpReadClockKHR",
                "OpMemoryBarrier",
                "OpControlBarrier",
            },
            "demote_fragment": {"OpDemoteToHelperInvocation"},
            "query": {
                "OpRayQueryGetRayTMinKHR",
                "OpRayQueryGetRayFlagsKHR",
                "OpRayQueryGetIntersectionInstanceIdKHR",
                "OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR",
                "OpRayQueryGetIntersectionBarycentricsKHR",
                "OpRayQueryGetIntersectionCandidateAABBOpaqueKHR",
                "OpRayQueryGetIntersectionObjectRayOriginKHR",
                "OpRayQueryGetIntersectionObjectRayDirectionKHR",
                "OpRayQueryGetWorldRayOriginKHR",
                "OpRayQueryGetWorldRayDirectionKHR",
            },
            "operations": {
                "OpAtomicLoad",
                "OpAtomicStore",
                "OpAtomicIAdd",
                "OpAtomicISub",
                "OpAtomicExchange",
                "OpAtomicUMin",
                "OpAtomicUMax",
                "OpAtomicAnd",
                "OpAtomicOr",
                "OpAtomicXor",
                "OpAtomicCompareExchange",
                "OpControlBarrier",
                "OpMemoryBarrier",
                "OpGroupNonUniformBallot",
                "OpGroupNonUniformBallotBitCount",
                "InclusiveScan",
                "ExclusiveScan",
            },
            "vector_matrix": {"OpDot", "OpTypeVector"},
            "vec_mat_headers": {
                "OpDot",
                "OpVectorShuffle",
                "OpFAdd %v4float",
                " Sqrt ",
            },
            "width_aliases": {
                "OpIAdd %v2uchar",
                "OpIAdd %v2ushort",
                "OpIAdd %v2uint",
                "OpIAdd %v2ulong",
                "OpFAdd %v2half",
                "OpFAdd %v2float",
                "OpFAdd %v2double",
            },
            "specialization": {"OpSpecConstant", "OpSpecConstantTrue", "SpecId"},
            "pointer_link_and_loops": {
                "OpTypePointer PhysicalStorageBuffer",
                "OpAccessChain",
            },
            "pointer_link_and_loops_fragment": {
                "OpTypeImage",
                "OpImageSampleImplicitLod",
            },
        }
        for operation in expected_operations.get(entry, set()):
            expect(operation in assembly, f"{entry}: missing {operation}")
        if entry == "vec_mat_headers":
            expect(
                assembly.count("OpDot ") >= 5,
                "matrix operators must lower row/column products through OpDot",
            )
        if entry == "sample_descriptors":
            descriptor = next(
                i
                for i in reflection["interfaces"]
                if i.get("resource_type") == "descriptor_array"
            )
            expect(
                descriptor["descriptor_count"] == 0
                and descriptor["element_resource_type"] == "texture_2d",
                "incorrect descriptor array reflection",
            )
            expect(
                {"runtimeDescriptorArray", "shaderSampledImageArrayNonUniformIndexing"}
                <= set(reflection["required_features"]),
                "missing descriptor feature requirements",
            )
        if entry == "atomic_float":
            expect(
                {
                    "shaderBufferFloat32AtomicAdd",
                    "shaderBufferFloat64AtomicAdd",
                    "shaderBufferFloat32AtomicMinMax",
                    "shaderBufferFloat64AtomicMinMax",
                    "shaderSharedFloat32AtomicAdd",
                    "shaderSharedFloat32AtomicMinMax",
                }
                <= set(reflection["required_features"]),
                "missing floating atomic feature requirements",
            )
        if entry == "control_extended":
            expect(
                {"shaderDeviceClock", "shaderSubgroupClock"}
                <= set(reflection["required_features"]),
                "missing clock feature requirements",
            )
        if entry == "demote_fragment":
            expect(
                "shaderDemoteToHelperInvocation" in reflection["required_features"],
                "missing demotion feature requirement",
            )
        if entry == "query":
            expect(
                {"rayQuery", "accelerationStructure"}
                <= set(reflection["required_features"]),
                "missing ray query feature requirements",
            )
        if entry == "specialization_types":
            constants = {c["id"]: c for c in reflection["specialization_constants"]}
            expect(len(constants) == 9, "alias specialization ID did not deduplicate")
            expect(
                constants[6]["default"] == 0xFFFFFFFFFFFFFFFF
                and constants[6]["default_bits"] == "FFFFFFFFFFFFFFFF",
                "unsigned specialization default was truncated",
            )
            expect(
                [constants[i]["bit_width"] for i in range(9)]
                == [32, 8, 8, 16, 16, 64, 64, 64, 16],
                "wrong specialization widths",
            )
        if entry == "atomic_types":
            expect(
                "Int64Atomics" in assembly
                and all(
                    "Op" + op in assembly
                    for op in (
                        "AtomicLoad",
                        "AtomicStore",
                        "AtomicIAdd",
                        "AtomicISub",
                        "AtomicExchange",
                        "AtomicSMin",
                        "AtomicUMin",
                        "AtomicSMax",
                        "AtomicUMax",
                        "AtomicAnd",
                        "AtomicOr",
                        "AtomicXor",
                        "AtomicCompareExchange",
                    )
                ),
                "missing typed atomic operations",
            )
        if entry == "specialization":
            constants = reflection["specialization_constants"]
            expect(
                len(constants) == 5, "specialization constants were not deduplicated"
            )
            expect(
                {c["id"] for c in constants} == set(range(5)),
                "wrong specialization IDs",
            )
        if entry.startswith("interpolation_"):
            interfaces = {
                i.get("location"): i
                for i in reflection["interfaces"]
                if i["kind"] == ("input" if entry.endswith("fragment") else "output")
            }
            expect(interfaces[0]["interpolation"] == "flat", "missing reflected flat")
            expect(
                interfaces[1]["interpolation"] == "noperspective"
                and interfaces[1]["interpolation_sampling"] == "centroid",
                "missing linear centroid",
            )
            expect(
                interfaces[2]["interpolation_sampling"] == "sample", "missing sample"
            )
            expect(
                interfaces[3]["interpolation"] == "noperspective",
                "missing array interpolation",
            )
            expect(
                "interpolation" not in interfaces[5]
                and "interpolation_sampling" not in interfaces[5],
                "default interpolation changed",
            )
            for decoration in ("Flat", "NoPerspective", "Centroid", "Sample"):
                expect(
                    re.search(r"OpDecorate %\S+ " + decoration + r"$", assembly, re.M),
                    f"missing {decoration} decoration",
                )
            expect(
                "sampleRateShading" in reflection["required_features"],
                "missing sample feature",
            )
        if entry == "scale":
            push = next(
                i for i in reflection["interfaces"] if i["kind"] == "push_constant"
            )
            expect(push["size"] == 16 and push["alignment"] == 8, "push ABI mismatch")
            expect(
                [f["offset"] for f in push["members"]] == [0, 8],
                "push offsets mismatch",
            )
            expect(
                push["members"][0]["kind"] == "device_address", "pointer must be BDA"
            )
            expect(reflection["workgroup_size"] == [64, 1, 1], "wrong workgroup size")
        if entry == "uniform":
            uniform = next(
                i for i in reflection["interfaces"] if i["kind"] == "uniform"
            )
            expect((uniform["set"], uniform["binding"]) == (2, 7), "binding mismatch")
            for decoration in ("DescriptorSet 2", "Binding 7", "Block"):
                expect(decoration in assembly, f"missing {decoration}")
        if entry == "align":
            accesses = [line for line in assembly.splitlines() if "Aligned" in line]
            expect(
                len(accesses) == 4
                and ["Aligned 16" in line for line in accesses]
                == [True, True, False, False],
                "alignment must weaken after increment",
            )
        if entry in ("vertex", "fragment"):
            expect("Location 0" in assembly, "missing varying location")
            expect(reflection["stage"] == entry, "wrong stage")
            inputs = {i.get("builtin") for i in reflection["interfaces"]}
            expect(
                ({42, 43} if entry == "vertex" else {17}) <= inputs,
                "missing stage builtins",
            )
            expect(
                (
                    "OpEntryPoint Vertex"
                    if entry == "vertex"
                    else "OpEntryPoint Fragment"
                )
                in assembly,
                "wrong execution model",
            )
            if entry == "fragment":
                expect(
                    "OriginUpperLeft" in assembly and "OpTypeBool" in assembly,
                    "fragment origin or boolean interface is wrong",
                )
        if entry == "math_all":
            for operation in (
                "Sqrt",
                "Sin",
                "Cos",
                "Exp",
                "Exp2",
                "Log",
                "Log2",
                "FAbs",
                "Floor",
                "Ceil",
                "Trunc",
                "NMin",
                "NMax",
                "Fma",
            ):
                expect(
                    f" {operation} " in assembly, f"missing math intrinsic {operation}"
                )
        if entry == "pointers":
            for operation in (
                "OpConvertPtrToU",
                "OpPtrAccessChain",
            ):
                expect(operation in assembly, f"missing pointer operation {operation}")
            expect(
                "OpPtrEqual" not in assembly and "OpPtrNotEqual" not in assembly,
                "physical pointers must compare as addresses in SPIR-V 1.4",
            )
        if entry == "layout":
            push = next(
                i for i in reflection["interfaces"] if i["kind"] == "push_constant"
            )
            fields = push["members"]
            expect(
                (push["size"], push["alignment"]) == (48, 16),
                "record size/ABI alignment mismatch",
            )
            expect(
                [f["offset"] for f in fields] == [0, 8, 12, 28, 36],
                "record offsets mismatch",
            )
            expect(
                fields[2]["members"][0]["stride"] == 4
                and fields[3]["stride"] == 4
                and fields[4]["stride"] == 4
                and fields[4]["count"] == 3,
                "nested array stride mismatch",
            )
        if entry == "shared":
            expect(
                "OpTypePointer Workgroup" in assembly and " Workgroup" in assembly,
                "shared memory must use workgroup storage",
            )
        if entry == "scope":
            ids = {i.get("builtin") for i in reflection["interfaces"]}
            expect(
                {24, 26, 27, 28, 29, 36, 38, 40, 41} <= ids, "missing compute builtins"
            )
            expect(
                25 not in ids,
                "workgroup_size must be synthesized from execution dimensions",
            )
            expect(reflection["workgroup_size"] == [2, 3, 4], "wrong dimensions")
        if entry == "invocation_state":
            expect(
                "state" not in {i["name"] for i in reflection["interfaces"]},
                "ordinary global leaked into shader interface",
            )
        if entry == "vertex_scope":
            expect(
                {42, 43, 4424, 4425, 4426, 4440}
                <= {i.get("builtin") for i in reflection["interfaces"]},
                "vertex builtins missing",
            )
            expect(
                {"shaderDrawParameters", "multiview"}
                <= set(reflection["required_features"]),
                "vertex feature requirements missing",
            )
        if entry == "fragment_scope":
            expect(
                {7, 15, 16, 17, 18, 19, 23}
                <= {i.get("builtin") for i in reflection["interfaces"]},
                "fragment builtins missing",
            )
            expect(
                {"sampleRateShading", "geometryShader"}
                <= set(reflection["required_features"]),
                "missing fragment features",
            )
            expect(
                "SampleRateShading" in assembly and " Flat" in assembly,
                "sample builtins require capability and integer interpolation decoration",
            )

    # Optimization can change pointer spellings and loop induction variables.
    # Validate every positive fixture through that separate compilation path.
    for source, entry in cases:
        output = tmp / f"{entry}-o2.spv"
        run(
            BIN / "clang++",
            "-O2",
            "-fspirv-shader",
            "-std=c++20",
            f"-fspirv-entry={entry}",
            ROOT / "shader-cxx" / source,
            "-o",
            output,
        )
        _, optimized_assembly = validate(output, entry)
        if entry == "vec_mat_headers":
            expect(
                "OpFAdd %v4float" in optimized_assembly
                and optimized_assembly.count("OpDot ") >= 5
                and "OpVectorShuffle" in optimized_assembly,
                "vector/matrix headers lost native SPIR-V operations at -O2",
            )
    # O0 links the typed physical-pointer module; O1 exercises the backend's
    # loop pipeline with logical pointers to matrix elements. The loops must
    # also survive the most aggressive optimization setting.
    for optimization in ("-O1", "-O3"):
        output = tmp / f"pointer_link_and_loops{optimization}.spv"
        run(
            BIN / "clang++",
            optimization,
            "-fspirv-shader",
            "-std=c++20",
            "-fspirv-entry=pointer_link_and_loops",
            ROOT / "shader-cxx/tests/fixtures/pointer-link-and-loops.cpp",
            "-o",
            output,
        )
        _, assembly = validate(output, "pointer_link_and_loops")
        expect(
            "OpTypePointer PhysicalStorageBuffer" in assembly,
            "physical storage pointer lost at " + optimization,
        )
        if optimization == "-O1":
            expect("OpLoopMerge" in assembly, "matrix loops lost at -O1")
    # The original O1 failure only appeared when both stages were emitted
    # together. Keep the default multi-entry pipeline in this regression.
    for optimization in ("-O0", "-O1", "-O2", "-O3"):
        output = tmp / f"pointer_link_and_loops_all{optimization}.spv"
        run(
            BIN / "clang++",
            optimization,
            "-fspirv-shader",
            "-std=c++20",
            ROOT / "shader-cxx/tests/fixtures/pointer-link-and-loops.cpp",
            "-o",
            output,
        )
        run(
            BIN / "spirv-val",
            "--target-env",
            "vulkan1.3",
            "--scalar-block-layout",
            output,
        )
        reflection = json.loads(Path(str(output) + ".json").read_text())
        assembly = run(
            BIN / "spirv-dis", output, capture_output=True, text=True
        ).stdout
        expect(
            {entry["stage"] for entry in reflection["entries"]}
            == {"vertex", "fragment"}
            and assembly.count("OpEntryPoint ") == 2,
            "pointer regression lost a stage at " + optimization,
        )
        if optimization == "-O1":
            expect("OpLoopMerge" in assembly, "combined matrix loops lost at -O1")
        checks += 1
    optimized = tmp / "scale-o2.spv"
    helper = tmp / "helper.cpp"
    helper.write_text(
        "template<class T> T twice(T x) { return x * 2; }\n"
        "float helper(float x) { return twice(x); }\n"
    )
    shader = tmp / "shader.cpp"
    shader.write_text(
        "float helper(float);\nstruct P { __storage__ float *data; };\n"
        "__compute__ __workgroup_size__(1,1,1) void linked(__push_constant__ const P *p) "
        "{ *p->data = helper(*p->data); }\n"
    )
    for source in (helper, shader):
        run(
            BIN / "clang++",
            "-fspirv-shader",
            "-std=c++20",
            "-c",
            source,
            "-o",
            source.with_suffix(".bc"),
        )
    run(
        BIN / "clang-spirv-link",
        helper.with_suffix(".bc"),
        shader.with_suffix(".bc"),
        "--entry=linked",
        "-o",
        tmp / "linked.spv",
    )
    _, assembly = validate(tmp / "linked.spv", "linked")
    expect("OpFunctionCall" not in assembly, "helpers were not inlined")
    run(
        BIN / "clang-spirv-link",
        helper.with_suffix(".bc"),
        shader.with_suffix(".bc"),
        "--entry=linked",
        "--emit-llvm",
        "-o",
        tmp / "linked.ll",
    )
    expect(
        f"define void @{json.loads((tmp / 'linked.spv.json').read_text())['entry']}()"
        in (tmp / "linked.ll").read_text(),
        "LLVM inspection failed",
    )
    checks += 1

    def reject(name, source, diagnostic, *options):
        global checks
        path = tmp / f"{name}.cpp"
        path.write_text(source)
        output = tmp / f"{name}.spv"
        result = subprocess.run(
            [
                str(BIN / "clang++"),
                "-std=c++20",
                "-fspirv-shader",
                *options,
                str(path),
                "-o",
                str(output),
            ],
            capture_output=True,
            text=True,
        )
        expect(
            result.returncode != 0 and diagnostic in result.stderr,
            f"{name}: expected {diagnostic!r}, got:\n{result.stderr}",
        )
        expect(
            "PLEASE submit a bug report" not in result.stderr
            and "LLVM ERROR" not in result.stderr,
            f"{name}: failure must be a diagnostic, not a crash",
        )
        expect(
            not output.exists() and not Path(str(output) + ".json").exists(),
            f"{name}: failed compile published output",
        )
        checks += 1

    compute = "__compute__ __workgroup_size__(1,1,1) "
    negatives = [
        (
            "bad-texture-element",
            compute + "void entry(__binding__(0,0) texture_2d<double> t){}",
            "native descriptors require",
        ),
        (
            "bad-descriptor-element",
            compute + "void entry(__binding__(0,0) ray_query t[4]){}",
            "native descriptors require",
        ),
        (
            "resource-binding",
            compute + "void entry(texture_2d<float> t){}",
            "require __binding__",
        ),
        (
            "spec-default-conflict",
            "struct P{__storage__ int *data;};"
            + compute
            + "void entry(__push_constant__ const P *p){p->data[0]=specialization_constant(0,1)+specialization_constant(0,2);}",
            "conflicting specialization constant",
        ),
        (
            "spec-type-conflict",
            "struct P{__storage__ int *data;};"
            + compute
            + "void entry(__push_constant__ const P *p){p->data[0]=specialization_constant(0,1)+specialization_constant(0,1u);}",
            "conflicting specialization constant",
        ),
        ("recursive", compute + "void recursive() { recursive(); }", "recursion"),
        (
            "mutual",
            "void b(); void a(){ b(); } void b(){ a(); }"
            + compute
            + "void entry(){a();}",
            "recursion",
        ),
        (
            "assembly",
            compute + 'void entry(){asm volatile("OpNop" : : "z"(1));}',
            "SPIR-V asm constraints",
        ),
        (
            "unresolved",
            "void missing();" + compute + "void entry(){missing();}",
            "unresolved shader function",
        ),
        (
            "indirect",
            "struct P{void (*f)();};"
            + compute
            + "void entry(__push_constant__ const P *p){p->f();}",
            "unresolved indirect call",
        ),
        (
            "external",
            "extern unsigned missing;" + compute + "void entry(){unsigned x=missing;}",
            "unresolved external shader variable",
        ),
        (
            "global-init",
            "int init(); int x=init();" + compute + "void entry(){}",
            "dynamic global initialization",
        ),
        ("missing-size", "__compute__ void entry(){}", "requires __workgroup_size__"),
        (
            "vertex-size",
            "__vertex__ __workgroup_size__(1,1,1) void entry(){}",
            "does not have workgroup dimensions",
        ),
        (
            "plain-param",
            compute + "void entry(int x){}",
            "entry parameters must be qualified",
        ),
        (
            "push-count",
            compute + "void entry(__push_constant__ int *a,__push_constant__ int *b){}",
            "only one push-constant",
        ),
        (
            "uniform-binding",
            compute + "void entry(__uniform__ int *p){}",
            "uniform interfaces require __binding__",
        ),
        (
            "push-binding",
            compute + "void entry(__binding__(0,1) __push_constant__ int *p){}",
            "__binding__ applies to descriptor resources",
        ),
        (
            "output-value",
            "__vertex__ void entry(__output__(0) float x){}",
            "output parameters must be pointers",
        ),
        ("binding-arity", "__binding__(0) int x;", "requires exactly 2 arguments"),
        ("binding-extra", "__binding__(0,1,2) int x;", "requires exactly 2 arguments"),
        ("binding-negative", "__binding__(0,-1) int x;", "non-negative integral"),
        ("binding-overflow", "__binding__(0,0x100000000ULL) int x;", "32-bit"),
        ("binding-runtime", "int x; __binding__(0,x) int y;", "integer constant"),
        (
            "binding-conflict",
            "void f(__binding__(0,1) __binding__(0,2) int *x);",
            "conflicting descriptor bindings",
        ),
        ("workgroup-zero", "__workgroup_size__(1,0,1) void f();", "must be positive"),
        (
            "workgroup-negative",
            "__workgroup_size__(-1,1,1) void f();",
            "non-negative integral",
        ),
        (
            "workgroup-conflict",
            "__workgroup_size__(1,1,1) __workgroup_size__(2,1,1) void f();",
            "conflicting workgroup sizes",
        ),
        (
            "stage-conflict",
            "__compute__ __vertex__ void f();",
            "conflicting entry-point stages",
        ),
        ("return-type", "__compute__ int f();", "returning void"),
        ("variadic", "__compute__ void f(int,...);", "non-variadic free functions"),
        ("method", "struct S{__compute__ void f();};", "non-variadic free functions"),
        (
            "alignment-zero",
            "void f(__storage__ __align__(0) float *p);",
            "positive power of two",
        ),
        (
            "alignment-negative",
            "void f(__storage__ __align__((-2147483647-1)) float *p);",
            "non-negative integral",
        ),
        (
            "alignment-nonpower",
            "void f(__storage__ __align__(3) float *p);",
            "positive power of two",
        ),
        ("memory-conflict", "void f(__storage__ __uniform__ float *p);", "conflicting"),
        ("input-negative", "void f(__input__(-1) float x);", "non-negative integral"),
        ("input-write", "void f(__input__(0) float *p){*p=1;}", "read-only"),
        ("alias-index", "void f(){unsigned x=vertex_index;}", "undeclared identifier"),
        (
            "array-w",
            "void f(){unsigned x=global_invocation_id.w;}",
            "is not a structure",
        ),
        (
            "array-shorthand",
            "void f(){unsigned x[3]; unsigned y=x.x;}",
            "is not a structure",
        ),
        (
            "flat-noperspective",
            "__fragment__ void f(__input__(0) __flat__ __noperspective__ float x){}",
            "conflicting interpolation",
        ),
        (
            "flat-centroid",
            "__fragment__ void f(__input__(0) __centroid__ __flat__ float x){}",
            "conflicting interpolation",
        ),
        (
            "flat-sample",
            "__fragment__ void f(__input__(0) __flat__ __sample__ float x){}",
            "conflicting interpolation",
        ),
        (
            "centroid-sample",
            "__fragment__ void f(__input__(0) __sample__ __centroid__ float x){}",
            "conflicting interpolation",
        ),
        ("global-flat", "__flat__ float x;", "entry-point parameters"),
        (
            "flat-uniform",
            "struct P { float x; }; __fragment__ void f(__binding__(0,0) __flat__ __uniform__ P *p){}",
            "vertex outputs or fragment inputs",
        ),
        (
            "flat-vertex-input",
            "__vertex__ void f(__flat__ __input__(0) float x){}",
            "vertex outputs or fragment inputs",
        ),
        (
            "flat-fragment-output",
            "__fragment__ void f(__flat__ __output__(0) float *x){}",
            "vertex outputs or fragment inputs",
        ),
        (
            "integer-fragment-input",
            "__fragment__ void f(__input__(0) unsigned x){}",
            "require __flat__",
        ),
        (
            "integer-interpolation",
            "__vertex__ void f(__output__(0) __noperspective__ unsigned *x){}",
            "require floating-point",
        ),
        ("no-entry", "void helper(){}", "no matching C++ shader entry point"),
    ]
    for name, source, diagnostic in negatives:
        reject(name, source, diagnostic)
    for name, source in [
        ("global-input", "__input__(0) extern const float value;"),
        ("global-output", "__output__(0) extern float value;"),
        ("global-binding", "__binding__(0,1) int value;"),
        ("global-uniform", "__uniform__ float *value;"),
        ("global-push", "__push_constant__ float *value;"),
        ("global-storage", "__storage__ float *value;"),
        ("global-shared", "extern __shared__ float value[4];"),
        ("namespace-interface", "namespace n { __output__(0) extern float value; }"),
        ("namespace-resource", "namespace n { __storage__ float *value; }"),
        ("static-member", "struct S { static __storage__ float *value; };"),
        ("alias-resource", "using P=__storage__ float*; P value;"),
        ("array-resource", "using P=__storage__ float*; P values[2];"),
        ("record-resource", "struct S{__storage__ float *data;}; S value;"),
        (
            "nested-resource",
            "struct S{__storage__ float *data;}; struct T{S s[2];}; T value;",
        ),
        (
            "auto-resource",
            "using P=__storage__ float*; auto value=static_cast<P>(nullptr);",
        ),
        ("extern-resource", "void f(){extern __storage__ float *value;}"),
        ("static-resource", "void f(){static __storage__ float *value;}"),
        ("local-input", "void f(){__input__(0) float value;}"),
        ("local-output", "void f(){__output__(0) float value;}"),
        ("local-binding", "void f(){__binding__(0,1) int value;}"),
    ]:
        reject(name, source, "entry-point parameters")
    for stage in (
        "tessellation_control",
        "tessellation_evaluation",
        "task",
        "mesh",
        "ray_generation",
        "intersection",
        "any_hit",
        "closest_hit",
        "miss",
        "callable",
    ):
        size = "__workgroup_size__(1,1,1)" if stage in ("task", "mesh") else ""
        reject(
            stage,
            f"__{stage}__ {size} void entry(){{}}",
            f"stage '{stage}' is not implemented",
        )
    reject(
        "old-standard",
        compute + "void entry(){}",
        "requires C++20 or later",
        "-std=c++17",
    )
    reject(
        "wrong-profile",
        compute + "void entry(){}",
        "requires C++20 or later",
        "--target=spirv1.5-unknown-vulkan1.3",
    )
    reject(
        "unknown-entry",
        compute + "void entry(){}",
        "no matching C++ shader entry point",
        "-fspirv-entry=absent",
    )

    incompatible = tmp / "host.ll"
    incompatible.write_text(
        'target triple = "x86_64-unknown-linux-gnu"\ndefine void @host() { ret void }\n'
    )
    result = subprocess.run(
        [str(BIN / "clang-spirv-link"), str(incompatible), "-o", str(tmp / "host.spv")],
        capture_output=True,
        text=True,
    )
    expect(
        result.returncode != 0 and "incompatible target triple" in result.stderr,
        "mixed target modules accepted",
    )
    expect(not (tmp / "host.spv").exists(), "incompatible input published output")
    checks += 1

    # Force validator rejection and verify neither artifact nor temporary output survives.
    output = tmp / "rejected.spv"
    validator = tmp / "reject-validator"
    validator.write_text("#!/bin/sh\nexit 1\n")
    validator.chmod(0o755)
    result = subprocess.run(
        [
            str(BIN / "clang-spirv-link"),
            str(helper.with_suffix(".bc")),
            str(shader.with_suffix(".bc")),
            "--entry=linked",
            f"--validator={validator}",
            "-o",
            str(output),
        ],
        capture_output=True,
        text=True,
    )
    expect(
        result.returncode != 0 and "Vulkan shader validation failed" in result.stderr,
        "validator failure not propagated",
    )
    expect(not list(tmp.glob("rejected.spv*")), "validator failure published artifacts")
    checks += 1

    # Failed validation must also leave a previous successful artifact intact.
    output.write_bytes((tmp / "scale.spv").read_bytes())
    sidecar = Path(str(output) + ".json")
    sidecar.write_bytes(Path(str(tmp / "scale.spv") + ".json").read_bytes())
    original_binary, original_json = output.read_bytes(), sidecar.read_bytes()
    result = subprocess.run(
        [
            str(BIN / "clang-spirv-link"),
            str(helper.with_suffix(".bc")),
            str(shader.with_suffix(".bc")),
            "--entry=linked",
            f"--validator={validator}",
            "-o",
            str(output),
        ],
        capture_output=True,
        text=True,
    )
    expect(
        result.returncode != 0
        and output.read_bytes() == original_binary
        and sidecar.read_bytes() == original_json,
        "failed validation replaced existing output",
    )
    expect(len(list(tmp.glob("rejected.spv*"))) == 2, "temporary SPIR-V output leaked")
    checks += 1

    audit_outputs = []
    for catalog in ("audit-regressions.json", "audit-runtime.json"):
        for case in json.loads((ROOT / "shader-cxx/tests" / catalog).read_text()):
            path = tmp / ("audit-" + case["name"] + ".cpp")
            path.write_text(case["source"])
            for opt in ("O0", "O2"):
                output = tmp / ("audit-" + case["name"] + "-" + opt + ".spv")
                result = subprocess.run(
                    [
                        str(BIN / "clang++"),
                        "-std=c++20",
                        "-fspirv-shader",
                        "-" + opt,
                        str(path),
                        "-o",
                        str(output),
                    ],
                    capture_output=True,
                    text=True,
                )
                if "diagnostic" in case:
                    expect(
                        result.returncode != 0 and case["diagnostic"] in result.stderr,
                        f"{case['name']}/{opt}: missing diagnostic: {result.stderr}",
                    )
                    expect(
                        "PLEASE submit" not in result.stderr,
                        f"{case['name']}/{opt}: crashed instead of diagnosing",
                    )
                    checks += 1
                    continue
                expect(result.returncode == 0, f"{case['name']}/{opt}: {result.stderr}")
                reflection, assembly = validate(output, "main")
                for feature in case.get("features", []):
                    expect(
                        feature in reflection["required_features"],
                        f"{case['name']}: missing {feature}",
                    )
                for feature in case.get("absent_features", []):
                    expect(
                        feature not in reflection["required_features"],
                        f"{case['name']}: unnecessary {feature}",
                    )
                for operation in case.get("asm", []):
                    expect(
                        operation in assembly, f"{case['name']}: missing {operation}"
                    )
                for operation in case.get("absent_asm", []):
                    expect(
                        operation not in assembly,
                        f"{case['name']}: unexpected {operation}",
                    )
                if "descriptor_counts" in case:
                    counts = sorted(
                        i["descriptor_count"]
                        for i in reflection["interfaces"]
                        if i.get("resource_type") == "descriptor_array"
                    )
                    expect(
                        counts == sorted(case["descriptor_counts"]),
                        f"{case['name']}: wrong descriptor bounds: {counts}",
                    )
                if "descriptor_element_types" in case:
                    types = sorted(
                        i["element_resource_type"]
                        for i in reflection["interfaces"]
                        if i.get("resource_type") == "descriptor_array"
                    )
                    expect(
                        types == sorted(case["descriptor_element_types"]),
                        f"{case['name']}: wrong descriptor element types: {types}",
                    )
                required = reflection["required_limits"]
                if "workgroup_size" in reflection:
                    expect(
                        required["maxComputeWorkGroupSize"]
                        == reflection["workgroup_size"],
                        "wrong dimension requirements",
                    )
                    product = 1
                    for dimension in reflection["workgroup_size"]:
                        product *= dimension
                    expect(
                        product == required["maxComputeWorkGroupInvocations"],
                        "wrong invocation requirement",
                    )
                push = [
                    i["size"]
                    for i in reflection["interfaces"]
                    if i["kind"] == "push_constant"
                ]
                expect(
                    required["maxPushConstantsSize"] == max(push, default=0),
                    "wrong push-constant requirement",
                )
                if case["name"] == "shared_aligned":
                    expect(
                        required["maxComputeSharedMemorySize"] == 16,
                        "wrong shared-memory requirement",
                    )
                if catalog == "audit-runtime.json":
                    audit_outputs.append((case, output, reflection["entry"]))

    if args.runtime:
        harness = tmp / "run-scale"
        run(
            args.host_cxx,
            "-std=c++20",
            "-I.shader-deps/Vulkan-Headers/include",
            "shader-cxx/tests/run-scale.cpp",
            "-Illvm/include",
            "-Ibuild/include",
            "build/lib/libLLVMSupport.a",
            "build/lib/libLLVMDemangle.a",
            "-lz",
            "-lzstd",
            "-ldl",
            "-lpthread",
            "-l:libvulkan.so.1",
            "-o",
            harness,
        )
        env = os.environ.copy()
        env.setdefault("MESA_SHADER_CACHE_DISABLE", "true")
        exported_scale = json.loads((tmp / "scale.spv.json").read_text())["entry"]
        result = subprocess.run(
            [str(harness), str(tmp / "scale.spv"), exported_scale, "scale"], env=env
        )
        if result.returncode == 77:
            print("Runtime skipped: Vulkan device lacks required features")
        elif result.returncode:
            raise SystemExit(result.returncode)
        else:
            run(harness, optimized, exported_scale, "scale", env=env)
            checks += 2
            for entry, scenario in [
                ("align", "align"),
                ("math_all", "math"),
                ("vector_matrix", "vector_matrix"),
                ("specialization", "specialization"),
                ("specialization", "specialization_override"),
                ("specialization", "specialization_off"),
                ("specialization_types", "specialization_types"),
                ("pointers", "pointers"),
                ("operations", "operations"),
                ("shared", "shared"),
                ("synchronization", "synchronization"),
                ("linked", "linked"),
                ("invocation_state", "invocation_state"),
                ("cxx20", "scale"),
            ]:
                outputs = [tmp / f"{entry}.spv"]
                if entry != "linked":
                    outputs.append(tmp / f"{entry}-o2.spv")
                for output in outputs:
                    result = subprocess.run(
                        [
                            str(harness),
                            str(output),
                            json.loads(Path(str(output) + ".json").read_text())[
                                "entry"
                            ],
                            scenario,
                        ],
                        env=env,
                    )
                    if result.returncode == 77:
                        print(
                            f"Runtime skipped: {entry}/{scenario} lacks device features"
                        )
                    elif result.returncode:
                        raise SystemExit(result.returncode)
                    else:
                        checks += 1
            header_env = dict(
                env,
                SHADER_TEST_EXPECTED="10,44.267261,72",
                SHADER_TEST_TOLERANCE="0.0001",
            )
            for output in (
                tmp / "vec_mat_headers.spv",
                tmp / "vec_mat_headers-o2.spv",
            ):
                result = subprocess.run(
                    [
                        str(harness),
                        str(output),
                        json.loads(Path(str(output) + ".json").read_text())["entry"],
                        "audit",
                    ],
                    env=header_env,
                )
                if result.returncode == 77:
                    print(f"Runtime skipped: {output.stem} lacks device features")
                else:
                    expect(result.returncode == 0, f"{output.stem}: numerical regression")
                    checks += 1
            for case, output, exported in audit_outputs:
                audit_env = dict(
                    env, SHADER_TEST_EXPECTED=",".join(map(str, case["expected"]))
                )
                if "push" in case:
                    audit_env["SHADER_TEST_PUSH"] = case["push"]
                if "tolerance" in case:
                    audit_env["SHADER_TEST_TOLERANCE"] = str(case["tolerance"])
                result = subprocess.run(
                    [str(harness), str(output), exported, "audit"], env=audit_env
                )
                if result.returncode == 77:
                    print(f"Runtime skipped: {case['name']} lacks device features")
                else:
                    expect(
                        result.returncode == 0, f"{case['name']}: numerical regression"
                    )
                    checks += 1
with tempfile.TemporaryDirectory(prefix="shader-backend-regressions-") as directory:
    tmp = Path(directory)
    # These translated shaders exercise backend paths that the small feature
    # fixtures do not trigger: typed whole-object memset, byte offsets and
    # vector loads across array elements, late merge placement, and nested
    # selection merges.
    for shader, optimization in (
        ("clBGRy", "-O0"),
        ("dtS3zy", "-O0"),
        ("ml2GR3", "-O0"),
        ("dlBGR1", "-O2"),
        ("dtfGWj", "-O2"),
        ("DlB3WV", "-O2"),
        ("ctSGzG", "-O2"),
        ("ctfSRS", "-O2"),
        ("dl23Wt", "-O2"),
        ("dlj3Dy", "-O2"),
        ("dlj3Wy", "-O2"),
        ("dtXSWr", "-O2"),
        ("dtfSRS", "-O2"),
        ("dllXDH", "-O2"),
        ("Dl2GWm", "-O2"),
        ("dtfSWN", "-O2"),
        ("mlB3Rz", "-O2"),
        ("mlS3zD", "-O0"),
        ("mlS3zD", "-O2"),
    ):
        output = tmp / f"{shader}{optimization}.spv"
        run(
            BIN / "clang++",
            "-std=c++20",
            "-fspirv-shader",
            "-fspirv-entry=shadertoy",
            optimization,
            ROOT / "shader-ports/shadertoy/ports" / f"{shader}.cpp",
            "-o",
            output,
        )
        validate(output, "shadertoy")

print(f"Shader C++ checks passed ({checks} cases)")
