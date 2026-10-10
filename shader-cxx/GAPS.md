# SPIR-V feature implementation status

This inventory retains the full scope requested in the gap discussion. It is
not a claim that every SPIR-V feature now has a native C++ API. Inline assembly
provides access to additional valid SPIR-V operations, but does not replace
missing entry-stage, interface, resource or intrinsic APIs.

## Implemented in this expansion

- Direct SPIR-V extended assembly with scalar/vector/memory/immediate operands,
  named and tied operands, read/write and multiple outputs, type/alignment
  substitutions, local IDs, memory effects and module-section hoisting.
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
- Ray-query scalar/vector getters for distances, flags, instance/SBT identifiers,
  barycentrics, AABB opacity and object/world ray origin/direction.
- Explicit scoped barriers, device/subgroup shader clocks and fragment demotion.
- Floating add/sub/min/max atomics with storage/width-specific feature reflection.
- Extended subgroup scans, clustered and logical operations, scalar/vector
  floating classification, carry/borrow/extended multiply, constant image
  offsets, and explicit atomic scope/order overloads.

See [LANGUAGE.md](LANGUAGE.md) for signatures and [tests/COVERAGE.md](tests/COVERAGE.md)
for the executable evidence.

## Still outstanding

| Area | Required work |
| --- | --- |
| Scalar and matrix types | Float8 E4M3/E5M2, bfloat16 and other extension encodings; matrices beyond the current `f32` utility type; cooperative matrix types |
| Integer and classification APIs | Further optional integer extensions and classification edge-case coverage; carry/borrow, extended multiply and aggregate classification APIs are present |
| Image APIs | Exact depth/comparison-image types and sampling/gather methods; dynamic offsets and additional legal operand combinations; sparse texels/residency; image atomics; input attachments; 64-bit texels |
| Fragment interfaces | Interpolate-at-centroid/sample/offset intrinsics, depth/sample-mask outputs, clip/cull distances, point size and additional execution modes |
| Subgroups | Comprehensive invalid selector checks and optional subgroup extension families; core inclusive/exclusive, clustered and logical operations are present |
| Atomic policy | Image atomics and remaining floating operations; explicit scope/order overloads are present for pointer atomics |
| Ray queries | Object-to-world/world-to-object matrix getters |
| Advanced stages | Geometry, tessellation, task/mesh and ray-stage lowering; their complete builtins, execution modes, payloads, mesh emission and trace-ray operations |
| Specialization | Workgroup-size IDs and composite specialization constants |
| Optional extension APIs | Cooperative matrices, integer dot products, fragment interlock, fragment rate and barycentrics |
| C++ evaluation | Constexpr support for eligible native math/intrinsic templates |
| Vector utility address spaces | `vec.hpp` member functions need Uniform/PushConstant-compatible receivers; the Filament PBR ports currently materialize local vectors from their scalar fields. Constant logical vector component indices now narrow to 32 bits, avoiding an unnecessary `shaderInt64` requirement. Dynamic 64-bit indices and physical-address arithmetic retain their required width. |
| Reliability and reflection | Broader pointer reinterpretation/byte-address patterns, remaining backend failure cases, source member names and complete optional-feature inference for arbitrary inline assembly |
| Execution and corpus | Image/descriptor/fragment rendering, extension-enabled floating atomic/clock tests, hardware ray traversal, broader Slang/Vulkan Samples/Filament/FidelityFX ports and the outstanding Shadertoy translation failures |

## Verified results

The current suite passes **658 mandatory checks**. A previous optional Vulkan
execution run passed **129 checks** on the test device. Floating extension operations,
images and ray-query getters have compile/validation/codegen checks; these counts
must not be read as hardware execution coverage for those families.

The LLVM/Clang run passed **1,618 tests**, with **13 unsupported skips** and
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
