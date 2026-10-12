# Native shader C++ for Clang

This repository is an LLVM/Clang fork based on upstream
`92dfb7a5b82c63895be374da73e24a9aabbd5309` (6 October 2026).
The implementation is experimental and **does not yet implement the full agreed
language plan**. It includes a working compute, vertex and fragment compilation
path. The compute device-address example has also been executed on Mesa Vulkan.

See the [language reference](LANGUAGE.md) for attributes, types, members,
intrinsic signatures, scoping rules and implementation limits.

## Build and test

Requirements: Git, Python 3, CMake, Ninja, a modern C++ compiler, and disk space for
an LLVM build. Dependencies are pinned in `dependencies.json`. The build script
fetches the three Khronos dependencies into the ignored `.shader-deps` directory.
It builds Clang, the native shader linker, and SPIR-V Tools. No default shader
header is injected.

The SPIR-V resource headers installed with Clang provide optional C++ types.
Include `<spirv/types.hpp>` for aliases such as `spirv::u32`, `spirv::f32x3`,
and `spirv::f32x3x3`. That header includes `<spirv/vec.hpp>` and
`<spirv/mat.hpp>`, which define `spirv::vec<T, N>` and
`spirv::mat<Rows, Columns>` and their operations.
Their C-array storage retains ordinary C layout. Member functions use
snake_case.

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
The last complete run passed 658 mandatory checks; a previous compatible-device
run passed 129 optional Vulkan execution checks.
Every positive fixture is validated at default optimization and `-O2`; runtime
checks include scalar/vector/matrix math, specialization defaults and overrides,
atomics, subgroups, synchronization, pointer operations, alignment, shared memory,
linked helpers, invocation-local state and C++20 algorithms.

```sh
build/bin/clang++ -std=c++20 -fspirv-shader \
  -fspirv-entry=scale shader-cxx/examples/scale.cpp -o scale.spv
```

The shader mode defaults to `spirv1.4-unknown-vulkan1.3`. By default, each
compilation emits every entry in the translation unit into one SPIR-V module.
Use `-fspirv-entry=scale` to select only `scale`. Each output has an adjacent
`.spv.json` reflection file. Entry names follow
normal C++ linkage: `void shade()` exports `_Z5shadev`, while an `extern "C"`
entry exports `shade`. Namespaces and parameter types participate in C++
mangling. `-fspirv-entry` accepts the exact exported name or an unambiguous
qualified source name; overloaded entries require the mangled name. A
single-entry reflection file has `entry` and `source_entry` fields. A module
with multiple entries uses schema version 2 and an `entries` array containing
those fields, stage, interfaces, limits and specialization constants for each
entry. Use each entry's `entry` string as Vulkan's
`VkPipelineShaderStageCreateInfo::pName`. Module-level required features are the
union of the entries' requirements. The compiler requires `spirv-val` beside
the compiler or on PATH, and `spirv-link` when combining multiple entries. It
validates the binary against
Vulkan 1.3 before publishing it. A separate check requires a SPIR-V 1.4 header.

For reuse across translation units:

```sh
build/bin/clang++ -std=c++20 -fspirv-shader -c helpers.cpp -o helpers.bc
build/bin/clang++ -std=c++20 -fspirv-shader -c shader.cpp -o shader.bc
build/bin/clang-spirv-link helpers.bc shader.bc --entry=main -o shader.spv
```

The native linker links LLVM bitcode before lowering each entry. Reachable
helper functions are inlined. Statically known function-pointer calls
can be resolved; recursive, unresolved and dynamic indirect calls are diagnosed. Dynamic global initialization is rejected.

## Implemented language surface

```cpp
struct Params {
  __storage__ __align__(16) float *data;
  unsigned count;
};
__compute__
__workgroup_size__(
    64,
    1,
    1)
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
  Outputs use pointers. Scalar inputs can be passed by value. Inputs, outputs
  and bindings are declared on entry-point parameters; global interfaces and
  resource-bearing globals are rejected, including through aliases or records.
* `__storage__` pointers use PhysicalStorageBuffer device addresses. Arithmetic,
  increment/decrement and dereference use native C++ expressions. Address and
  element count are explicit fields in application-owned records.
* `__uniform__` and `__push_constant__` pointees are read-only.
  Workgroup scratch uses function-local `static __shared__` objects.
  Global `__shared__` declarations are rejected. Ordinary C++ global state and
  compiler-provided scope values remain available.
* `__align__(N)` requires a positive power of two. Alignment facts on pointers
  loaded from immutable uniform/push records reach memory operations and weaken
  with pointer arithmetic. Initialized local pointers and pointer parameters
  supply an initial guarantee too, including through type aliases; reassignment
  and increments weaken it. Uninitialized locals, scalar declarations, return
  types and loads from mutable pointer fields diagnose unsupported placement.
  Use ordinary `alignas(N)` for object alignment.
* Shader memory records retain their target C ABI layout. There are no std140
  or std430 packing modes. Specify padding members and `alignas(N)` explicitly
  for your data contract. Reflection includes sizes, alignment, member offsets,
  array strides and device-address fields. Vulkan validation uses scalar block
  layout and reflection requires `scalarBlockLayout`; this does not repack data.
* Scope variables are compiler-synthesized const scalar values or actual C
  arrays. Array reference deduction and decay work; `.x/.y/.z/.w` are shorthands
  on the named builtin arrays. Values are runtime values, not C++ constexpr.
  The catalog is `clang/include/clang/Basic/SPIRVShaderBuiltins.def`; availability
  for supported compute/vertex/fragment stages is checked before backend emission.
* `vertex_idx`, `instance_idx`, `primitive_idx`, and other index-valued names
  use the `_idx` suffix. Identifier-valued names retain `_id`.
* `format` is a compiler-synthesized unscoped enum with SPIR-V image format
  values and an unsigned underlying type; it supports template arguments such
  as `storage_texture_2d<float, r32f>`. Qualified names such as `format::r32f`
  also remain valid.
* Varyings are entry-point parameters with `__output__(location)` on the vertex
  entry and matching `__input__(location)` on the fragment entry. Interpolation
  defaults to perspective-correct. `__flat__` disables interpolation;
  `__noperspective__` selects screen-space linear interpolation. Floating-point
  varyings may additionally use `__centroid__` or `__sample__` to select sampling.
  Integer fragment inputs require `__flat__`. Flat cannot be combined with the
  other interpolation qualifiers; centroid and sample are mutually exclusive.
  The qualifiers emit SPIR-V decorations and appear in interface reflection.

  ```cpp
  __vertex__
  void vertex(__output__(0) __flat__ unsigned *id) { *id = vertex_idx; }
  __fragment__
  void fragment(
      __flat__ __input__(0) unsigned id,
      __output__(0) float *color) {
    *color = static_cast<float>(id);
  }
  ```

* Headerless scalar float math: `sqrtf`, `sinf`, `cosf`, `expf`, `exp2f`, `logf`,
  `log2f`, `fabsf`, `floorf`, `ceilf`, `truncf`, `fminf`, `fmaxf`, `fmaf`.

`-O2` is covered by a validation and runtime regression. Shader-specific
legalization avoids pointer transformations that discard SPIR-V element types.
Complex pointer casts and byte-addressing need more coverage.

## Native types and intrinsics

The compiler creates these declarations internally; no default header is injected.
All interfaces and resources are entry parameters with `__binding__(set, binding)`.

| Native type | Meaning |
| --- | --- |
| `texture_1d<T>`, `texture_2d<T>`, `texture_3d<T>`, `texture_cube<T>` and layered variants | Sampled images with a 32-bit scalar or homogeneous C array/record texel |
| `texture_2d_ms<T>`, `texture_2d_ms_layers<T>`, `texture_buffer<T>` | Multisampled images and texel buffers |
| `storage_texture_*<T, r32f>` | Storage images, including layers, multisampling and texel buffers |
| `sampler`, `sampler_comparison` | Opaque sampler handles |
| `Resource resources[N]`, `Resource resources[]` | Fixed and runtime descriptor arrays declared as C arrays |
| `acceleration_structure`, `ray_query` | Acceleration descriptor and invocation-local ray query, created with `make_ray_query()` |

Layered images use `_layers`, such as `texture_2d_layers<float>`. Descriptor
arrays use ordinary C arrays: `texture_2d_layers<float> images[4]` binds four
layered images. The same naming applies to storage and multisampled resources.

Math intrinsics accept C arrays and homogeneous records as operands and return
results by value: `normalize(a)`, `cross(a, b)`, `add(a, b)` and `mul(a, b)`.
Matrix `mul`, `transpose`, `inverse` and `determinant` support row-major shapes
up to four components per dimension. Component operations use overloaded names:
`min`, `max`, `clamp`, `lerp`, `add`, `sub`, `mul`, `div`, `sqrt`, `abs`, `floor`,
`ceil`, `sin`, `cos`, `exp`, `log` and `pow`. Scalar arithmetic uses C++
conversions; arithmetic array operations also support integer elements.

Array-valued results are compiler-provided aggregates. Use `result[i]` or
`matrix[row][column]`; their public `data` member is a C array, so ordinary C++
array-reference algorithms work. Record operands return the record type where
the result shape matches. Results can be copied, returned, assigned, and used as
operands of other intrinsics. There are no output-reference parameters.
`subgroup_ballot(predicate)` returns an indexable four-element unsigned value.
The compiler supplies these types and function templates without a default header.

`-fshader-host-math` supplies the same math intrinsics on a host target. The
[shared algorithm](tests/math-body.h) is compiled and numerically checked on both
host and Vulkan paths, including arrays, padded records and rectangular matrices.

Texture operations are native members with scalar arguments and return values:

```cpp
float value = image.sample_lod(s, u, v, lod);
value = image.load(x, y);
auto gathered = image.gather(s, u, v, component);
auto size = image.dimensions();
target.store(x, y, value);
```

`sample(s, coordinates...)` and `sample_grad(s, coordinates..., dx..., dy...)`
also return the texel. Array layers follow the spatial coordinates; gradients
contain only spatial components. Stores take one scalar argument per texel
component. Numeric texel types return a scalar, record texels return their
record type, and C-array texels return an indexable value with C-array `data`.
Gather always returns four components; dimensions returns unsigned components
(or a scalar for one dimension). Sampled mipmapped textures optionally accept a
mip level for `dimensions`; storage and multisampled images use `dimensions()`.
Texture handles are C++ class values: copying them, passing const references,
using `->` and calling methods on indexed descriptors work. Their representation
is private. The former output-reference and array-coordinate signatures are rejected.
Fetches currently select mip/sample zero. Descriptor accesses use
`array[index]` and are conservatively marked nonuniform;
`nonuniform(index)` supplies an explicitly decorated index.
Declare descriptor ranges as C arrays on bound entry parameters:

```cpp
__compute__
__workgroup_size__(1, 1, 1)
void shade(__binding__(0, 0) texture_2d<float> textures[4],
           __binding__(0, 1) sampler samplers[],
           __binding__(0, 2) storage_texture_2d<float, r32f> targets[4]) {
  float value = textures[0].sample_lod(samplers[0], 0.0f, 0.0f, 0.0f);
  targets[0].store(0, 0, value);
}
```

Fixed bounds are positive 32-bit constant expressions; `[]` selects a runtime
array. Normal C++ parameter adjustment applies: an array parameter becomes a
pointer, while `const texture_2d<float> (&textures)[4]` retains its bound for
array-reference template deduction and `sizeof`. Array aliases and unknown-bound
references also work. Subscripts return resource handle values, preserving
immutable bindings. Decayed range pointers support offsets, increment/decrement
and method access; range pointer comparisons and differences are diagnosed.
Descriptor array storage is borrowed from entry parameters; locally owned and
multidimensional descriptor arrays are unsupported. Reflection retains
`resource_type: "descriptor_array"` with the fixed bound or runtime count zero.
The former `descriptor_array<Resource, N>` template is no longer supplied. See the
[resource fixtures](tests/fixtures/resources.cpp).

The atomic API uses `atomic_load(pointer)`, `atomic_store(pointer, value)`,
`atomic_add`, `atomic_sub`, `atomic_exchange`, `atomic_min`, `atomic_max`, `atomic_and`,
`atomic_or`, `atomic_xor`, and `atomic_compare_exchange(pointer, compare, value)`.
These names match the image atomic methods; the former camelCase spellings
are no longer intrinsics: for example, `atomic_add(pointer, value)` and
`image.atomic_add(x, y, value)` use the same operation name. Binary operations
use `(pointer, value)`. Load returns the current value, store returns `void`, and
read-modify-write operations return the previous value. The existing signed and
unsigned 32/64-bit shared/storage integer support and memory semantics are
preserved. The former snake_case spellings are rejected.
Explicit `memory_order` and `memory_scope` arguments are available at the end of
each call; compare-exchange accepts separate success and failure orders. Vulkan
shader calls using `memory_order::seq_cst` are diagnosed.

Native device intrinsics include integer atomics, workgroup/subgroup/device memory
barriers, subgroup voting, ballot, reductions, scans, broadcasts and shuffles,
fragment derivatives/discard, and ray query traversal/intersection operations.
Ray query operations are methods of the native class:

```cpp
ray_query q = make_ray_query();
q.initialize(scene, 0, 255, 0.0f, 0.0f, 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 1000.0f);
while (q.proceed()) {
  if (q.intersection_type(0) == 0)
    q.confirm_intersection();
}
unsigned primitive = q.primitive_idx(1);
```

The other methods are `terminate()`, `generate_intersection(t)`,
`intersection_t(selector)`, `instance_custom_idx(selector)`,
`geometry_idx(selector)` and `front_facing(selector)`. Selectors are constant
`0` (candidate) or `1` (committed). Intersection getters accept const objects;
initialization and traversal mutate the query. Pointer receivers use `->`.
Copies share the underlying query state. The former `ray_query_*` free function
names are no longer exposed.
Their internal lowering names and arities (including member receivers) are listed in
[SPIRVShaderIntrinsics.def](../clang/include/clang/Basic/SPIRVShaderIntrinsics.def).

`specialization_constant(id, default)` returns a scalar of the default's type.
IDs are constant unsigned 32-bit values; repeated IDs must agree in type and
value. Reflection includes IDs, types, widths, values and exact default bits.
These are pipeline constants, not C++ constant expressions: their values cannot
be used as template arguments or array bounds. For example:

```cpp
unsigned count = specialization_constant(0, 64u);
float gain = specialization_constant(1, 1.0f);
```

Without a pipeline override, these evaluate to `64u` and `1.0f`. To override
them, populate `VkSpecializationMapEntry` entries with `constantID` 0 and 1,
the byte offsets and sizes of the corresponding values in your data block,
and attach a `VkSpecializationInfo` to the shader stage's `pSpecializationInfo`.
The compiler emits `OpSpecConstant` (or its boolean variants) decorated with
`SpecId`; ordinary uses of the returned values remain specialization-dependent.
Vulkan execution tests cover defaults and integer/float/boolean overrides.

## Remaining agreed work

These are required parts of the project, not exclusions from its scope:

* Additional texture operations: comparison sampling, sparse results, dynamic offsets, storage formats with 64-bit texels and input attachments. Constant-offset sampling and gather methods are available.
  `sampler_comparison` is a native type; comparison sampling is still pending.
* Mesh emission and stage-specific ray tracing intrinsics.
* Tessellation, task, mesh and six ray-stage backend lowering and execution
  modes. Their keywords parse, but compilation is currently diagnosed as
  unsupported rather than routed into unsupported backend code.
* Complete advanced-stage builtin catalog, full layout diagnostics, arbitrary
  pointer reinterpretation and source member names in reflection. Supported
  stages have scope diagnostics; physical pointer nulls and nested address
  records are covered by numeric tests. Reflection reports used optional
  features and push/shared/workgroup limits, which the runtime harness checks
  against the selected device.
* Port and inventory more applicable Slang tests at the pinned revision in
  `dependencies.json`, retaining their license and traceability. One Slang test
  has been ported. The current small regression suite does not substitute
  for that corpus.
* Broader upstream regression coverage, CI, installation packaging, and Vulkan
  runtime coverage for supported advanced features.

The implementation lives in Clang parsing/Sema/CodeGen, `clang-spirv-link`, the
SPIR-V backend and target-triple handling. It is not a source preprocessor or
external C++ translator. The executable checks and examples document the current
working subset; the remaining list records the gap to the agreed final design.
