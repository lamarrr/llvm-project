# Shader port results

All **120 cases compile and validate** for SPIR-V 1.4 / Vulkan 1.3: 76
Godot/ImGui binaries plus 44 from eleven Vulkan Samples, Filament,
FidelityFX and Slang shader ports, each compiled at `-O0`, `-O1`, `-O2` and
`-O3`. The Godot/ImGui set contains 13 primary Godot configurations, three
template-expanded luminance variants, one whole-record output probe and both
Dear ImGui Vulkan stages, each at all four levels. There are zero failures.

The added ports check a texture/sampler pair, a matrix-based vertex shader, a
particle integration compute shader, complete nonuniform descriptor-indexing and
ray-query fragments, layered Gaussian blur, Filament standard and clear-coat PBR
fragments, one- and two-level subgroup-quad SPD reductions, and shared-memory
atomics. The upstream manifests record source commits and SHA-256 hashes.
See [README.md](README.md) for the precise algorithm
subset and interface adaptations.

Run `python3 shader-ports/test.py` from the repository root. The generated
[report](artifacts/report.json) records compiler version, pinned source hashes,
commands, reflection and operation checks. [README.md](README.md) describes
adaptations and which source paths are covered.

## Resolved compiler gaps

| Original failure | Fix | Regression evidence |
| --- | --- | --- |
| Whole-record fragment output assignment rejected an Output address-space receiver | Trivial copy/move assignment preserves the receiver and result address space; supported record/array copies lower to typed field loads/stores | `record-output.cpp`, blur and tone-map outputs at both optimization levels; Sema assignment/return-type checks; Vulkan record assignment and chained field write; read-only and nontrivial assignment rejection |
| Roughness limiting had no native `acos` | Added scalar and component overloads, GLSL.std.450 Acos lowering, and host math support | Complete roughness port at both levels; scalar/array/record type checks, invalid operands, namespace isolation, host reference values and Vulkan reference values |
| Optimized reduction failed convergence verification | Basic-block merging, jump threading and clone/prune inlining preserve loop/entry tokens across preceding convergent calls | LLVM regressions independently run SimplifyCFG, JumpThreading and AlwaysInliner with verification; all literal-loop luminance variants compile; Vulkan 64-lane reduction yields 2080 at both levels |
| Optimized template reduction failed shared-pointer legalization | Recover typed shared-array access chains from combined byte strides before pointer legalization, preserving nested aggregate boundaries | All template-expanded and optimized literal reductions validate; matrix dynamic indexing remains covered; Vulkan template reduction yields 2080 at both levels |

Fragment output checks require actual Output stores and no generic pointer casts.
Reduction checks verify the 256-byte shared array, workgroup barrier scopes and
AcquireRelease WorkgroupMemory semantics. At `-O0` the literal loop contains two
static barriers; at `-O2` and `-O3` it expands to seven. Template-expanded
variants contain seven at all four levels. The runner rejects missing or extra
operations.

The core compiler suite additionally executes `acos`, record assignment and both
reduction forms through its Vulkan harness at both levels. The `acos` device
comparison uses a 0.0005 relative/absolute floor tolerance; the host comparison
is tighter. Record assignment and reduction checks require exact results.

The engine port suite itself checks compilation, validation, reflection and
structural code generation. It does not execute the texture-based ports or
compare rendered pixels with Godot. These results do not establish full engine
shader coverage or numerical equivalence of every port.

## Dear ImGui integration findings

Both shaders now compile and validate with their original interface locations,
push layout and separate texture/sampler bindings. Porting the vertex stage
required an explicit `__position__ float (*position)[4]` output parameter. It is
restricted to one mutable float[4] pointer in a vertex entry, with no location or
interpolation qualifier, and emits BuiltIn Position on an Output float4 vector.

Stage inputs/outputs now marshal homogeneous two-, three- and four-component
records/C arrays into SPIR-V vectors. Nested record fields preserve the GLSL
color/UV vector grouping. Without this, scalar record members each occupied a
location and ImGui's adjacent vertex attributes overlapped. Reflection retains
the original C++ layout while the binary exposes the vector stage ABI.

Dynamic component indexing uses vector extraction/insertion rather than a
backend component-pointer cast. Permanent tests cover dynamic Position indexing,
attribute placement, wrong stage/type/width, read-only destinations, duplicate
Position outputs and conflicting location/interpolation attributes. Existing
Godot ports and compiler regressions remain covered.
