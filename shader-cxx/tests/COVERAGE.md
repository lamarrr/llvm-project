# Shader C++ feature coverage

Run `python3 shader-cxx/tools/test.py` for 94 mandatory cases. Add `--runtime`
for 15 Vulkan execution checks (109 total on a compatible device). Mandatory
checks include both default optimization and `-O2` for every positive fixture.
Runtime skips are explicit; they never skip compilation or SPIR-V validation.
The runner uses explicit checks that remain active under `python3 -O`.
Coverage guards fail if the compiler adds a scope builtin or scalar math alias
without an independent test, or if a fixture is omitted from the compilation matrix.

| Implemented surface | Coverage and evidence |
| --- | --- |
| Headerless C++20 mode and 64-bit C++ sizes | `SemaSPIRVShader/headerless.cpp`, `catalog.cpp`; C++17/profile rejection; real examples contain no includes |
| Keyword isolation from ordinary C++ | `SemaSPIRVShader/host-gating.cpp` uses every keyword family as ordinary host identifiers |
| Templates, concepts, constexpr, consteval, classes, lambdas, loops | `fixtures/cxx20.cpp`: compile/validate and execute at both optimization levels; `scope.cpp`: array borrowing and lambda captures |
| Compute/vertex/fragment entry wrappers | `CodeGenSPIRVShader/entry.cpp`; `examples/stages.cpp`; execution model, origin and selected-entry reflection assertions |
| Workgroup dimensions and synthesized `workgroup_size` | `scope.cpp`: dimensions 2/3/4 and no WorkgroupSize input; missing, zero, negative, conflicting and inappropriate-stage dimensions are rejected |
| Exactly two positional binding arguments | `examples/uniform.cpp`: DescriptorSet 2/Binding 7 in binary and reflection; arity, negative, overflow, runtime value and conflict failures |
| Stage input/output locations, scalar value input and pointer output | `examples/stages.cpp`, `global-interfaces.cpp`; inspect binary/reflection locations; writes to inputs and value outputs are rejected |
| Storage device addresses | PhysicalStorageBuffer capability, extension, stride and address-conversion assertions; device address supplied by Vulkan host harness |
| Pointer increment/decrement, indexing, `*`, `->`, comparison, difference | `pointers.cpp`: emitted operations, validation and runtime record fields/values at both optimization levels |
| Pointer alignment and weakening after arithmetic | `examples/alignment.cpp`: emitted Aligned 16 then Aligned 4; runtime at both levels; zero, non-power-of-two and negative powers are rejected |
| Uniform/push read-only pointees | `SemaSPIRVShader/diagnostics.cpp`; input-write and missing/misplaced binding rejection |
| Shared workgroup memory | `shared.cpp`: emitted Workgroup pointer, validation and runtime at both levels |
| C++ record layout, nested records/arrays, explicit alignment | `layout.cpp`: reflected 48-byte size, 16-byte alignment, field offsets and array strides; native compiler static assertions; validation at both levels |
| Invocation-local initialized globals | `invocation-state.cpp`: helper mutations are visible within the invocation; two dispatches verify initialization is renewed; both levels |
| All current scope-value public types | `catalog.cpp` independently pins all 40 names, scalar types and array extents, including advanced-stage declarations |
| Compute/subgroup scope lowering | `scope.cpp`: every current compute/subgroup BuiltIn number checked in reflection |
| Vertex scope lowering | `vertex-scope.cpp`: vertex/instance/base/draw/view values; DrawParameters/MultiView capabilities and reflected feature requirements |
| Fragment scope lowering | `fragment-scope.cpp`: primitive/sample/coordinate/boolean values; SampleRateShading/Geometry requirements; boolean types and integer builtin Flat decoration |
| Actual C arrays, borrowing, decay, component shorthand | `headerless.cpp`, `catalog.cpp`, `scope.cpp`; all x/y/z/w component widths; runtime-constexpr and invalid shorthand checks |
| ImageFormat scoped enum and template argument use | `catalog.cpp` checks all 42 enumerator values and unsigned underlying type; `headerless.cpp` checks NTTPs; implicit integer conversion is rejected |
| All 14 headerless scalar math intrinsics | `math-all.cpp`: independently listed math calls, emitted GLSL instruction checks and CPU-reference runtime values at both levels |
| Link translation units before legalization | Two temporary source files with helper/template code; bitcode linking, validation, no remaining function calls and Vulkan execution |
| Entry selection | Both entries from `stages.cpp`; multiple/no/unknown entry failures |
| Shader call restrictions | Direct/mutual recursion, unresolved functions/variables, indirect calls and dynamic global initialization rejected without crashes |
| SPIR-V 1.4/Vulkan 1.3 profile | Binary magic/version checks, real spirv-val invocation, capability/extension/feature reflection; incompatible module triple rejected |
| Reflection | Entry, stage, dimensions, bindings, locations, BuiltIns, layout, device addresses, capabilities, extensions and used feature requirements checked |
| Validation before artifact publication | Forced validator failure leaves no binary, JSON or temporary file; existing successful binary/JSON remain intact |
| LLVM inspection output | Native linker's `--emit-llvm` output checked for selected entry |

## Accepted declarations with incomplete execution support

The 10 advanced stage spellings (tessellation control/evaluation, task, mesh and
six ray stages) each have a specific unsupported-stage diagnostic test. Their
scope values have declaration/type coverage in `catalog.cpp`; this does not claim
advanced-stage code generation works. Stage availability for otherwise declared
builtins is still chiefly enforced by SPIR-V validation.

Pointer alignment outside immutable uniform/push record fields, full physical
pointer null/alias behavior, arbitrary reinterpretation, source member names in
reflection and complete feature/limit inference remain incomplete. This suite
covers their currently implemented subset, not those remaining requirements.

## Features not implemented

Public `__flat__` and the other interpolation qualifiers, native resource
templates, vector/matrix intrinsic marshalling, advanced intrinsic families,
specialization constants and host math mode are not implemented. `__flat__` has
an explicit rejection test so it cannot be mistaken for working syntax. The
compiler supplies required Flat decoration for integer *builtin* fragment inputs;
that is distinct from a user-facing interpolation attribute.

SPIR-V inline assembly has an explicit diagnostic test and no OpAsmINTEL output.
No Slang tests have been ported; these regression fixtures do not replace the
planned licensed Slang corpus.

## Upstream regressions

The full `llvm/test/CodeGen/SPIRV` suite plus the shader Clang tests was run:
1,335 passed and three unsupported skips. The target-triple unit tests also pass.
This catches interactions with pre-existing logical pointers, HLSL resources,
OpenCL kernels and extension instruction lowering.
