# Native shader C++ for Clang

This repository is an LLVM/Clang fork based on upstream
`92dfb7a5b82c63895be374da73e24a9aabbd5309` (6 October 2026).
The implementation is experimental and **does not yet implement the full agreed
language plan**. It includes a working compute, vertex and fragment compilation
path. The compute device-address example has also been executed on Mesa Vulkan.

## Build and test

Requirements: Git, Python 3, CMake, Ninja, a modern C++ compiler, and disk space for
an LLVM build. Dependencies are pinned in `dependencies.json`. The build script
fetches the three Khronos dependencies into the ignored `.shader-deps` directory.
It builds Clang, the native shader linker, and SPIR-V Tools. No default shader
header is injected.

```sh
python3 shader-cxx/tools/build.py --cc clang --cxx clang++ --jobs 16
python3 shader-cxx/tools/test.py
# Optional: needs the Vulkan loader and a compatible Vulkan device.
python3 shader-cxx/tools/test.py --runtime --host-cxx clang++
```

To use Mesa's CPU Vulkan driver, set `VK_ICD_FILENAMES` to its installed ICD
manifest. The runtime test returns a skip if required hardware features are
unavailable. Compilation and validation always run.

The feature audit is recorded in [tests/COVERAGE.md](tests/COVERAGE.md).
The runner has 94 mandatory cases and 15 optional Vulkan execution checks.
Every positive fixture is validated at default optimization and `-O2`; runtime
checks include scalar math, pointer operations, alignment, shared memory, linked
helpers, invocation-local state and a C++20 loop using a class and lambda.

```sh
build/bin/clang++ -std=c++20 -fspirv-shader \
  -fspirv-entry=scale shader-cxx/examples/scale.cpp -o scale.spv
```

The shader mode defaults to `spirv1.4-unknown-vulkan1.3`. Each compilation emits
one selected entry and a `scale.spv.json` reflection file. The compiler requires
`spirv-val` beside the compiler or on PATH and validates the binary against
Vulkan 1.3 before publishing it. A separate check requires a SPIR-V 1.4 header.

For reuse across translation units:

```sh
build/bin/clang++ -std=c++20 -fspirv-shader -c helpers.cpp -o helpers.bc
build/bin/clang++ -std=c++20 -fspirv-shader -c shader.cpp -o shader.bc
build/bin/clang-spirv-link helpers.bc shader.bc --entry=main -o shader.spv
```

The native linker links LLVM bitcode before selecting and legalizing the entry.
Reachable helper functions are inlined. Recursive, unresolved and indirect calls
are diagnosed. Dynamic global initialization is rejected.

## Implemented language surface

```cpp
struct Params {
  __storage__ __align__(16) float *data;
  unsigned count;
};
__compute__ __workgroup_size__(64, 1, 1)
void scale(__push_constant__ const Params *p) {
  const unsigned (&id)[3] = global_invocation_id;
  if (id[0] < p->count)
    p->data[id[0]] *= 2.0f;
}
```

* `__compute__`, `__vertex__`, `__fragment__` generate entry wrappers.
* `__workgroup_size__(x, y, z)` requires positive compile-time dimensions.
* `__binding__(set, binding)` has exactly two positional unsigned arguments.
  Uniform entry pointers require a binding; see `examples/uniform.cpp`.
* `__input__(location)` and `__output__(location)` expose stage interfaces.
  Outputs use pointers. Scalar inputs can be passed by value.
* `__storage__` pointers use PhysicalStorageBuffer device addresses. Arithmetic,
  increment/decrement and dereference use native C++ expressions. Address and
  element count are explicit fields in application-owned records.
* `__uniform__` and `__push_constant__` pointees are read-only.
  `__shared__` maps to workgroup address space.
* `__align__(N)` requires a positive power of two. Alignment facts on pointers
  loaded from immutable uniform/push records reach memory operations and weaken
  with pointer arithmetic. Guarantees on all other declaration contexts still
  need implementation.
* C++ records retain their LLVM target ABI layout. Reflection includes sizes,
  alignment, member offsets, array strides and device-address fields. Validation
  currently uses scalar block layout and reflection conservatively requires
  `scalarBlockLayout`.
* Scope variables are compiler-synthesized const scalar values or actual C
  arrays. Array reference deduction and decay work; `.x/.y/.z/.w` are shorthands
  on the named builtin arrays. Values are runtime values, not C++ constexpr.
  The catalog is `clang/include/clang/Basic/SPIRVShaderBuiltins.def`; availability
  for individual stages currently relies on SPIR-V validation.
* `vertex_idx`, `instance_idx`, `primitive_idx`, and other index-valued names
  use the `_idx` suffix. Identifier-valued names retain `_id`.
* `ImageFormat` is a compiler-synthesized scoped enum with SPIR-V image format
  values and an unsigned underlying type; it supports template arguments.
* Headerless scalar float math: `sqrtf`, `sinf`, `cosf`, `expf`, `exp2f`, `logf`,
  `log2f`, `fabsf`, `floorf`, `ceilf`, `truncf`, `fminf`, `fmaxf`, `fmaf`.

`-O2` is covered by a validation and runtime regression. Shader-specific
legalization avoids pointer transformations that discard SPIR-V element types.
Complex pointer casts and byte-addressing need more coverage.

## Remaining agreed work

These are required parts of the project, not exclusions from its scope:

* Compiler-native resource templates: sampled/storage textures, samplers,
  descriptor arrays and nonuniform access, acceleration structures and ray
  queries. The descriptor-indexing extension is enabled in the profile, but the
  public resource types and operations are not implemented.
* Fixed-array/homogeneous-record vector and matrix intrinsic marshalling,
  matrix operations, and the host math mode, without public vector/matrix types.
* Full texture, atomic, subgroup, synchronization, fragment, mesh and ray
  intrinsic families and specialization constants.
* Tessellation, task, mesh and six ray-stage backend lowering and execution
  modes. Their keywords parse, but compilation is currently diagnosed as
  unsupported rather than routed into unsupported backend code.
* Complete builtin catalog, stage-aware semantic checks, full layout diagnostics,
  physical pointer alias/null handling, complete reflection feature inference,
  device limits, specialization reflection and source member names.
* Direct SPIR-V extended inline assembly, including `r/v/m/i` constraints,
  named operands, `%t[name]`, `%a[name]`, local IDs, effects and module hoisting.
  It is explicitly diagnosed as unimplemented; it is not emitted as OpAsmINTEL.
* Port and inventory the applicable Slang tests at the pinned revision in
  `dependencies.json`, retaining their license and traceability. No Slang tests
  have been ported yet. The current small regression suite does not substitute
  for that corpus.
* Broader upstream regression coverage, CI, installation packaging, and Vulkan
  runtime coverage for supported advanced features.

The implementation lives in Clang parsing/Sema/CodeGen, `clang-spirv-link`, the
SPIR-V backend and target-triple handling. It is not a source preprocessor or
external C++ translator. The executable checks and examples document the current
working subset; the remaining list records the gap to the agreed final design.
