# SPIR-V feature implementation status

This inventory retains the full scope requested in the gap discussion. It is
not a claim that every SPIR-V feature now has a native C++ API. Inline assembly
provides access to additional valid SPIR-V operations, but does not replace
missing entry-stage, interface, resource or intrinsic APIs.

## Implemented in this expansion

- Core geometry/tessellation, EXT task/mesh and all six KHR ray-tracing stages:
  execution models/modes, per-vertex arrays, task/ray/callable payloads, hit
  attributes, shader records, geometry/mesh emission and trace/callable/hit
  operations. Twelve entries compile, validate and reflect at both `-O0`
  and `-O2` (24 checks); this is not hardware pipeline execution coverage.

- Direct SPIR-V extended assembly with scalar/vector/memory/immediate operands,
  named and tied operands, read/write and multiple outputs, type/alignment
  substitutions, local IDs, memory effects and module-section hoisting.
- Headerless `printf` for non-semantic SPIR-V debug output with a literal
  format and numeric scalar/vector arguments.
- Component rounding/sign/fract, exp2/log2, inverse square root, step/smoothstep,
  fused multiply-add and floor-based modulo.
- Return-value modf/frexp decomposition and scalar/vector ldexp.
- Matrix/vector multiplication in both orders, matrix scaling in both orders,
  outer products, scalar conversion and one-by-one matrices.
- Scalar/vector bit count/reversal/extraction/insertion and least/most
  significant set-bit searches, including Vulkan 1.3-compatible 64-bit lowering.
- Signed/unsigned normalized, half and double packing/unpacking.
- Vector subgroup arithmetic/broadcast/shuffle/all-equal, inverse ballot,
  ballot extraction/find/scans and quad exchanges.
- Explicit fetch mip/sample and multisample storage selectors, biased sampling,
  image LOD queries and level/sample counts.
- Depth image comparison sampling and gathering with comparison samplers,
  explicit LOD and constant offsets.
- Sparse 2D float texel fetch with residency test, 32-bit unsigned image
  atomic add, indexed fragment input attachments and R64 signed/unsigned
  storage texel loads and stores.
- Fragment depth and sample-mask output interfaces, including read-only
  sample-mask input.
- Early fragment tests, conservative depth modes, post-depth coverage and
  fragment stencil-reference output.
- Color, depth and stencil tile-image reads, sampled reads and noncoherent
  attachment modes, with attachment-location and feature reflection.
- Ordered and unordered pixel, sample and shading-rate fragment interlock.
- Fragment centroid/sample/offset interpolation and clip/cull-distance inputs
  and outputs, plus vertex point-size output.
- Dynamic gather coordinate offsets under Vulkan 1.3.
- Specialization-controlled workgroup dimensions and two-, three- and
  four-component vector specialization composites.
- Four-lane 8-bit signed, unsigned and mixed integer dot products, including
  saturating accumulate.
- Fused row-major `f32` subgroup cooperative matrix multiply-add over shared
  arrays, with native SPIR-V operations and Vulkan feature reflection.
- Fragment barycentric coordinates, shading-rate input and ordered pixel
  interlock operations, with Vulkan feature reflection.
- Ray-query scalar/vector getters for distances, flags, instance/SBT identifiers,
  barycentrics, AABB opacity and object/world ray origin/direction.
- Ray-query object-to-world and world-to-object matrix getters with 3-by-4
  row-major C++ results.
- Explicit scoped barriers, device/subgroup shader clocks and fragment demotion.
- Floating add/sub/min/max atomics with storage/width-specific feature reflection.
- Extended subgroup scans, clustered and logical operations, scalar/vector
  floating classification, carry/borrow/extended multiply, constant image
  offsets, and explicit atomic scope/order overloads.
- Address-space-preserving vector and matrix component access, swizzles and
  storage writes from Uniform, PushConstant and Storage receivers. Constant
  logical vector component indices use 32-bit indices; dynamic 64-bit indices
  and physical-address arithmetic retain their required width.

See [LANGUAGE.md](LANGUAGE.md) for signatures and [tests/COVERAGE.md](tests/COVERAGE.md)
for the executable evidence.

## Still outstanding

| Area | Required work |
| --- | --- |
| Scalar and matrix types | Float8 E4M3/E5M2, bfloat16 and other extension encodings; matrix utility element types other than `f32` |
| Integer and classification APIs | Additional optional integer-extension APIs and classification boundary-case tests |
| Image APIs | Sparse sampling/gathering and additional sparse image dimensions/texel types; multisampled input attachments; coverage of additional legal image operand combinations |
| Fragment interfaces | Per-vertex barycentric inputs; fully-covered and fragment-invocation-density builtin interfaces |
| Subgroups | Comprehensive invalid-selector tests and additional optional subgroup-extension APIs |
| Advanced stages | Optional execution modes, stream-qualified interfaces, primitive shading-rate output and ray-stage object/world matrix getters |
| Specialization | Array/struct specialization composites and specialization-constant expressions beyond direct scalar/vector assembly |
| Cooperative matrices | Public cooperative matrix value types and independent load/store APIs; mixed element/accumulator types, including `f16`/`f32` and 8-bit/32-bit integers; column-major layouts and explicit strides; extension-gated workgroup scope; fill, component access, length queries, conversions, element-wise arithmetic, scalar multiplication and integer saturating accumulation |
| Integer dot products | Packed four-byte scalar operands; wider component/result types and vector lengths other than four |
| C++ evaluation | Constant evaluation for additional pure intrinsic families, including native geometric/matrix and integer-dot operations |
| Reliability and reflection | Signed-image atomic pointer-type validation failure; broader pointer reinterpretation/byte-address patterns; direct nonzero array-field indexing through physical struct pointers at `-O0`; source member names and complete optional-feature inference for arbitrary inline assembly |
| Execution and corpus | GPU execution coverage for image/descriptor/fragment pipelines, extension-enabled floating atomics/clocks and ray traversal; broader Slang/Vulkan Samples/Filament/FidelityFX ports; unresolved Shadertoy translations |

## Verified results

The last complete suite run passed **658 mandatory checks**. A previous optional Vulkan
execution run passed **129 checks** on the test device. Floating extension operations,
images and ray-query getters have compile/validation/codegen checks; these counts
must not be read as hardware execution coverage for those families.

The recorded LLVM/Clang run passed **1,618 tests**, with **13 unsupported skips** and
**two expected failures**.
All **120 shader-port binaries**, including 44 from the newly selected four
projects, compiled and validated across `-O0` through `-O3`. The 256-item
Shadertoy experiment was rerun at `-O0`, `-O1`, `-O2` and `-O3`; its compiler
and translation failures remain recorded separately. It validates 169 shaders
at all four levels with no compiler crashes or backend/validation failures in
the final corpus run.
The separately selected [512-shader complex corpus](../shader-ports/shadertoy/complex/README.md)
validates all 512 at `-O0` and `-O2`; 486 pass all four optimization levels.
Its 1,011-candidate screening log retains failed translations and builds;
the selected pass rate is not a corpus-wide success-rate estimate.
