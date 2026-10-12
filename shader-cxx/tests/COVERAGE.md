# Shader C++ feature coverage

Run `python3 shader-cxx/tools/test.py` for the mandatory suite (658 checks in
the last complete run). Add `--runtime` for optional Vulkan execution checks
(129 passed in a previous compatible-device run; 787 combined if both counts
are reproduced). Mandatory checks include both default optimization and `-O2`
for every positive fixture.
Runtime skips are explicit; they never skip compilation or SPIR-V validation.
The runner uses explicit checks that remain active under `python3 -O`.
Coverage guards fail if the compiler adds a scope builtin, scalar math alias or native intrinsic
without an independent test, or if a fixture is omitted from the compilation matrix.

| Implemented surface | Coverage and evidence |
| --- | --- |
| Headerless C++20 mode and 64-bit C++ sizes | `SemaSPIRVShader/headerless.cpp`, `catalog.cpp`; C++17/profile rejection; real examples contain no includes |
| Keyword isolation from ordinary C++ | `SemaSPIRVShader/host-gating.cpp` uses every keyword family as ordinary host identifiers |
| Templates, concepts, constexpr, consteval, classes, lambdas, loops | `fixtures/cxx20.cpp`: compile/validate and execute at both optimization levels; `scope.cpp`: array borrowing and lambda captures |
| C++ and C entry linkage | `CodeGenSPIRVShader/linkage.cpp`; exported SPIR-V names, namespaces, overload selection, ambiguous-selector rejection, reflection names and Vulkan pipeline lookup |
| Built-in vertex Position and stage vector marshalling | `position.cpp` Sema/CodeGen tests; dynamic C-array indexing and nine invalid Position declarations at both levels; ImGui ports check exact input/varying vector shapes, Position decoration, adjacent locations and fragment color sampling |
| Compute/vertex/fragment entry wrappers | `CodeGenSPIRVShader/entry.cpp`; `examples/stages.cpp`; execution model, origin and selected-entry reflection assertions |
| Geometry/tessellation, EXT task/mesh and KHR ray stages | `CodeGenSPIRVShader/advanced-stages.cpp`, `fixtures/advanced-stages.cpp`: 12 independently selected entries at `-O0`/`-O2`, native operations, per-vertex arrays/records, boolean mesh culling, payloads, patch builtins and feature reflection; configuration/operation misuse diagnostics |
| Workgroup dimensions and synthesized `workgroup_size` | `scope.cpp`: dimensions 2/3/4 and no WorkgroupSize input; missing, zero, negative, conflicting and inappropriate-stage dimensions are rejected |
| Exactly two positional binding arguments | `examples/uniform.cpp`: DescriptorSet 2/Binding 7 in binary and reflection; arity, negative, overflow, runtime value and conflict failures |
| Stage input/output locations, scalar value input and pointer output | `examples/stages.cpp`; inspect binary/reflection locations; global/local interfaces, writes to inputs and value outputs are rejected |
| Native interpolation qualifiers | `interpolation.cpp`: matching vertex/fragment interfaces, default and `-O2` validation; Flat/NoPerspective/Centroid/Sample decorations and reflection; C arrays, default perspective, attribute ordering, conflicts, placement, integer input rules and sampling feature requirements |
| Storage device addresses | PhysicalStorageBuffer capability, extension, stride and address-conversion assertions; device address supplied by Vulkan host harness |
| Pointer increment/decrement, indexing, `*`, `->`, comparison, difference | `pointers.cpp`: emitted operations, validation and runtime record fields/values at both optimization levels |
| Pointer alignment | `examples/alignment.cpp`: natural float pointee alignment; runtime at both levels; removed shader-specific alignment annotations are rejected |
| Uniform/push read-only pointees | `SemaSPIRVShader/diagnostics.cpp`; input-write and missing/misplaced binding rejection |
| Shared workgroup memory | `shared.cpp`: function-local `static __shared` scratch, emitted Workgroup pointer, validation and runtime at both levels; global shared declarations rejected |
| C ABI record layout, nested records/arrays, manual padding/alignment | `layout.cpp`: explicit padding member; reflected 48-byte size, 16-byte alignment, all field offsets and array strides; native compiler static assertions; validation at both levels; no std140/std430 repacking |
| Invocation-local initialized globals | `invocation-state.cpp`: helper mutations are visible within the invocation; two dispatches verify initialization is renewed; both levels |
| All current scope-value public types | `catalog.cpp` independently pins all 40 names, scalar types and array extents, including advanced-stage declarations |
| Compute/subgroup scope lowering | `scope.cpp`: every current compute/subgroup BuiltIn number checked in reflection |
| Vertex scope lowering | `vertex-scope.cpp`: vertex/instance/base/draw/view values; DrawParameters/MultiView capabilities and reflected feature requirements |
| Fragment scope lowering | `fragment-scope.cpp`: primitive/sample/coordinate/boolean values; SampleRateShading/Geometry requirements; boolean types and integer builtin Flat decoration |
| Fragment built-in interfaces | `fragment-interfaces.cpp` and `CodeGenSPIRVShader/fragment-builtins.cpp`, `fragment-interpolation.cpp`: depth and sample-mask outputs, clip/cull-distance vertex outputs and fragment inputs, point-size output, and centroid/sample/offset interpolation; binary validation and invalid-placement diagnostics |
| Fragment execution modes and stencil export | `CodeGenSPIRVShader/fragment-modes.cpp` and `SemaSPIRVShader/fragment-modes.cpp`: early tests, conservative depth, post-depth coverage, stencil-reference output, capability/extension emission and invalid placement/type diagnostics |
| Tile-image reads and interlock scopes | `tile-image.cpp` fixtures and Sema/CodeGen tests cover float, signed-record and unsigned color reads, depth/stencil reads, sample indices, noncoherent modes, attachment locations and feature reflection at O0/O2; `fragment-interlock-modes.cpp` validates all six ordered/unordered pixel/sample/shading-rate modes |
| Actual C arrays, borrowing, decay, component shorthand | `headerless.cpp`, `catalog.cpp`, `scope.cpp`; all x/y/z/w component widths; runtime-constexpr and invalid shorthand checks |
| format unscoped enum and template argument use | `catalog.cpp` checks all 42 enumerator values and unsigned underlying type; `headerless.cpp` checks NTTPs; qualified/unqualified lookup, enum identity, implicit integer conversion and resource template arguments are checked |
| All 14 headerless scalar math intrinsics | `math-all.cpp`: independently listed math calls, emitted GLSL instruction checks and CPU-reference runtime values at both levels |
| Ray query member operations | `ray-query-methods.cpp` checks native class identity, return types, arity, selectors, const receivers, private handles and former free-function rejection; `ray-query.cpp` covers all 21 operations, pointer/template receivers and factory returns at both optimization levels |
| Return-value APIs and scalar method arguments | `value-results.cpp` checks result types, C-array storage, const indexing, matrix dimensions, ballots, invalid descriptor elements and rejection of old signatures; its shader fixture exercises array texel returns, scalar stores and chaining; texture and ray coordinates are separate scalar arguments |
| Namespace isolation for every function name | `namespace-isolation.cpp`: all 216 intrinsic/alias/member spellings have independent user overloads; qualified and imported lookup, dependent templates, constexpr calls and function pointers in shader/host math modes; the runner checks normal C++ calls for every name and guards catalog coverage |
| Complete angle/trig family | `trig.cpp`, `host-trig.cpp`, and the audit catalogs: all 15 operations; scalar/array/record results; host float/double and Vulkan numerical execution; half compilation; shader double rejection; namespace isolation and invalid operands |
| Native `acos` | `acos.cpp`: scalar/array/record types, invalid operands and constexpr rejection; host reference execution and Vulkan scalar/array reference values at both levels; Godot roughness port emits `Acos` |
| Qualified trivial record assignment | `record-assignment.cpp`, positive Godot fragment outputs, exact Vulkan chained assignment results and read-only/nontrivial rejection at both levels |
| Optimized barrier reductions | LLVM SimplifyCFG/JumpThreading/AlwaysInliner verifier regressions; literal and template-expanded 64-lane Vulkan reductions yield 2080; all Godot luminance variants validate at both levels |
| All 108 native intrinsics | Independent call coverage guard; positive fixtures validated at both optimization levels; `native-intrinsics.cpp` checks invalid arguments without crashes |
| Array/record vector math and row-major matrix math | `math-body.h`, `vector-matrix.cpp`, `host-math.cpp`: 78 numerical results on host and Vulkan at both levels; scalar float/double/integer and mixed-type overloads, integer arrays, padded records, returned-value chaining, C-array borrowing and aggregate copies, 2/3/4 square matrices and rectangular multiplication |
| Installed `<spirv/types.hpp>` / `<spirv/vec.hpp>` / `<spirv/mat.hpp>` types | All headers compile independently; scalar aliases in `namespace spirv` at 8/16/32/64-bit widths plus `b8`, all 33 numeric and three `b8xN` Boolean vector aliases, and all nine `f32` matrix aliases compile with their expected C sizes; all 15 GLSL-style vector/matrix aliases are rejected; all 336 two-/three-/four-component named swizzles compile; component access and representative swizzles are constexpr; C-array size/alignment and rectangular matrix layout are asserted; host arithmetic and matrix results execute at `-O0`/`-O2`; shader code validates at both levels with integer and floating-point vector addition across all supported widths, `OpDot`, `OpVectorShuffle`, and GLSL math instruction checks; Vulkan execution covers the `f32` vector/matrix fixture at both optimization levels |
| Qualified vector/matrix member access | `qualified-vec-mat.cpp` Sema checks Uniform, PushConstant and Storage component and matrix-row reference address spaces plus named/generic swizzles; CodeGen links and validates direct resource-backed reads, Storage component/swizzle writes and matrix indexing at `-O0`/`-O2`; Filament PBR entries validate using direct Uniform/PushConstant record copies without scalar materialization |
| Native sampled/storage texture classes and methods | `resources.cpp`, `image-types.cpp`: 1D/2D/3D, arrays, cube/cube array, multisampling, texel buffers, scalar/record texels, signed/unsigned formats; inspect sampling/fetch/read/write/query operations; class identity, private representation, copy/return, const references, templates, arrow access, temporary descriptor results and method arity; old free functions are rejected |
| C-array descriptors and nonuniform access | `descriptor-array.cpp` and `ray-query.cpp`: `Resource name[N]` / `Resource name[]` bindings for textures, storage images, samplers and acceleration structures; array aliases, fixed/unknown-bound references, template extent deduction, decayed pointers and reference-to-pointer arguments; subscript and iterator/arrow nonuniform feature inference, exact reflected counts and O0/O2 validation; `descriptor-subscripts.cpp` checks real C-array identity, index types and evaluation order; the former `descriptor_array` template and invalid bounds, elements, nesting, owned storage and pointer comparisons/differences are rejected |
| Integer atomics | `atomic-names.cpp` Sema/CodeGen tests and `operations.cpp`/`atomic-types.cpp` fixtures: all eleven snake_case names matching image atomic methods, camelCase-name rejection, load/store return types, successful and failed `atomic_compare_exchange(pointer, compare, value)` with previous-value checks, signed/unsigned 32/64-bit physical storage and shared pointers; instruction checks and numerical Vulkan results for shared 32-bit operations |
| Barriers and subgroups | `operations.cpp`: voting/ballot, scans/reductions, shuffle/broadcast and barriers; `synchronization.cpp`: four invocations increment shared state with barriers, executed at both levels |
| Fragment derivatives and discard | `fragment-operations.cpp`: ordinary/fine/coarse derivatives, fwidth, discard, implicit/gradient sampling and gather; inspect emitted operations and validate at both levels |
| Ray queries | `ray-query.cpp`: all declared query operations, candidate/committed selectors, reflection feature requirements; validate at both levels; invalid query/selector diagnostics |
| Specialization constants | `specialization.cpp`, `specialization-types.cpp`: bool, signed/unsigned integer widths 8/16/32/64 and float widths 16/32/64; repeated IDs through aliases, exact default reflection, conflicting defaults/types rejected; Vulkan defaults and int/float/bool overrides |
| Workgroup and vector specialization | `workgroup-size-id.cpp` and `specialization-composite.cpp`: `LocalSizeId`, three reflected defaults/IDs, vector `OpSpecConstantComposite` at O0/O2 |
| Optional fragment and integer dot | `integer-dot.cpp`, `optional-fragment.cpp`, and `fragment-interlock.cpp`: native signed/unsigned/mixed integer dot, barycentrics, fragment shading-rate input, ordered pixel interlock, capabilities and feature reflection; validated at O0/O2 |
| Cooperative matrix multiply-add | `cooperative-matrix.cpp`: native subgroup `f32` load, multiply-add, and store over shared arrays, with feature reflection and invalid-operand diagnostics; validated at O0/O2 |
| Host math mode | Host algorithm compiled and executed at both levels; `host-math-gating.cpp` rejects device operations and scope values; ordinary host identifiers remain available without the flag |
| Link translation units before legalization | Two temporary source files with helper/template code; bitcode linking, validation, no remaining function calls and Vulkan execution |
| Multiple entries in one module | Default output contains compute, vertex and fragment entries, including overloaded names; same-stage entries retain independent workgroup sizes and shared descriptor bindings; optimized mixed-stage fixtures validate; schema version 2 lists each entry and merged requirements |
| Entry selection | `-fspirv-entry` selects one entry; ambiguous overload, missing and unknown entry failures |
| Entry parameters own shader interfaces/resources | `SemaSPIRVShader/global-interfaces.cpp` and 20 compile-failure cases cover globals, namespaces, static members, aliases, arrays, records, auto, block extern/static resources and misplaced binding/location attributes |
| Shader call restrictions | Direct/mutual recursion, unresolved functions/variables, indirect calls and dynamic global initialization rejected without crashes |
| SPIR-V 1.4/Vulkan 1.3 profile | Binary magic/version checks, real spirv-val invocation, capability/extension/feature reflection; incompatible module triple rejected |
| Reflection | Entry, stage, dimensions, bindings, locations, BuiltIns, layout, device addresses, capabilities, extensions and used feature requirements checked |
| Validation before artifact publication | Forced validator failure leaves no binary, JSON or temporary file; existing successful binary/JSON remain intact |
| LLVM inspection output | Native linker's `--emit-llvm` output checked for selected entry |

## Audit regression coverage

`audit-regressions.json` contains 122 permanent compile/diagnostic cases, each
checked at `-O0` and `-O2`. `audit-runtime.json` contains 35 independent numeric
cases, compiled, validated and executed at both levels. Every successful module
is validated for SPIR-V 1.4 under Vulkan 1.3. The original audit evidence remains
unchanged in `audits/2026-10-07`; `verification.txt` records the fixes and checks.

| Finding | Permanent regression evidence |
| --- | --- |
| F01 bool arithmetic | Scalar/array addition and min on Vulkan; promoted bool arithmetic on the host at both levels |
| F02 entry ABI | Float/integer stage records; recursive Flat requirement and invalid non-Flat integer record rejection |
| F03/F04 nested device addresses | Push pointer arrays, pointer-to-pointer and physical pointer table cases; numeric push/address-table execution |
| F05 physical selection | Selected device addresses, comparisons, null selection, numeric results |
| F06 storage formats | Unknown load/store capability and feature assertions; unused unknown resource; invalid values and category mismatches rejected; signed/unsigned/extended formats |
| F07/F08 mixed/nested records | Multiple float/integer addresses and nested records; numeric results for both address fields |
| F09 physical null | Null comparisons and selected null; successful validation without physical OpConstantNull; numeric execution |
| F10 aggregate results | Dynamic vector/matrix indexing, `.data`/nested result use; numeric results |
| F11 nested push arrays | Dynamic nested arrays and numeric nested push record values |
| F12 first-member paths | Nested and overaligned first-member arrays; numeric nested-first-member results |
| F13 logical selections | Conditional local pointers, volatile memory, finite-base and offset loops; numeric selected stores/loads |
| F14 narrow storage | 8/16-bit push and uniform types, including half; capability and optional feature assertions |
| F15 unnecessary features | Float-only addresses, over-aligned padded float records and physical-pointer leaves exclude unwanted narrow features |

Related cases cover resource reference copying/rebinding, known function pointers,
natural local/parameter/type-alias pointer alignment and removed annotation rejection,
volatile intrinsic component reads, extended storage formats and separate shared
versus buffer 64-bit atomic feature requirements. Namespace and global overload
catalog guards require user C++ calls for every applicable intrinsic spelling.

`reflection-requirements.cpp` uses mocked devices to test acceptance and rejection
at push/shared/workgroup limits, all three dimension limits, formatless image
features, narrow storage features and shared/buffer atomic feature separation.
These checks run without Vulkan execution or a compatible GPU. The runtime
harness parses reflection and checks actual features and limits before creating
a pipeline. `descriptorIndexing` is the compiler family profile marker; the
individual descriptor features remain explicit device requirements.

## Expanded intrinsic and assembly coverage

| Area | Evidence |
| --- | --- |
| Direct SPIR-V assembly | `inline-asm.cpp` plus audit cases: scalar, array and record operands, multiple/named/read-write/tied outputs, aligned physical memory, immediates, unique local IDs, distinct template optimizer identities, single evaluation, quoted strings and module-section hoisting with forward decoration/type aliases. Positive cases validate and execute at O0/O2; invalid constraints, shapes, immediates, modifiers, instructions, types, module redefinition and asm goto have diagnostic cases. |
| Debug printf | `debug-printf.cpp` validates the non-semantic extension/import, `OpString` and `OpExtInst` with scalar/vector and zero-argument calls; Sema diagnostics reject nonliteral formats, control characters and unsupported argument shapes. |
| Extended math and decomposition | `extended-math.cpp`, `decomposition.cpp`, `host-extended.cpp`, `host-decomposition.cpp` and numerical audit cases: rounding ties, negative sign/fract/mod, unsigned sign, scalar/array/record shapes, decomposition fields and signed exponents. Shader double exp2/log2 are diagnosed. |
| Matrix overloads | `matrix-overloads.cpp` and exact numerical cases cover rectangular matrix-vector/vector-matrix multiplication, both scalar orders, scalar conversion, outer products and one-by-one matrices. Invalid dimensions have rejection cases. |
| Integer and packing families | `bit-packing.cpp`, `wide_bit_operations` and exact numerical expectations exercise every added bit/packing function at both levels, including negative signed lanes, double bit patterns and 64-bit emulation within Vulkan 1.3. Bad widths, shapes and constant ranges have diagnostics. |
| Subgroup vectors and quads | `subgroup-extended.cpp` and a four-lane numerical case check vector arithmetic, broadcasts, shuffles, all-equal, ballot queries/scans and every quad exchange against distinct lane values. |
| Image fetch selectors and queries | `image-queries.cpp` checks explicit mip/sample operands, storage multisample reads/writes, LOD/level/sample queries and biased sampling. Required operations and feature reflection are checked at both levels; image execution remains a coverage gap. |
| Ray-query getters | `ray-query.cpp` checks scalar/vector and 3-by-4 matrix getters, const receivers and native SPIR-V operations. The fixture observes traversal/triangle/AABB preconditions; hardware execution remains a coverage gap. |
| Barriers, clocks and demotion | `control-extended.cpp` checks explicit scopes/semantics, device/subgroup clocks and fragment demotion, their operations and optional feature reflection. Dynamic/invalid scopes are rejected. Existing discard tests continue to require termination. |
| Floating atomics | `atomic-float.cpp` checks buffer float/double and shared float add/sub/min/max, generated floating atomic operations and specific optional feature requirements. Unsupported floating overloads are rejected. Extension-enabled execution is still pending. |
| Extended core intrinsics | `extended-integer.cpp`, `classification.cpp` and `subgroup-extended.cpp` validate native carry/borrow/multiply, f16/f32/f64 floating classification and the complete new scan/cluster/logical subgroup set at O0/O2. Invalid widths, element types and cluster sizes are diagnosed. |
| Image constant offsets | `image-offsets.cpp` validates sample, LOD, gradient, bias and gather offset methods at O0/O2 and checks image operand masks. Dynamic sampling offsets and cube/buffer offsets are diagnosed. |
| Depth comparison and gather offsets | `image-depth-compare.cpp`, `CodeGenSPIRVShader/image-depth-compare.cpp` and `image-dynamic-offsets.cpp` validate 2D, cube and layered depth sampling, comparison gather, constant sample offsets and dynamic gather offsets under Vulkan 1.3. Dynamic sample offsets are rejected. Image execution remains pending. |
| Optional image APIs | `image-sparse.cpp`, `image-atomic.cpp`, `input-attachment.cpp`, `image-64.cpp`, and `CodeGenSPIRVShader/image-extensions.cpp`: sparse fetch/residency, unsigned image atomic add, indexed input attachment reads, signed/unsigned R64 loads/stores, validation and feature reflection at O0/O2; invalid types in `SemaSPIRVShader/image-extensions.cpp` |
| Explicit atomic policy | `atomic-policy.cpp` validates acquire/release/acquire-release and relaxed scope/order operations at O0/O2. The compiler diagnoses invalid pointer scopes, nonconstant orders, invalid load/store/CAS orders and Vulkan-forbidden sequential consistency. |

Every new spelling is included in namespace-isolation checks. Global overload
checks exclude names already occupied by compiler-provided scope constants,
following ordinary C++ namespace rules.

Latest upstream regression run: **1,618 passed, 13 unsupported, 2 expected failures** across SPIR-V
code generation and Shader C++ Sema/CodeGen. All **60** selected shader-port binaries
compile and validate. These checks do not substitute for the remaining features
listed in [GAPS.md](../GAPS.md).

## Accepted declarations with incomplete execution support

`CodeGenSPIRVShader/advanced-stages.cpp` and `fixtures/advanced-stages.cpp`
compile and validate geometry, tessellation control/evaluation, EXT task/mesh
and all six KHR ray stages at `-O0` and `-O2`. They cover execution models/modes,
geometry emission, task payloads, mesh outputs, ray/callable payloads, hit
attributes, shader records and native trace/callable/intersection operations.
Advanced stage semantic and codegen error tests cover invalid configuration
and payload placement. These tests do not exercise hardware pipelines.

Arbitrary pointer reinterpretation, dynamic local record-field offsets, escaping
local pointer loops and source member names in reflection remain unsupported.
Local pointer loops over finite allocations and array offsets are numerically
checked; unsupported bases/escapes have an explicit linker diagnostic.

## Remaining coverage limits

Resource, descriptor, fragment and ray query cases have compilation, operation
inspection and SPIR-V validation coverage. They do not yet have Vulkan rendering,
texture/descriptor execution or hardware ray traversal tests. Atomics run with
32-bit shared storage; 64-bit and physical storage variants are validated but do
not yet have numerical execution tests. Host math and vector/matrix operations
have independent CPU reference results, rather than only emission checks.

Remaining optional image variants and advanced-stage extensions are listed in
`GAPS.md`. Coverage of the operations declared in `SPIRVShaderIntrinsics.def`
does not imply coverage of every SPIR-V extension.

SPIR-V inline assembly has an explicit diagnostic test and no OpAsmINTEL output.
One Slang shared-atomic shader has been ported; these regression fixtures do not
replace the planned broader licensed Slang corpus.

## Upstream regressions

The full `llvm/test/CodeGen/SPIRV`, `llvm/test/Transforms/IndVarSimplify`
and shader Clang suites were run: 1,618 passed, 13 were unsupported and two
were expected failures. There were no unexpected failures.
This checks interactions with logical pointers, HLSL resources, OpenCL kernels,
address-space overload resolution and extension instruction lowering.

The shader-port suite independently validates 120 binaries across `-O0` through
`-O3`, including roughness limiting, both barrier-reduction forms,
whole-record fragment output and both ImGui Vulkan stages. See
[port results](../../shader-ports/RESULTS.md).

The independent [Shadertoy experiment](../../shader-ports/shadertoy/RESULTS.md)
attempts 256 fixed sources at all four optimization levels. It validates 172 ports
at `-O0`, 171 at `-O1`, and 174 each at `-O2` and `-O3`; 169 pass all four.
There were no compiler crashes or backend/validation failures among the
translated inputs. Its
remaining translation and compilation failures are retained separately. The
additional [complex Shadertoy corpus](../../shader-ports/shadertoy/complex/RESULTS.md)
validates 512 more distinct shaders at `-O0`/`-O2`; 486 also pass `-O1`/`-O3`.
Its 1,011-candidate screen retains failures and selection provenance. The
mandatory suite also validates
representative corpus cases for whole-object
zero fills, cross-element byte offsets and vector loads, merge placement, and
nested selection dominance.

Layered image names use `_layers`. `image-types.cpp` and `resources.cpp`
exercise all seven sampled/storage variants. The audit catalog rejects all seven
former `_array` names and checks a four-descriptor layered-image array, including
SPIR-V Arrayed=1, descriptor bounds, element-type reflection and nonuniform access
at both optimization levels.
