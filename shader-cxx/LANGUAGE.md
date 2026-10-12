# Shader C++ language reference

This document describes the additions and restrictions of this repository's
Clang C++20 shader mode. It documents the current implementation, including
unfinished parts; it is not a specification that all planned features already
work. The target is **Vulkan 1.3 with SPIR-V 1.4**.

The extensions are enabled by `-fspirv-shader`. Types, functions and scope values
are synthesized by the compiler: **no default header is required or injected**.
Ordinary C++ without that flag does not acquire shader attributes or resources.
`-fshader-host-math` separately enables the reusable math subset on a host target.

## Contents

- [Relationship to C++](#relationship-to-c)
- [Entry points and attributes](#entry-points-and-attributes)
- [Interfaces, varying values and scope](#interfaces-varying-values-and-scope)
- [Pointers, memory spaces and alignment](#pointers-memory-spaces-and-alignment)
- [Compiler-provided scope values](#compiler-provided-scope-values)
- [Arrays, records and mathematical results](#arrays-records-and-mathematical-results)
- [Math intrinsics](#math-intrinsics)
- [Resources and descriptor arrays](#resources-and-descriptor-arrays)
- [Texture members](#texture-members)
- [Image formats](#image-formats)
- [Atomics, barriers and subgroups](#atomics-barriers-and-subgroups)
- [Fragment intrinsics](#fragment-intrinsics)
- [Ray queries](#ray-queries)
- [Specialization constants](#specialization-constants)
- [Compilation, reflection and device requirements](#compilation-reflection-and-device-requirements)
- [Current limits and verification](#current-limits-and-verification)

## Relationship to C++

Templates, concepts, namespaces, overload resolution, classes, constructors,
members, member functions, references, lambdas, loops, `constexpr`, `consteval`
and `alignas` retain their C++ roles. Reachable code must also be expressible by
the implemented shader lowering. C++ parsing alone does not establish shader
support for a library operation or language construct.

There is no additional public `vector` or `matrix` class. Use C arrays or
homogeneous user records for math. There are no `Input<T>`, `Output<T>`, buffer
wrapper or push-constant wrapper types: entry attributes, qualified pointers and
ordinary C++ records express those roles.

| Added construct | Relationship to C++ |
| --- | --- |
| Entry and interface attributes | Mark functions and parameters as shader entry interfaces |
| Memory-space qualifiers | Distinguish logical memory and physical buffer-device-address pointees |
| Builtin scope names | Read-only compiler-provided scalar objects and C arrays, accessed like CUDA scope values |
| Math/device intrinsics | Compiler-provided functions with checked operand and result shapes |
| Texture and `ray_query` types | Compiler-provided classes with native methods and private representation |
| Sampler and acceleration handles | Opaque compiler-provided pointer-like types |
| Mathematical result objects | Compiler-provided aggregates with a public C-array `data` member and indexing |
| `format` | A compiler-provided **unscoped enum** used in storage texture templates |
| Resource C-array parameters | Borrow fixed or runtime descriptor ranges from entry bindings |
| `specialization_constant` | Produce pipeline-specializable scalar values, not C++ constant expressions |

The shader ABI uses 64-bit pointers, `long`, `long long` and `size_t`; `int` and
`unsigned` are 32-bit. Query actual layout with normal `sizeof`, `alignof` and
`__builtin_offsetof`, and use reflection when preparing host data. Host ABI
agreement must be checked explicitly when sharing layouts.

Shader memory records use the target C ABI. There are no `std140`, `std430` or
other shader packing modes, and the compiler does not rearrange fields or add
padding to satisfy those modes. Specify your data contract with explicit padding
members and `alignas(N)`, then verify it with `sizeof`, `alignof` and
`__builtin_offsetof`. `scalarBlockLayout` is a Vulkan validation/device requirement
for representing these offsets; it is not a layout transformation.

```cpp
struct alignas(16) Material {
  float color[3];
  float opacity;
  float roughness;
  unsigned padding[3];
};
static_assert(sizeof(Material) == 32);
static_assert(alignof(Material) == 16);
static_assert(__builtin_offsetof(Material, opacity) == 12);
static_assert(__builtin_offsetof(Material, roughness) == 16);
static_assert(__builtin_offsetof(Material, padding) == 20);
```

The C++ term *standard-layout* below is a record eligibility constraint, not a
shader buffer packing mode. Ordinary C ABI alignment rules still apply.

The optional [scalar](../clang/lib/Headers/spirv/types.hpp),
[vector](../clang/lib/Headers/spirv/vec.hpp), and
[matrix](../clang/lib/Headers/spirv/mat.hpp) utility headers ship in Clang's SPIR-V
resource-header set. `<spirv/types.hpp>` includes the implementation headers
and defines all scalar, vector, and matrix aliases in `namespace spirv`.
`<spirv/vec.hpp>` and `<spirv/mat.hpp>` independently define C-array-backed
`spirv::vec<T, N>` and row-major `spirv::mat<Rows, Columns>` templates and
their operators. They are explicitly included, never injected by the compiler.
Vectors have indexed and
named component access (`.x()` through `.w()`), generic swizzles and every
two- to four-component named `xyzw` swizzle. Named swizzles return values;
`set_swizzle` writes components. Matrix operators call `mul` and support
rectangular dimensions. Their data retains ordinary C layout: `f32x3` is 12
bytes, `f32x3x3` is 36 bytes, and neither type adds padding or alignment. Add
`alignas` to a containing record only when its host ABI contract requires it.
Vector and matrix accessors preserve Uniform, PushConstant, and Storage
address spaces: reads and swizzles work directly on resource-backed values,
while component and swizzle writes work on writable receivers, including Storage.
Scalar component references and matrix row references retain the receiver's
address space.
Native floating-point math uses matching vector-typed SPIR-V instructions
where available, including `FMix` for `lerp`/`mix`,
`FClamp`/`UClamp`/`SClamp` for `clamp`, and vector derivative instructions.
`lerp`/`mix` follows the `FMix` blend `a * (1 - weight) + b * weight`;
floating-point rounding can differ from `a + (b - a) * weight`. `clamp` uses
the SPIR-V clamp instructions rather than composing the separate `min`/`max`
intrinsics. `mod` uses `OpFMod`. Matrix `mul` expands to component
multiply-adds; it does not currently emit native `OpMatrixTimes*` instructions.

The preferred names encode scalar width and dimensions. `<spirv/types.hpp>` defines
`u8`/`u16`/`u32`/`u64`, `i8`/`i16`/`i32`/`i64`, `f16`/`f32`/`f64`, and `b8`
(an alias for `bool`). It also defines two-, three-, and four-component
aliases such as `u32x2`, `i32x4`, and `f32x3`. It defines every `f32xRxC`
alias for R and C from 2 to 4,
including `f32x3x3` and `f32x4x4`, as well as `b8x2`/`b8x3`/`b8x4`.
GLSL-style `vec2`/`ivec2`/`uvec2`/`bvec2` and `mat2`/`mat3`/`mat4` aliases
are not part of the public headers; the Shadertoy translator emits the
width-explicit names.
The public vector and matrix member functions use snake_case. Helpers such as
`less_than` and `smoothstep` live in `namespace spirv`; the Shadertoy
compatibility layer exposes them through `namespace glsl`.

The scalar aliases cover the implemented integer widths (8, 16, 32, 64) and
IEEE floating-point widths (16, 32, 64). There is no `f8` alias or native
float8 lowering. The [SPIR-V float8 extension](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/EXT/SPV_EXT_float8.asciidoc)
defines E4M3 and E5M2 encodings, but these are outside this compiler's current
Vulkan shader type surface. The aliases also do not cover every extension type,
such as bfloat16 or cooperative matrices; `mat` currently stores `f32` only.

Stage interface records and C arrays of two to four identical floating-point or
32-/64-bit integer components
marshal as SPIR-V vectors, so their components share a shader location. Nested
record fields retain their vector grouping. Reflection describes the original
C++ layout; the binary uses the vector stage interface. Uniform, push-constant
and storage-buffer records keep their memory layout.

Grouped interface records do not receive implicit member locations. Annotate each
scalar/vector field explicitly with postfix `__location(N)` and omit the
location argument on the entry's `__input`/`__output` annotation:

```cpp
struct Varyings {
  spirv::f32x3 normal __location(7);
  spirv::f32x2 uv __location(3);
};
__vertex
void vertex(__output Varyings *varyings,
            __position spirv::f32x4 *position);
```

Nested record containers require explicitly located fields as well. The compiler
lowers those fields to separate SPIR-V interface variables at the specified
locations, independent of declaration order. Missing locations, overlapping
location ranges and a parameter location on a grouped record are rejected.
Vector-shaped records still use their parameter's explicit location, e.g.
`__input(3) spirv::f32x2 uv`; vector components are not separate varyings.
Builtin interfaces such as Position remain separate and do not consume locations.

Interpolation qualifiers may be postfixed on located fields:

```cpp
struct FragmentInputs {
  unsigned id __location(0) __flat;
  spirv::f32x2 uv __location(1) __varying;
  spirv::f32x2 screen_uv __location(2) __noperspective __centroid;
};
```

`__varying` explicitly selects the default perspective-correct interpolation;
it does not declare input/output direction and emits no additional SPIR-V
decoration. `__flat` disables interpolation. `__noperspective` selects
screen-space linear interpolation. `__centroid` and `__sample` select the
sampling position and can accompany perspective or non-perspective interpolation.
These apply to fragment inputs and rasterization-stage outputs (vertex, geometry,
tessellation evaluation and mesh), not vertex inputs or memory buffers.

Parameter qualifiers provide defaults for all members. Nested record-container
qualifiers provide defaults for their descendants. An explicit field interpolation
mode overrides the enclosing mode; an explicit sampling qualifier overrides the
enclosing sampling qualifier. A flat override drops inherited centroid/sample
sampling. Conflicting qualifiers on the same declaration are rejected, as are
incompatible inherited combinations. Interpolated values must be floating-point;
integer and double-precision fragment inputs must resolve to flat interpolation.
Qualified components
of a vector-shaped record cannot silently become separate varyings: use explicit
field locations instead.

For example, clip-space position is an entry output rather than a global:

```cpp
__vertex
void vertex_position(__position float (*position)[4]) {
  (*position)[0] = 0.0f;
  (*position)[1] = 0.0f;
  (*position)[2] = 0.0f;
  (*position)[3] = 1.0f;
}
```

### Classes and lookup

User classes and their member functions use ordinary C++ lookup. Math record
operands must have a homogeneous supported component shape, be standard-layout,
have no bases, unions or bitfields, and satisfy the relevant size constraints.
Record entry parameters passed by value additionally require trivially copyable,
standard-layout records without bases or unions. Arbitrary user class behavior
is not a shader interface ABI.

Trivial record copy/move assignment to qualified shader memory preserves the
receiver's address space, including the reference returned by assignment.
Standard-layout records without bases, unions or bitfields, and their array
members, copy through typed field loads/stores. This supports `*output = color`
and chained field writes. Read-only destinations still reject assignment;
user-defined assignment methods retain normal address-space requirements.

Shader C++ accepts explicit-object member functions in C++20 mode to let
helpers deduce the receiver's address space. The `spirv::vec` accessors and
swizzles, and `spirv::mat::operator[]`, use this to read uniform and
push-constant objects without copying them into local memory. Writable vector
accessors also preserve storage addresses. Ordinary C++20 compilation without
`-fspirv-shader` retains the standard C++23 requirement for explicit-object
members.

Native texture classes support copying, assignment, return by value, passing
`const` references, templates and calls through object pointers. Copies refer
to the same resource; copying a texture does not copy image contents. Their
representation is private; there is no public handle member. `ray_query` copies
share query state. Result aggregates expose `data`, unlike resource classes.

Public free intrinsics participate in C++ overload resolution. A user overload
such as `int normalize(int)` can be selected normally. A qualified call to
`functions::normalize` resolves in that namespace and does not become a shader
intrinsic because of its spelling. The legacy scalar `*f` math aliases follow
Clang builtin behavior; do not assume the same global redeclaration behavior as
the synthesized intrinsic templates. Qualified user namespace functions remain
ordinary functions for all these names.

Pure component math intrinsics such as `sin`, `sqrt`, `add`, and `clamp` can be
used directly in constant expressions, including with homogeneous records.
Bit operations, packing/unpacking, floating decomposition, and extended integer
arithmetic also support constant inputs where their operations are defined.
Their runtime calls still lower to SPIR-V intrinsics. Resource, derivative,
subgroup, and synchronization operations require runtime execution.

## Entry points and attributes

Examples below use `void` entry functions with outputs supplied through entry
parameters. A function's C++ return value is not a shader stage output.

```cpp
struct Params {
  __storage float *values;
  unsigned count;
};

__compute
__workgroup_size(64, 1, 1)
void scale(__push_constant const Params *params) {
  unsigned i = global_invocation_id.x;
  if (i < params->count)
    params->values[i] *= 2.0f;
}
```

| Spelling | Arguments | Placement and meaning |
| --- | --- | --- |
| `__compute` | None | Compute entry function; requires workgroup size |
| `__vertex` | None | Vertex entry function |
| `__fragment` | None | Fragment entry function; emitted with upper-left fragment origin |
| `__early_fragment_tests` | None | Fragment entry with early depth/stencil tests |
| `__post_depth_coverage` | None | Fragment entry whose coverage input reflects the post-depth-test mask; requires `SPV_KHR_post_depth_coverage` |
| `__fragment_interlock` | One `interlock_mode` constant | Select ordered or unordered pixel, sample, or shading-rate interlock |
| `__noncoherent_color_attachment_read`, `__noncoherent_depth_attachment_read`, `__noncoherent_stencil_attachment_read` | None | Fragment tile reads without rasterization-order guarantees for the selected attachment |
| `__depth_greater`, `__depth_less`, `__depth_unchanged` | None | Mutually exclusive conservative-depth hints on fragment entries with a `__frag_depth` output |
| `__stencil_ref` | None | One fragment entry output pointer to a 32-bit integer stencil reference; requires `SPV_EXT_shader_stencil_export` |
| `__workgroup_size(x, y, z)` | Three positive constant unsigned 32-bit dimensions | Compute function; dimensions are recorded in execution mode and reflection |
| `__workgroup_size_id(x_id, x_default, y_id, y_default, z_id, z_default)` | Three unsigned 32-bit IDs and three positive defaults | Workgroup entry; emits `LocalSizeId` using scalar specialization constants |
| `__binding(set, binding)` | Exactly two constant unsigned 32-bit values | Bound descriptor/uniform entry parameter |
| `__input(location)` | One constant unsigned 32-bit location, or omitted for explicitly located record members | Read-only stage input entry parameter |
| `__output(location)` | One constant unsigned 32-bit location, or omitted for explicitly located record members | Stage output entry parameter; must be a pointer |
| `__location(location)` | One constant unsigned 32-bit location | Postfix record-field annotation for grouped stage inputs/outputs |
| `__position` | None | Vertex-processing Position pointer to `float[4]` or a matching record; per-vertex arrays for advanced stages, const for inputs; no location or interpolation qualifier |
| `__frag_depth` | None | One fragment entry output pointer to mutable `float`; emits BuiltIn FragDepth and DepthReplacing |
| `__sample_mask` | None | Fragment input `const unsigned (*)[1]` or output `unsigned (*)[1]` (at most one of each); emits BuiltIn SampleMask |
| `__point_size` | None | One vertex entry output pointer to mutable `float`; emits BuiltIn PointSize |
| `__clip_distance` | None | Vertex output `float (*)[N]` or fragment input `const float (*)[N]`; emits BuiltIn ClipDistance and requires Vulkan `shaderClipDistance` |
| `__cull_distance` | None | Vertex output `float (*)[N]` or fragment input `const float (*)[N]`; emits BuiltIn CullDistance and requires Vulkan `shaderCullDistance` |
| `__flat` | None | Rasterization-stage output or fragment input parameter/record field; disables interpolation |
| `__varying` | None | Floating rasterization-stage output or fragment input parameter/record field; explicit default perspective-correct interpolation |
| `__noperspective` | None | Floating rasterization-stage output or fragment input parameter/record field; screen-space linear interpolation |
| `__centroid` | None | Floating varying parameter/record field; centroid sampling |
| `__sample` | None | Floating varying parameter/record field; sample interpolation and corresponding feature requirement |
| `__storage` | None | Type qualifier for physical buffer-device-address memory |
| `__uniform` | None | Type qualifier for read-only uniform memory |
| `__push_constant` | None | Type qualifier for read-only push-constant memory |
| `__shared` | None | Type qualifier for workgroup memory |

Annotations have a double-underscore prefix and no trailing underscores.
Arguments are positional: use `__binding(0, 3)`, with both arguments.
Defaults and named forms such as `binding=3` are not an
extension. Conflicting stages, workgroup dimensions, memory spaces, bindings or
interpolation qualifiers are diagnosed. Workgroup dimensions must also fit the
selected device's limits; a valid attribute alone does not establish that.

### Advanced stages

Geometry, tessellation, EXT task/mesh and all six KHR ray-tracing stages lower
to native SPIR-V execution models. Select each entry independently as usual.

| Stage | Required configuration |
| --- | --- |
| `__geometry` | `__geometry_mode(geometry_input::triangles, geometry_output::triangle_strip, max_vertices)`; optional `__geometry_invocations(count)` |
| `__tessellation_control` | `__output_vertices(count)` |
| `__tessellation_evaluation` | `__tessellation_mode(tessellation_domain::triangles, tessellation_spacing::equal, vertex_order::counter_clockwise)` |
| `__task` | `__workgroup_size(x, y, z)` |
| `__mesh` | Workgroup size and `__mesh_output(mesh_topology::triangles, max_vertices, max_primitives)` |
| `__ray_generation`, `__intersection`, `__any_hit`, `__closest_hit`, `__miss`, `__callable` | No workgroup dimensions |

Geometry input modes also include points, lines and adjacency variants; output
modes include points and line strips. Tessellation domains include quads and
isolines; spacing also supports fractional-even/odd, with either vertex order.
Mesh topology also supports points and lines. Raw SPIR-V mode numbers are
accepted in these annotations.

`__position` accepts per-vertex arrays for geometry/tessellation inputs and
tessellation-control/mesh outputs. Const pointers denote inputs; writable
pointers denote outputs. `__tess_level_outer` and
`__tess_level_inner` expose float[4]/float[2] patch interfaces with `__patch`.
`__tess_coord` exposes the three-float tessellation-evaluation input.
`__primitive_id`, `__invocation_id`, `__layer`, `__viewport_index`,
and `__patch_vertices` expose integer builtin interfaces where supported by
the stage and direction. Mesh primitive index arrays use
`__primitive_point_indices`, `__primitive_line_indices`, or
`__primitive_triangle_indices`. `__cull_primitive` accepts a bool array;
interface marshalling uses SPIR-V booleans. Builtin interface qualifiers take
no arguments; numeric builtin annotations and the `shader_builtin` enum are not
supported.
`__per_primitive` marks mesh outputs or fragment inputs.

Builtin interface semantics also accept equivalent homogeneous records, including
`spirv::vec` values and nested record wrappers. Records must be standard-layout,
trivially copyable, and contain exactly the required scalar components with no
padding, bases, unions or bitfields. For example, Position accepts a record of
four separate `float` fields or `spirv::f32x4`; FragDepth and PointSize accept a
single-float record; SampleMask accepts a single-`unsigned` record; and tessellation
levels accept `f32x4`/`f32x2` records. Per-vertex/per-primitive arrays retain their
outer array dimension; a single-field record wrapping that array is also accepted.
Stage, direction, component type/count and qualifier requirements still apply.
The compiler marshals these C++ layouts to the semantic's native SPIR-V type:
Position/TessCoord remain vectors, sample masks/distances/tessellation levels remain
arrays, and depth/point-size/stencil references remain scalars.

Payload/record pointer parameters use `__task_payload`, `__ray_payload`,
`__incoming_ray_payload`, `__hit_attribute`, `__callable_data`,
`__incoming_callable_data`, and `__shader_record`. Task payloads in mesh,
hit attributes in hit shaders, and shader records must be const.

Stage operations are headerless:

| Operation | Stage / behavior |
| --- | --- |
| `emit_vertex()`, `end_primitive()` | Geometry emission |
| `emit_stream_vertex(stream)`, `end_stream_primitive(stream)` | Geometry; constant stream 0..3 |
| `set_mesh_outputs(vertices, primitives)` | Mesh; call uniformly before output writes |
| `emit_mesh_tasks(x, y, z[, payload])` | Task; terminates the invocation |
| `trace_ray(scene, flags, mask, sbt_offset, sbt_stride, miss_index, origin, tmin, direction, tmax, payload)` | Ray generation, closest-hit or miss; float3 records/arrays and a ray-payload interface pointer |
| `execute_callable(index, data)` | Ray generation, closest-hit, miss or callable; callable-data interface pointer |
| `report_intersection(distance, kind)` | Intersection; returns bool |
| `ignore_intersection()`, `terminate_ray()` | Any-hit; terminate the invocation |

Payload operands must name entry interface variables, not subobjects or
invocation-local copies. Task/mesh and ray stages reflect the required Vulkan
features. Compilation/validation coverage is not hardware rendering or ray
traversal coverage.

## Interfaces, varying values and scope

An entry parameter owns each shader input, output, uniform, push-constant block
or descriptor binding. Global shader interfaces and resource-bearing globals
are rejected, including namespace declarations, static members, aliases, arrays
and nested records. Attributes on local declarations do not create interfaces.

Input scalars and supported records can be passed by value. Pointer inputs refer
to read-only input memory. Outputs use pointers. Uniform and push-constant
pointees are read-only even when a redundant explicit `const` is omitted.
A uniform parameter needs `__binding`; a push-constant parameter does not.
An entry can contain at most one push-constant interface.

A varying is an output location on the vertex entry matched to an input location
on the fragment entry. There is no separate varying class or keyword.

```cpp
__vertex
void vertex(__output(0) __flat unsigned *id) {
  *id = static_cast<unsigned>(vertex_idx);
}

__fragment
void fragment(__input(0) __flat unsigned id, __output(0) float *color) {
  *color = static_cast<float>(id);
}
```

Default varying interpolation is perspective-correct. Integer fragment inputs,
including integer components inside records, require `__flat`. Flat cannot be
combined with no-perspective, centroid or sample qualifiers. Centroid and sample
are mutually exclusive. Non-flat interpolated values must be floating-point.
The compiler's required Flat decorations for integer **builtin** fragment inputs
are separate from this user-facing varying rule.

Ordinary global C++ state with constant initialization can be used: it is
materialized as invocation-local state, rather than a persistent GPU global.
Mutations do not persist between invocations. Dynamic global initialization and
escaping global addresses are rejected. Function-local `static __shared`
objects are the explicit workgroup-sharing exception.

Helpers use normal C++ scopes and parameters. Interfaces stay on the entry;
helper references or pointers borrow those values. Reachable helpers are linked
and inlined. Recursion, unresolved functions and unresolved dynamic indirect
calls are diagnosed. Statically known function-pointer calls can be resolved.
A host runtime, heap allocator, exception runtime or virtual-dispatch runtime is
not supplied automatically.

## Pointers, memory spaces and alignment

The qualifier belongs to the pointee type, not to the variable holding a pointer.

| Declaration/qualifier | Storage model | Access |
| --- | --- | --- |
| Ordinary local `T *` | Logical invocation-local/Function memory unless initialized from an address-space-qualified pointer | The initializer's pointee address space is deduced when the pointee types match; otherwise normal local C++ pointer access |
| `__storage T *` | SPIR-V PhysicalStorageBuffer | Buffer device address; writable unless pointee is const |
| `__uniform T *` | Uniform block, descriptor binding | Read-only |
| `__push_constant T *` | Push-constant block | Read-only |
| `static __shared T object` inside a function | Workgroup memory | Shared by invocations in that workgroup |
| Pointer parameter with `__input` | Input interface storage | Read-only |
| Pointer parameter with `__output` | Output interface storage | Writable |
| Native resource/descriptor-range pointer | Borrowed native descriptor handle/range | Resource-specific behavior; not a physical buffer address |

```cpp
struct Addresses {
  __storage float *values[2];
  __storage float *__storage *pointer_table;
};

void local_example() {
  float values[2] = {1.0f, 2.0f};
  float *p = values;
  ++p;
  *p = 3.0f;
}
```

Physical storage supports dereference, indexing, `->`, offsets, increment and
decrement, pointer difference/comparison, null comparison and selection, and
nested address fields/arrays and physical pointer tables. Arithmetic is scaled
by pointee size. Applications provide the device addresses and accessible element
counts. An initialized local pointer with an unqualified pointee, such as
`float *p = storage_pointer`, deduces the initializer's pointee address space.
This preserves `const` and the declared pointee type; it does not convert or
copy the pointed-to data. Uninitialized locals and fixed `T *` function
parameters remain invocation-local, so use an explicitly qualified parameter
or a deduced/template parameter for helpers that accept other spaces.
Address-space-qualified pointees remain distinct C++ types.

Supported local logical pointer choices are lowered to control flow so only the
selected memory access executes. Finite local allocation/array-offset loops are
supported. Dynamic local record-field offsets, unsupported address bases and
escapes can be diagnosed. Arbitrary reinterpretation and byte addressing do not
have complete support; this reference does not promise every legal C++ pointer
cast lowers to a legal shader.

### Object alignment

Use standard `alignas(N)` to align a C++ object or record. It affects object
layout and reflection. It does not guarantee alignment of an arbitrary address
stored in a pointer member; device-address accesses use alignment derived from
the pointee type and pointer arithmetic. There is no shader-specific pointer
alignment annotation.

## Compiler-provided scope values

These are read-only objects in global lookup, with the types below. They are
runtime invocation values, **not C++ constexpr values**. Builtin arrays support
ordinary `[i]`, array references and decay, plus `.x`, `.y`, `.z`, `.w` shorthands
for components that exist. Those shorthands are compiler syntax on named builtin
arrays, not members added to every C array.

```cpp
void scope_example() {
  unsigned x = global_invocation_id.x;
  const unsigned (&id)[3] = global_invocation_id;
  unsigned y = id[1];
}
```

`workgroup_size` is synthesized from the compute entry's declared dimensions.
The compiler checks the stage availability of used scope values before emission.
Device feature/capability requirements may apply even when a stage is listed.

| Name | C++ type | Currently supported stages |
| --- | --- | --- |
| `global_invocation_id` | `const unsigned[3]` | compute |
| `local_invocation_id` | `const unsigned[3]` | compute |
| `workgroup_id` | `const unsigned[3]` | compute |
| `num_workgroups` | `const unsigned[3]` | compute |
| `workgroup_size` | `const unsigned[3]` | compute |
| `local_invocation_idx` | `const unsigned` | compute |
| `subgroup_size` | `const unsigned` | compute, vertex, fragment |
| `num_subgroups` | `const unsigned` | compute |
| `subgroup_id` | `const unsigned` | compute |
| `subgroup_invocation_id` | `const unsigned` | compute, vertex, fragment |
| `vertex_idx` | `const int` | vertex |
| `instance_idx` | `const int` | vertex |
| `primitive_idx` | `const int` | fragment |
| `invocation_id` | `const int` | Geometry or tessellation control |
| `patch_vertices_in` | `const int` | Tessellation control/evaluation |
| `tess_coord` | `const float[3]` | Tessellation evaluation |
| `tess_level_outer` | `const float[4]` | Tessellation evaluation |
| `tess_level_inner` | `const float[2]` | Tessellation evaluation |
| `frag_coord` | `const float[4]` | fragment |
| `point_coord` | `const float[2]` | fragment |
| `front_facing` | `const bool` | fragment |
| `sample_id` | `const int` | fragment |
| `sample_position` | `const float[2]` | fragment |
| `bary_coord`, `bary_coord_noperspective` | `const float[3]` | fragment; requires `fragmentShaderBarycentric` |
| `shading_rate` | `const unsigned` | fragment; requires `pipelineFragmentShadingRate` |
| `helper_invocation` | `const bool` | fragment |
| `view_idx` | `const unsigned` | vertex, fragment |
| `base_vertex` | `const int` | vertex |
| `base_instance` | `const int` | vertex |
| `draw_idx` | `const int` | vertex |
| `launch_id` | `const unsigned[3]` | All ray-tracing stages |
| `launch_size` | `const unsigned[3]` | All ray-tracing stages |
| `world_ray_origin` | `const float[3]` | Intersection, any-hit, closest-hit or miss |
| `world_ray_direction` | `const float[3]` | Intersection, any-hit, closest-hit or miss |
| `object_ray_origin` | `const float[3]` | Intersection, any-hit or closest-hit |
| `object_ray_direction` | `const float[3]` | Intersection, any-hit or closest-hit |
| `ray_tmin` | `const float` | Intersection, any-hit, closest-hit or miss |
| `ray_tmax` | `const float` | Intersection, any-hit, closest-hit or miss |
| `instance_custom_idx` | `const int` | Intersection, any-hit or closest-hit |
| `hit_kind` | `const unsigned` | Any-hit or closest-hit |
| `incoming_ray_flags` | `const unsigned` | Intersection, any-hit, closest-hit or miss |
| `ray_geometry_idx` | `const int` | Intersection, any-hit or closest-hit |

## Arrays, records and mathematical results

Signature notation used below:

| Symbol | Meaning |
| --- | --- |
| `S` | Arithmetic scalar type after conversions, or an explicitly constrained scalar type |
| `E` | Common component type of an aggregate math operand |
| `V` | C array or homogeneous record with 2–4 components |
| `M<R,C>` | Row-major floating-point C array `E[R][C]`, with each dimension 1–4 |
| `Result<E,N>` | Not a public template name: notation for a compiler result aggregate with `E data[N]` |
| `MatrixResult<E,R,C>` | Not a public template name: result aggregate with `E data[R][C]` |

A mathematical result has a public `data` C array. `result[i]` is compiler-supported
shorthand for `result.data[i]`; matrix results support `[row][column]`. These are
aggregate result objects, not C arrays returned directly by a C++ function.
Copying, returning, assigning, chaining and borrowing their `data` arrays work.

Vector-shaped operations return the first operand's record type when that
operand is a compatible user record. Otherwise they return a compiler aggregate.
Matrix operations return matrix result aggregates. Scalar-producing operations
return the component scalar. Matrix `mul` means matrix multiplication; vector
`mul` and scalar `mul` mean component multiplication.

```cpp
struct Vec3 {
  float x, y, z;
};

void math_example() {
  float a[3] = {1.0f, 2.0f, 3.0f};
  auto n = normalize(a);
  float (&components)[3] = n.data;
  components[0] = n[1];

  Vec3 v{1.0f, 2.0f, 3.0f};
  Vec3 normalized = normalize(v);
  float matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
  auto transposed = transpose(matrix);
  float (&rows)[3][2] = transposed.data;
}
```

## Math intrinsics

All functions in this section return values. There are no output-reference
parameters. Floating aggregate operations require matching component types and
shapes; there is no implicit conversion of a `float[3]` into a `double[3]`.
Compatible arrays and homogeneous records can supply the same component shape.

### Scalar and component overloads

Scalar overloads use C++ arithmetic conversions, including integer promotion.
For integer inputs to transcendental/floating functions, the scalar result type
is `double`. For example, `min(short(1), short(2))` returns `int`, `min(1, 2.0f)`
returns `float`, and `sqrt(4)` returns `double`.

Aggregate overloads use the common component type `E`; the first operand decides
record versus result-object representation. Integer/bool aggregate operations
are supported only for the arithmetic/min/max/clamp/abs families listed below.
Boolean aggregate arithmetic promotes operands before computation and normalizes
the result back to `bool`; scalar boolean arithmetic follows ordinary promotion.

| Function | Parameters | Scalar return | Aggregate support |
| --- | --- | --- | --- |
| `add` | `(a, b)` | Promoted common `S` | `V, V`; floating or integer/bool components |
| `sub` | `(a, b)` | Promoted common `S` | `V, V`; floating or integer/bool components |
| `mul` | `(a, b)` | Promoted common `S` | `V, V`; floating or integer/bool components; matrix overload below |
| `div` | `(a, b)` | Promoted common `S` | `V, V`; floating or integer/bool components |
| `min` | `(a, b)` | Promoted common `S` | `V, V`; floating or integer/bool components |
| `max` | `(a, b)` | Promoted common `S` | `V, V`; floating or integer/bool components |
| `clamp` | `(value, lower, upper)` | Promoted common `S` | Three matching `V`; floating or integer/bool components |
| `abs` | `(value)` | Promoted `S` | `V`; floating or integer/bool components |
| `lerp` | `(a, b, weight)` | Common floating `S` | Three matching floating `V`; aggregate weight is component-wise |
| `sqrt` | `(value)` | Floating `S` | Floating `V` |
| `floor` | `(value)` | Floating `S` | Floating `V` |
| `ceil` | `(value)` | Floating `S` | Floating `V` |
| `sin` | `(value)` | Floating `S` | Floating `V` |
| `cos` | `(value)` | Floating `S` | Floating `V` |
| `acos` | `(value)` | Floating `S` | Floating `V` |
| `tan` | `(value)` | Floating `S` | Floating `V` |
| `asin` | `(value)` | Floating `S` | Floating `V` |
| `atan` | `(value)` | Floating `S` | Floating `V` |
| `sinh` | `(value)` | Floating `S` | Floating `V` |
| `cosh` | `(value)` | Floating `S` | Floating `V` |
| `tanh` | `(value)` | Floating `S` | Floating `V` |
| `asinh` | `(value)` | Floating `S` | Floating `V` |
| `acosh` | `(value)` | Floating `S` | Floating `V` |
| `atanh` | `(value)` | Floating `S` | Floating `V` |
| `radians` | `(value)` | Floating `S` | Floating `V` |
| `degrees` | `(value)` | Floating `S` | Floating `V` |
| `atan2` | `(y, x)` | Common floating `S` | Two matching floating `V` |
| `exp` | `(value)` | Floating `S` | Floating `V` |
| `log` | `(value)` | Floating `S` | Floating `V` |
| `pow` | `(base, exponent)` | Common floating `S` | Two matching floating `V` |

The complete GLSL.std.450 angle and trig family accepts 16- and 32-bit
floating components in shader mode. Shader calls with 64-bit components are
diagnosed, including scalar integer calls promoted to `double`; use an explicit
`float` conversion when needed. Host math supports both `float` and `double`.
`atan2(y, x)` preserves C/C++ argument ordering. Supported pure math calls
are constant-evaluated when their operands are constant expressions.

These component operations do not accept matrices, except the distinct matrix
`mul` overload. Scalar splatting into aggregate operands is not supplied.

### Geometric and matrix functions

| Function | Parameters and constraints | Return |
| --- | --- | --- |
| `dot` | `(V a, V b)`, floating, matching shapes/types | `E` |
| `length` | `(V value)`, floating | `E` |
| `distance` | `(V a, V b)`, floating, matching shapes/types | `E` |
| `normalize` | `(V value)`, floating | Same vector shape |
| `cross` | `(V a, V b)`, floating, exactly three components | Same three-component shape |
| `reflect` | `(V incident, V normal)`, floating, matching shapes/types | Same vector shape |
| `refract` | `(V incident, V normal, E eta)`, floating, matching vectors; scalar eta converted to `E` | Same vector shape |
| `faceforward` | `(V normal, V incident, V reference_normal)`, floating, matching shapes/types | Same vector shape |
| `mul` | `(M<R,K> a, M<K,C> b)`, same floating component type | `MatrixResult<E,R,C>` |
| `transpose` | `(M<R,C> value)` | `MatrixResult<E,C,R>` |
| `determinant` | `(M<N,N> value)` | `E` |
| `inverse` | `(M<N,N> value)` | `MatrixResult<E,N,N>` |

### Legacy scalar float aliases

These headerless aliases take and return `float` after parameter conversion:

| Signature | Names |
| --- | --- |
| `float f(float value)` | `sqrtf`, `sinf`, `cosf`, `expf`, `exp2f`, `logf`, `log2f`, `fabsf`, `floorf`, `ceilf`, `truncf` |
| `float f(float a, float b)` | `fminf`, `fmaxf` |
| `float f(float a, float b, float c)` | `fmaf` |

`-fshader-host-math` provides the component/geometric/matrix functions and scalar
aliases for reusable host algorithms. It does not provide shader scope variables,
resource types, specialization values or device operations.

## Resources and descriptor arrays

Resource handles are supplied through entry parameters marked with
`__binding(set, binding)`. Default-constructing a texture object does not
allocate an image or establish a binding. Obtain usable handles from the entry
or copies of those handles. There are no public resource data members.

`T` is ordinarily a 32-bit floating, signed or unsigned integer
scalar, a homogeneous C array of 1–4 such components, or a compatible homogeneous
record of 1–4 components. R64i/R64ui storage textures additionally support
signed/unsigned 64-bit scalar texels. Bool texels are not supported.
Sampled texture types require `T`. Storage texture types require both `T` and an
explicit `format` argument `F`, such as `r32f` or `unknown`.

| Sampled resource type | Storage counterpart | Spatial coordinates | Array layer | `dimensions()` components |
| --- | --- | --- | --- | --- |
| `texture_1d<T>` | `storage_texture_1d<T,F>` | One | No | Width |
| `texture_2d<T>` | `storage_texture_2d<T,F>` | Two | No | Width, height |
| `texture_3d<T>` | `storage_texture_3d<T,F>` | Three | No | Width, height, depth |
| `texture_cube<T>` | None | Three-component sampling direction | No | Width, height |
| `texture_1d_layers<T>` | `storage_texture_1d_layers<T,F>` | One | Yes | Width, layers |
| `texture_2d_layers<T>` | `storage_texture_2d_layers<T,F>` | Two | Yes | Width, height, layers |
| `texture_cube_layers<T>` | None | Three-component sampling direction | Yes | Width, height, layers |
| `texture_2d_ms<T>` | `storage_texture_2d_ms<T,F>` | Two integer coordinates | No | Width, height |
| `texture_2d_ms_layers<T>` | `storage_texture_2d_ms_layers<T,F>` | Two integer coordinates | Yes | Width, height, layers |
| `texture_buffer<T>` | `storage_texture_buffer<T,F>` | One integer texel coordinate | No | Texel count |

`depth_texture_2d<T>`, `depth_texture_cube<T>`, and their `_layers` forms
represent comparison images. `T` must be `float`; use a
`sampler_comparison` with their comparison methods.

The `_layers` suffix denotes layers inside one image. Descriptor bindings use
ordinary C arrays. For example, `texture_2d_layers<float> images[4]` is four
descriptors, each referring to a layered 2D image. The former `_array` resource
type spellings are removed.

| Other native type | Role and operations |
| --- | --- |
| `sampler` | Opaque pointer-like descriptor; passed to sampling methods; no public members |
| `sampler_comparison` | Opaque descriptor used with depth texture comparison methods |
| `acceleration_structure` | Opaque pointer-like descriptor; passed to `ray_query::initialize`; no public members |
| `ray_query` | Invocation-local class created with `make_ray_query()`; methods below; not a bindable descriptor-array element |

### Descriptor C arrays

```cpp
__compute
__workgroup_size(1, 1, 1)
void shade(__binding(0, 0) texture_2d<float> textures[4],
           __binding(0, 1) sampler samplers[],
           __binding(0, 2) storage_texture_2d<float, r32f> targets[4]) {
  float value = textures[0].sample_lod(samplers[0], 0.0f, 0.0f, 0.0f);
  targets[0].store(0, 0, value);
}
```

Fixed bounds are positive 32-bit constant expressions; `[]` selects a runtime
descriptor array. Normal C++ parameter adjustment applies: `T parameter[N]`
becomes `T *` in the function type. The compiler retains the declared entry
bound for binding reflection. Use `const T (&parameter)[N]` when a helper needs
array-reference extent deduction or whole-array `sizeof`. Unknown-bound array
references, aliases, borrowed pointers and reference-to-pointer parameters work.

Subscripts accept integral values and unscoped enums, including bool and 64-bit
indices. Both `array[index]` and `index[array]` evaluate the left operand before
the right operand, once each. Indexing returns a resource handle **by value**;
it does not return a mutable reference to a descriptor binding.

Borrowed range pointers support offsets, compound offsets, increment/decrement
and method access. Pointer comparisons and differences are explicitly diagnosed.
Locally owned arrays, array fields and multidimensional descriptor arrays are
unsupported. The element must be a native descriptor resource, not an arbitrary
class, `ray_query` or another descriptor array.

Accesses, including iterator-based method access, are conservatively marked
nonuniform. `nonuniform(index)` takes an integer scalar and returns that same
unqualified scalar type with a nonuniform marker; it does not retrieve a
resource. There are no public `descriptor_at` or `descriptor_subscript`
functions, and the former `descriptor_array<Resource,N>` template is removed.
Reflection retains the classification string `descriptor_array`, with count
`N` for fixed arrays and zero for runtime arrays.

## Texture members

All texture member operations are native methods and may be called through
`const` resource objects or references. A const handle can still call `store` on
a storage image: image memory mutability is distinct from handle-object constness.
Methods accept scalar coordinates and return values; output-reference and
array-coordinate signatures are not supplied.

Notation below is a signature schema, not literal C++ variadic syntax:
`coordinates...` expands to the spatial coordinates followed by an array layer,
if present. Cube sampling uses a three-component direction. Sampling coordinates
and layers convert to `float`; load/store coordinates and layers convert to
`int`. All arguments must be scalar arithmetic values.

| Method | Parameters | Result and restrictions |
| --- | --- | --- |
| `load` | `(int coordinates...)` | Texel result; sampled fetch accepts an optional final unsigned mip; multisampled read accepts an optional final unsigned sample; omitted selector is zero |
| `store` | `(int coordinates..., E component0, ..., E componentN_minus_1)` | `void`; storage image only; one scalar value for each texel component; multisampled store accepts an optional unsigned sample between coordinates and values |
| `sample` | `(sampler s, float coordinates...)` | Texel result; implicit LOD; sampled, non-multisampled image |
| `sample_lod` | `(sampler s, float coordinates..., float lod)` | Texel result; explicit LOD; sampled, non-multisampled image |
| `sample_grad` | `(sampler s, float coordinates..., float dx_spatial..., float dy_spatial...)` | Texel result; explicit spatial gradients; no layer gradient |
| `gather` | `(sampler s, float coordinates..., unsigned component)` | Four-component `Result<E,4>` |
| `dimensions` | `()` or `(unsigned mip_level)` | Unsigned dimension scalar/result; mip argument only for sampled, non-MS, non-buffer images; omitted mip defaults to zero |
| `levels` | `()` | Unsigned mip count; sampled mipmapped images |
| `samples` | `()` | Unsigned sample count; multisampled images |
| `lod` | `(sampler s, float coordinates...)` | Two-component float result; sampled image LOD query |
| `sample_bias` | `(sampler s, float coordinates..., float bias)` | Texel result; implicit LOD with bias |
| `sample_offset` | `(sampler s, float coordinates..., int offset_spatial...)` | Texel result; signed spatial offsets |
| `sample_lod_offset` | `(sampler s, float coordinates..., float lod, int offset_spatial...)` | Texel result; explicit LOD and spatial offsets |
| `sample_grad_offset` | `(sampler s, float coordinates..., float dx_spatial..., float dy_spatial..., int offset_spatial...)` | Texel result; gradients and spatial offsets |
| `sample_bias_offset` | `(sampler s, float coordinates..., float bias, int offset_spatial...)` | Texel result; bias and spatial offsets |
| `gather_offset` | `(sampler s, float coordinates..., unsigned component, int offset_spatial...)` | Four-component result; spatial offsets |
| `sample_compare` | `(sampler_comparison s, float coordinates..., float reference)` | Scalar depth comparison with implicit LOD |
| `sample_compare_lod` | `(sampler_comparison s, float coordinates..., float reference, float lod)` | Scalar depth comparison with explicit LOD |
| `sample_compare_offset` | `(sampler_comparison s, float coordinates..., float reference, int offset_spatial...)` | Scalar depth comparison with spatial offsets |
| `sample_compare_lod_offset` | `(sampler_comparison s, float coordinates..., float reference, float lod, int offset_spatial...)` | Scalar depth comparison with explicit LOD and spatial offsets |
| `gather_compare` | `(sampler_comparison s, float coordinates..., float reference)` | Four depth comparison values |
| `gather_compare_offset` | `(sampler_comparison s, float coordinates..., float reference, int offset_spatial...)` | Four depth comparison values with spatial offsets |

Compile-time offsets use the SPIR-V `ConstOffset` operand. Under Vulkan 1.3,
dynamic `Offset` operands are available only for gather methods; sampling
offsets must be constant.

Texel return representation:

| `T` | Return of load/sample methods |
| --- | --- |
| Scalar | That scalar type |
| Homogeneous record | That record type |
| `E[N]`, including `E[1]` | `Result<E,N>` with public `E data[N]` and indexing |

Dimensions with one component return `unsigned`; multiple components return an
indexable aggregate with `unsigned data[N]`. Gather always returns four texel
components, independent of the declared texel component count.

For a `texture_2d<float>` and a `storage_texture_2d<float[4], rgba32f>`:

```cpp
void texture_example(const texture_2d<float> &image, sampler filter,
                     const storage_texture_2d<float[4], rgba32f> &target) {
  float value = image.sample_lod(filter, 0.5f, 0.5f, 0.0f);
  value = image.sample_grad(filter, 0.5f, 0.5f, 0.1f, 0.0f, 0.0f, 0.1f);
  auto taps = image.gather(filter, 0.5f, 0.5f, 0u);
  auto size = image.dimensions(0u);
  target.store(0, 0, value, taps[0], taps[1], 1.0f);
  auto texel = target.load(0, 0);
}
```

SPIR-V image dimensional restrictions still apply. Multisampled and buffer
images use load/dimensions, plus store for storage variants; sampling is not an
implemented workflow for those types. Cube resources are used for sampling and
size queries, not texel fetch. Gather is used on supported 2D/cube dimensions,
not 1D/3D images. The compiler supplies the shared method spellings on texture
classes, but that does not guarantee every method/dimension combination is valid;
final SPIR-V validation is required.

Offset methods accept non-cube, non-buffer images. Each offset component must
be a signed 32-bit constant expression; arrays of offsets and dynamic offsets
are not supported. Comparison sampling is supported as described above.

`texture_2d<float>::sparse_load(x, y, mip, &texel)` returns a residency code
and writes the fetched texel; `sparse_resident(code)` tests it. Sparse images
require Vulkan `sparseBinding` and `sparseResidencyImage2D`. Only scalar float
2D sparse fetch is currently exposed.

Image atomic methods accept separate integer coordinates or a homogeneous
record of matching 32-bit integer components, including `spirv::i32x2` and
`spirv::u32x2` for 2D images. For example, `image.atomic_sub(coord, 1u)`
subtracts one and returns the previous texel value. Array layers are included
in the coordinate record; multisample selectors remain separate arguments.
The record form supports all image atomic methods and their optional explicit
memory order/scope arguments, just like the scalar-coordinate form.

`input_attachment<T, index>` is a read-only fragment input attachment. Bind it
with `__binding`; `load()` reads the current fragment using
`InputAttachmentIndex` set from the template's `index`. The descriptor binding
and attachment index are independent. `T` has the usual 32-bit texel shape.

`tile_image<T, location>` is a fragment color attachment in `TileImageEXT`
storage, with a `Location` decoration supplied by the template argument. It
does not take a descriptor binding. `load()` or `load(sample)` reads the current
fragment's color. Texels can be 32-bit float, signed integer, unsigned integer,
or homogeneous records of up to four components.

`depth_attachment_read()` and `stencil_attachment_read()` return the current
fragment's depth and stencil values. Both accept an optional integer sample
index, defaulting to zero. Depth and stencil reads cannot be combined with
early fragment tests. These operations lower to the `SPV_EXT_shader_tile_image`
instructions and reflect `shaderTileImageColorReadAccess`,
`shaderTileImageDepthReadAccess`, or `shaderTileImageStencilReadAccess` as used.

`interlock_mode` provides `pixel_ordered`, `pixel_unordered`, `sample_ordered`,
`sample_unordered`, `shading_rate_ordered`, and `shading_rate_unordered`. Set the
entry's mode with `__fragment_interlock(interlock_mode::sample_ordered)` and
delimit the region with `begin_fragment_interlock()` and
`end_fragment_interlock()`. Without an annotation, these calls select ordered
pixel interlock. Reflection reports the corresponding pixel, sample, or
shading-rate interlock feature.

## Image formats

`format` is a compiler-synthesized unscoped enum with an unsigned underlying type.
Both `r32f` and `format::r32f` are valid template arguments. It has no members or
methods beyond normal enum behavior.

```cpp
using FloatImage = storage_texture_2d<float, r32f>;
using IntegerImage = storage_texture_2d<int, r32i>;
using FormatlessImage = storage_texture_2d<float, unknown>;
```

Declared enum values, with their SPIR-V numeric values:

| Value | Enumerator |
| --- | --- |
| 0 | `unknown` |
| 1 | `rgba32f` |
| 2 | `rgba16f` |
| 3 | `r32f` |
| 4 | `rgba8` |
| 5 | `rgba8_snorm` |
| 6 | `rg32f` |
| 7 | `rg16f` |
| 8 | `r11f_g11f_b10f` |
| 9 | `r16f` |
| 10 | `rgba16` |
| 11 | `rgb10_a2` |
| 12 | `rg16` |
| 13 | `rg8` |
| 14 | `r16` |
| 15 | `r8` |
| 16 | `rgba16_snorm` |
| 17 | `rg16_snorm` |
| 18 | `rg8_snorm` |
| 19 | `r16_snorm` |
| 20 | `r8_snorm` |
| 21 | `rgba32i` |
| 22 | `rgba16i` |
| 23 | `rgba8i` |
| 24 | `r32i` |
| 25 | `rg32i` |
| 26 | `rg16i` |
| 27 | `rg8i` |
| 28 | `r16i` |
| 29 | `r8i` |
| 30 | `rgba32ui` |
| 31 | `rgba16ui` |
| 32 | `rgba8ui` |
| 33 | `r32ui` |
| 34 | `rgb10_a2ui` |
| 35 | `rg32ui` |
| 36 | `rg16ui` |
| 37 | `rg8ui` |
| 38 | `r16ui` |
| 39 | `r8ui` |
| 40 | `r64ui` |
| 41 | `r64i` |

`unknown` enables formatless access. A used load/store requests the corresponding
`shaderStorageImageReadWithoutFormat` / `shaderStorageImageWriteWithoutFormat`
feature; merely declaring an unused formatless handle does not require both.
Formats 1–20 require floating-point texel components, 21–29 signed integer
components, and 30–39 unsigned integer components. Out-of-range values and
component-category mismatches are rejected, including unused entry resources.
The declared `r64ui`/`r64i` enumerators are not supported storage texel formats in
this API. Extended and multisampled storage image operations can add optional
image features to reflection.

## Atomics, barriers and subgroups

### Atomics

`I` is a writable signed/unsigned 32-bit or 64-bit integer pointee. `P` is either
`__storage I *` or a pointer to `__shared I`. Plain local, read-only, floating
and differently sized integer pointers are not accepted. Value arguments convert
to unqualified `I`; return values are also unqualified `I`.

| Signature | Meaning |
| --- | --- |
| `I atomic_load(P pointer)` | Read the current value |
| `void atomic_store(P pointer, I value)` | Store a value |
| `I atomic_add(P pointer, I value)` | Add; return previous value |
| `I atomic_sub(P pointer, I value)` | Subtract; return previous value |
| `I atomic_exchange(P pointer, I value)` | Exchange; return previous value |
| `I atomic_min(P pointer, I value)` | Signed/unsigned minimum; return previous value |
| `I atomic_max(P pointer, I value)` | Signed/unsigned maximum; return previous value |
| `I atomic_and(P pointer, I value)` | Bitwise AND; return previous value |
| `I atomic_or(P pointer, I value)` | Bitwise OR; return previous value |
| `I atomic_xor(P pointer, I value)` | Bitwise XOR; return previous value |
| `I atomic_compare_exchange(P pointer, I compare, I value)` | Store `value` if the old value equals `compare`; return old value whether comparison succeeds or fails |

Names and parameter order follow the CUDA-style API, including casing. Every
operation also has an overload ending in `memory_order` and `memory_scope`;
`atomic_compare_exchange` takes both success and failure orders before the scope. These are
compiler-provided scoped enums. `memory_order` provides `relaxed`, `acquire`,
`release`, `acquire_release` and `seq_cst`; Vulkan rejects `seq_cst`, so shader
calls using it are diagnosed. Load permits relaxed/acquire, store permits
relaxed/release, and modifying operations permit all four remaining orders.
CAS failure permits relaxed or acquire, provided it is no stronger than success.
`memory_scope` provides `device`, `workgroup` and `subgroup`; shared pointers
permit workgroup/subgroup, and physical storage pointers permit all three.
The original short overloads remain available: load uses Acquire, store Release, and modifying
operations AcquireRelease; failed CAS uses Acquire. Scope is Workgroup for
shared storage and Device for physical storage. The corresponding WorkgroupMemory
or UniformMemory semantics are included. Floating overloads of `atomic_add`, `atomic_sub`, `atomic_min` and `atomic_max`
accept writable 32/64-bit shared or storage floating-point pointers. Subtraction
uses atomic floating add of the negated value. Reflection requests the specific
buffer/shared float width and add/min-max extension features. Other atomic
floating overloads are diagnosed. These semantics are the implementation's
fixed policy, not a claim of equivalence to every CUDA atomic memory rule.
64-bit operations request the appropriate `shaderSharedInt64Atomics` or
`shaderBufferInt64Atomics` feature separately.

### Barriers

| Signature | Current scope and effect |
| --- | --- |
| `void workgroup_barrier()` | Workgroup execution rendezvous and AcquireRelease WorkgroupMemory semantics |
| `void subgroup_barrier()` | Subgroup execution rendezvous and AcquireRelease WorkgroupMemory semantics |
| `void device_memory_barrier()` | Device-scope AcquireRelease memory barrier for UniformMemory, WorkgroupMemory and ImageMemory; no execution rendezvous |

A memory barrier is not a device-wide execution barrier. Control barriers need
appropriate convergent participation. Workgroup scratch is declared inside a
function, for example `static __shared unsigned scratch[4];`; a global shared
object is rejected.

### Subgroup functions

`S` below is a scalar integer or floating-point type supported by the operation
and target capabilities. Array/record vector operands are not subgroup scalar
operands. Integer-only bitwise operations do not provide a floating overload.
Lane IDs, masks and deltas convert to `unsigned`; predicates convert to bool.

| Signature | Effect |
| --- | --- |
| `bool subgroup_all(bool predicate)` | True if all active invocations satisfy predicate |
| `bool subgroup_any(bool predicate)` | True if any active invocation satisfies predicate |
| `bool subgroup_all_equal(S value)` | Whether participating values agree |
| `bool subgroup_elect()` | Elect one active invocation |
| `S subgroup_broadcast(S value, unsigned invocation)` | Broadcast the identified invocation's value |
| `S subgroup_broadcast_first(S value)` | Broadcast the first active invocation's value |
| `S subgroup_shuffle(S value, unsigned invocation)` | Read the indexed invocation's value |
| `S subgroup_shuffle_xor(S value, unsigned mask)` | Shuffle using XOR lane mask |
| `S subgroup_shuffle_up(S value, unsigned delta)` | Shuffle toward lower invocation indices |
| `S subgroup_shuffle_down(S value, unsigned delta)` | Shuffle toward higher invocation indices |
| `S subgroup_add(S value)` | Add reduction |
| `S subgroup_mul(S value)` | Multiply reduction |
| `S subgroup_min(S value)` | Minimum reduction |
| `S subgroup_max(S value)` | Maximum reduction |
| `S subgroup_and(S value)` | Integer bitwise AND reduction |
| `S subgroup_or(S value)` | Integer bitwise OR reduction |
| `S subgroup_xor(S value)` | Integer bitwise XOR reduction |
| `S subgroup_inclusive_add(S value)` | Inclusive add scan |
| `S subgroup_exclusive_add(S value)` | Exclusive add scan |
| `S subgroup_inclusive_mul(S value)` | Inclusive multiply scan |
| `S subgroup_exclusive_mul(S value)` | Exclusive multiply scan |
| `S subgroup_{inclusive,exclusive}_{min,max,and,or,xor}(S value)` | Inclusive or exclusive arithmetic or bitwise scan |
| `S subgroup_clustered_{add,mul,min,max,and,or,xor}(S value, unsigned cluster_size)` | Clustered reduction; cluster size is a constant power of two |
| `bool subgroup_logical_{and,or,xor}(bool predicate)` | Logical reduction |
| `bool subgroup_{inclusive,exclusive,clustered}_logical_{and,or,xor}(bool predicate[, unsigned cluster_size])` | Logical scans and clustered reduction |
| `Result<unsigned,4> subgroup_ballot(bool predicate)` | Four-word ballot mask; requires an arithmetic scalar condition |
| `unsigned subgroup_ballot_bit_count(mask)` | Count ballot bits; accepts an unsigned four-component array/result/compatible record |

Subgroup operations use Subgroup scope. Reduction and scan operations do not
supply arbitrary user-defined operators. Results depend on active participation;
invalid/inactive shuffle targets do not acquire a defined value from this API.
Actual subgroup operation and stage support must be available on the device.

## Fragment intrinsics

| Signature | Effect |
| --- | --- |
| `E ddx(E value)` | Horizontal derivative |
| `E ddy(E value)` | Vertical derivative |
| `E fwidth(E value)` | Sum of absolute horizontal and vertical derivatives |
| `E ddx_fine(E value)` | Fine horizontal derivative |
| `E ddy_fine(E value)` | Fine vertical derivative |
| `E ddx_coarse(E value)` | Coarse horizontal derivative |
| `E ddy_coarse(E value)` | Coarse vertical derivative |
| `E interpolate_at_centroid(const E *input)` | Interpolate a fragment input at the centroid |
| `E interpolate_at_sample(const E *input, unsigned sample)` | Interpolate a fragment input at a sample |
| `E interpolate_at_offset(const E *input, f32x2 offset)` | Interpolate a fragment input at a two-component offset |
| `void begin_fragment_interlock()` / `void end_fragment_interlock()` | Delimit an ordered pixel interlock region; requires `fragmentShaderPixelInterlock` |
| `void discard()` | Discard the fragment invocation |

Derivative input/result `E` is a floating scalar; these are not C-array vector
overloads. Fragment derivatives, discard and implicit-LOD sampling require a
valid fragment-stage SPIR-V context. Workgroup/subgroup operations and image
methods are also subject to SPIR-V stage and capability restrictions; not every
such restriction is currently a frontend diagnostic.

`integer_dot(a, b)` and `integer_dot_accumulate_sat(a, b, accumulator)`
operate on signed or unsigned four-component 8-bit integer vectors. Mixed
signedness is supported. They lower to SPIR-V integer-dot instructions and
require Vulkan `shaderIntegerDotProduct`.

`cooperative_matrix_multiply(a, b, c, m, k, n)` performs `C = A * B + C`
using subgroup-scoped cooperative matrices. `a`, `b`, and writable `c` must
point to row-major `__shared float` storage for `m`-by-`k`, `k`-by-`n`, and
`m`-by-`n` matrices. Dimensions are compile-time constants from 1 through
256. The operation loads and stores the matrices and requires Vulkan
`cooperativeMatrix` plus a device-supported cooperative matrix shape. Callers
must synchronize shared storage before or after the operation when needed.
This API does not yet expose independent cooperative matrix values or other
element types, scopes, layouts, and operations.

## Ray queries

Create the native invocation-local class with `ray_query make_ray_query()`.
There is no user-constructed acceleration structure or query-state member.
All following operations are methods, not `ray_query_*` free functions.

The complete initialization parameter order is:

```text
void initialize(acceleration_structure scene,
                unsigned flags, unsigned cull_mask,
                float origin_x, float origin_y, float origin_z, float t_min,
                float direction_x, float direction_y, float direction_z,
                float t_max)
```

Origin/direction/distance parameters are scalar arithmetic values converted to
float. Flags and mask convert to unsigned. The API currently uses numeric ray
flags and intersection selectors; there is no added public ray-flags enum or
candidate/committed enum class.

| Method signature | Receiver and result |
| --- | --- |
| `void initialize(...)` | Non-const; initializes traversal |
| `bool proceed()` | Non-const; advances traversal and reports whether it continues |
| `void terminate()` | Non-const; terminates traversal |
| `void confirm_intersection()` | Non-const; confirms a candidate triangle intersection |
| `void generate_intersection(float t)` | Non-const; supplies a procedural candidate intersection distance |
| `unsigned intersection_type(unsigned selector)` | Const-compatible; selected intersection classification |
| `float intersection_t(unsigned selector)` | Const-compatible; selected intersection distance along the ray |
| `unsigned primitive_idx(unsigned selector)` | Const-compatible; selected primitive index |
| `unsigned instance_custom_idx(unsigned selector)` | Const-compatible; selected instance custom index |
| `unsigned geometry_idx(unsigned selector)` | Const-compatible; selected geometry index |
| `bool front_facing(unsigned selector)` | Const-compatible; selected face orientation |

Every getter selector must be compile-time `0` for candidate or `1` for committed.
`intersection_t` is a **method name returning float**, not an intersection type.
Query state must be appropriate to each operation; this API does not statically
prove traversal state. Methods work through pointers (`query->proceed()`) and
references. Copying a query shares its state rather than cloning traversal.

```cpp
bool query_scene(acceleration_structure scene) {
  ray_query query = make_ray_query();
  query.initialize(scene, 0u, 255u, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                   100.0f);
  while (query.proceed()) {
    if (query.intersection_type(0u) == 0u)
      query.confirm_intersection();
  }
  return query.intersection_type(1u) != 0u;
}
```

Ray query support requests ray-query/acceleration-structure device features.
Ray queries in supported shader stages are separate from the unfinished ray
tracing **entry stages**. Ray queries do not imply that ray-generation or hit
entry points are implemented.

## Specialization constants

The conceptual signature is `S specialization_constant(id, S default_value)`.
`id` must be a constant nonnegative unsigned 32-bit value; `default_value` must
be a constant scalar expression. The returned unqualified scalar type follows
the default value. Supported/tested defaults include bool, signed/unsigned
integer widths 8/16/32/64 and floating widths 16/32/64.

```cpp
void specialization_example() {
  unsigned count = specialization_constant(0u, 64u);
  float gain = specialization_constant(1u, 1.0f);
  bool enabled = specialization_constant(2u, true);
}
```

These are pipeline constants, not C++ constexpr objects. They cannot supply
C++ template arguments, array extents or attribute constant arguments. In
particular, `__workgroup_size` has fixed C++ constant dimensions. Use
`__workgroup_size_id(0, 4, 1, 2, 2, 1)` for dimensions that pipelines may
override; reflection reports the default workgroup size and three IDs.

`specialization_composite2`, `specialization_composite3`, and
`specialization_composite4` form SPIR-V vector specialization composites from
matching numeric scalar constants or direct `specialization_constant` calls.
The component IDs, rather than a composite ID, are overridden by the pipeline.

Repeated IDs must have matching types and exact defaults, including default
bit patterns; conflicting types/defaults are diagnosed. Reflection records the
ID, type, width, default and exact default bits. Vulkan overrides use
`VkSpecializationMapEntry` / `VkSpecializationInfo`, attached to the shader stage.
Without an override, the default applies.

## Compilation, reflection and device requirements

```sh
build/bin/clang++ -std=c++20 -fspirv-shader \
  -fspirv-entry=scale shader-cxx/examples/scale.cpp -o scale.spv
```

The default target is `spirv1.4-unknown-vulkan1.3`. The binary is validated
against Vulkan 1.3 and checked for a SPIR-V 1.4 header before publication. The
compiler emits an adjacent `scale.spv.json` reflection file. If the translation
unit contains multiple entries, the default output contains all of them, even
when they belong to different stages. `-fspirv-entry` selects a single entry;
it accepts an unambiguous qualified source name or an exact exported/mangled
name. Overloaded entries require exact selection.

Entry names follow C++ linkage. A function `void shade()` exports `_Z5shadev`;
`extern "C" void shade()` exports `shade`. Namespace and parameter types affect
C++ mangling; C-array parameter adjustment also follows ordinary C++ rules.
Use reflection's `entry` string as Vulkan pipeline `pName`, rather than guessing
it from a source alias. `source_entry` records the qualified source name. For a
combined module, reflection schema version 2 has an `entries` array with the
single-entry reflection for each stage. Top-level `required_features`,
`spirv_capabilities` and `spirv_extensions` are the union across entries.
Combining entries requires SPIR-V Tools `spirv-link` beside Clang or on PATH.

Separate compilation emits LLVM bitcode with shader-mode `-c`; link it before
legalization:

```sh
build/bin/clang-spirv-link helpers.bc shader.bc --entry=main -o shader.spv
```

The shader profile always enables physical-storage-buffer/device-address and
descriptor-indexing support in the compiler. Buffer access uses physical device
addresses, while images/samplers/acceleration resources remain descriptor-based.
Compiler support for a feature family does not assert that every Vulkan device
supports every optional feature in that family.

Reflection retains schema version 1 and describes entry/stage, dimensions,
bindings/locations, builtins, layouts, pointer fields, specialization constants,
capabilities, extensions and `required_features`. `required_limits` records
push-constant bytes, workgroup shared-memory bytes, workgroup dimensions and
workgroup invocation product.

Examples of use-dependent requirements include narrow storage features,
64-bit integer atomics by storage class, formatless storage-image reads/writes,
extended image formats and nonuniform descriptor indexing. Physical pointer
pointee widths do not turn the inline interface into narrow scalar storage.
A float-only device-address record does not need shaderInt8 merely to represent
its pointers or suitable padding.

`descriptorIndexing` in reflection is a compiler feature-family profile marker;
individual optional Vulkan descriptor features are listed separately. The test
runtime harness queries device features/limits and checks reflected requirements
before creating a pipeline. Reflection and the harness are not a claim of
complete inference/checking for every unfinished advanced feature.

## Additional intrinsic families

All of these are headerless overloads. Scalar and vector operations accept the
C arrays and homogeneous records described above; the compiler marshals values
only at the intrinsic boundary.

| Family | Signatures and results |
| --- | --- |
| Component math | `round(x)`, `round_even(x)`, `trunc(x)`, `sign(x)`, `fract(x)`, `exp2(x)`, `log2(x)`, `inverse_sqrt(x)`, `step(edge,x)`, `smoothstep(edge0,edge1,x)`, `fma(a,b,c)`, `mod(x,y)`; shape-preserving scalar/vector results. `mod` uses floor division. `round_even` specifies ties to even. |
| Decomposition | `modf(x)` returns `.fraction` and `.whole`; `frexp(x)` returns `.significand` and signed 32-bit `.exponent`; fields are scalar or indexable vector results. `ldexp(x,exponent)` requires a matching signed 32-bit exponent shape. No output-reference arguments. |
| Matrix operations | `mul(matrix,vector)`, `mul(vector,matrix)`, `mul(matrix,scalar)`, `mul(scalar,matrix)` supplement matrix multiplication. `outer_product(vector,vector)` returns a matrix. Matrices keep row-major C-array indexing. |
| Integer bits | `bit_count(x)`, `bit_reverse(x)`, `bit_extract(x,offset,count)`, `bit_insert(base,value,offset,count)`, `find_lsb(x)`, `find_msb(x)` accept 32/64-bit integer scalar/vector shapes. Offsets/counts are scalar integers; constant out-of-range selections are diagnosed. Find results use signed 32-bit components. Wide operations are lowered through
32-bit pieces and ordinary 64-bit arithmetic, avoiding newer Vulkan bitwise
capabilities outside the Vulkan 1.3 profile. |
| Extended integers | `add_carry(a,b)` returns `.value` and `.carry`; `sub_borrow(a,b)` returns `.value` and `.borrow`; `mul_extended(a,b)` returns `.low` and `.high`. Inputs are matching 32/64-bit scalar/vector integers; add/sub require unsigned types. |
| Floating classification | `is_nan(x)`, `is_inf(x)`, `is_finite(x)`, `is_normal(x)`, `sign_bit(x)` accept 16/32/64-bit scalar/vector floats. Scalar results are `bool`; vector results have unsigned 0/1 components. |
| Packing | `pack_snorm4x8`, `pack_unorm4x8`, `pack_snorm2x16`, `pack_unorm2x16`, `pack_half2x16` take float vectors and return unsigned 32-bit integers; corresponding `unpack_*` functions return indexable float vectors. `pack_double2x32(unsigned[2])` returns double; `unpack_double2x32(double)` returns unsigned components. |
| Subgroup ballots | `subgroup_inverse_ballot(mask)`, `subgroup_ballot_bit_extract(mask,idx)`, `subgroup_ballot_find_lsb(mask)`, `subgroup_ballot_find_msb(mask)`, `subgroup_ballot_inclusive_bit_count(mask)`, `subgroup_ballot_exclusive_bit_count(mask)` use unsigned four-component masks. |
| Subgroup quads | `subgroup_quad_broadcast(value,idx)`, `subgroup_quad_swap_horizontal(value)`, `subgroup_quad_swap_vertical(value)`, `subgroup_quad_swap_diagonal(value)` return the operand shape. Arithmetic, broadcasts and shuffles also accept vector shapes. |
| Explicit barriers | `memory_barrier(scope,semantics)`, `control_barrier(execution_scope,memory_scope,semantics)` take constant unsigned SPIR-V scope/semantics values. Device=1, workgroup=2, subgroup=3. Vulkan validation checks legal combinations. |
| Clock | `shader_clock(scope)` returns unsigned 64-bit ticks; scope must be device=1 or subgroup=3. Reflection requests the appropriate optional device clock feature. |
| Demotion | `demote()` demotes a fragment invocation to a helper invocation and continues execution. Reflection requests `shaderDemoteToHelperInvocation`. |

`printf("value=%f", value)` emits `NonSemantic.DebugPrintf` for shader
diagnostics. The format must be an ordinary or UTF-8 string literal without
control characters. Arguments are 32- or 64-bit numeric scalars or two- to
four-component vectors; `printf("message")` needs no value arguments.
Output requires a tool or validation layer that handles non-semantic debug
printf and a device with `VK_KHR_shader_non_semantic_info` enabled. It has no
shader-visible return value, and non-semantic instructions may be stripped.

Ray-query const getters additionally include `ray_t_min()`, `ray_flags()`,
`instance_idx(selector)`, `instance_sbt_offset(selector)`,
`barycentrics(selector)`, `candidate_aabb_opaque()`,
`object_ray_origin(selector)`, `object_ray_direction(selector)`,
`world_ray_origin()`, `world_ray_direction()`, `object_to_world(selector)` and
`world_to_object(selector)`. Vector results support C-array indexing. Matrix
getters return a row-major 3-by-4 float result indexed `[row][column]`.
Selectors are constant candidate=0 or committed=1; callers must obey ray-query
traversal-state preconditions.

## Direct SPIR-V inline assembly

GNU-style `asm` bodies contain SPIR-V assembly:

```cpp
float twice;
asm("%0 = OpFAdd %t0 %1 %1" : "=r"(twice) : "r"(value));
asm volatile("OpStore %0 %1 Aligned %a0"
             : "=m"(*destination) : "r"(twice) : "memory");
```

`r` is a scalar or pointer value; `v` is a two-to-four-component C array or
homogeneous record; `m` is an lvalue address; input `i` is an integer constant
expression substituted as a literal. Outputs use `=` or `+`; tied inputs and
named operands use GNU syntax. `%t0` or `%t[name]` substitutes the SPIR-V type ID;
`%a0` or `%a[name]` substitutes an `m` operand's guaranteed alignment in bytes.
Read/write lvalues are evaluated once. Multiple outputs return to their original
C++ destinations without output-reference APIs.

Write local IDs as `%%name` in extended asm. They are renamed per expansion;
`OpType*`, constants, imports, capabilities, extensions, debug names and
decorations are placed in their module sections. Non-aggregate duplicate types
are reused. Memory operands and `"memory"` clobbers carry memory effects;
`volatile` prevents dead-call elimination. Different templates retain distinct
optimizer identities. `asm goto` and redefining module entry points, functions
or memory models are rejected. Templates must form valid structured SPIR-V;
the assembler and Vulkan validator check the final expanded module before
publication. The tools `spirv-as` and `spirv-dis` must be installed alongside the
compiler or on `PATH`. No `OpAsmINTEL` is emitted.

## Current limits and verification

| Area | Current limit |
| --- | --- |
| Advanced entry stages | Parse with explicit unsupported-stage diagnostics |
| Direct SPIR-V inline assembly | Supported as described above; module/function redefinition and asm goto are rejected |
| Native constexpr intrinsics | Constant evaluation covers selected pure floating-point and integer component math; other intrinsic families remain runtime-only |
| Additional image APIs | Broader sparse operations, image atomics, input attachment variants and 64-bit image combinations remain pending; basic forms and constant-offset sampling/gather methods are supported |
| Descriptor array ownership | Borrowed entry ranges only; local owned arrays, array fields, nested ranges and pointer comparisons/differences are rejected |
| Pointer generality | Arbitrary reinterpretation and all byte-address patterns are not covered; dynamic local record-field offsets/escapes remain unsupported |
| Reflection | Source member names and complete advanced-feature inference remain pending |
| Runtime coverage | Compute math/pointers/atomics and related cases execute; texture/descriptor rendering and hardware ray traversal currently have compile/validation coverage only |
| Slang corpus and packaging | One shared-atomic test has been ported; broad Slang test coverage and full installation/CI work remain pending |

The last complete mandatory suite run passed 658 checks. A previous compatible-device
run passed 129 optional Vulkan execution checks; together these would total 787 if
reproduced in one run. The recorded LLVM/Clang regression run
has 1,618 passes, 13 unsupported skips and two expected failures. Core
compilation/validation includes `-O0` and `-O2`; optional runtime skipping does
not skip compile checks. All 120 selected shader-port binaries passed validation
at `-O0` through `-O3`, including Godot, ImGui, Vulkan Samples, Filament,
FidelityFX and Slang cases.
The separate [complex Shadertoy corpus](../shader-ports/shadertoy/complex/README.md)
adds 512 distinct shaders that each compile and validate at `-O0` and `-O2`;
486 also pass at `-O1` and `-O3`.
See [feature coverage](tests/COVERAGE.md) for evidence and [remaining gaps](GAPS.md)
for the full outstanding scope.

Implementation catalogs and signature checks:

- [Scope value catalog](../clang/include/clang/Basic/SPIRVShaderBuiltins.def).
- [Intrinsic lowering catalog](../clang/include/clang/Basic/SPIRVShaderIntrinsics.def): texture/query names there are **internal lowering names**, not extra public free functions; their receiver contributes to internal arity.
- [Native declaration and argument checking](../clang/lib/Sema/SemaSPIRVShader.cpp).
- [Shape and resource classification](../clang/include/clang/AST/SPIRVShader.h).
- [Native intrinsic lowering](../clang/lib/CodeGen/CGShaderBuiltin.cpp).
- [Entry/interface lowering](../clang/lib/CodeGen/CGSPIRVShader.cpp).

Removed spellings such as `vector_min`, `vector_*`, `matrix_mul`, `matmul`,
`descriptor_array`, `descriptor_at`, former camelCase atomics, `texture_*` free
functions and `ray_query_*` free functions are not compatibility APIs. Use the
current overload names, native methods and C-array descriptor parameters above.
