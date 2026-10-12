# Shader port experiments

This directory contains shader-only C++ ports of selected **Godot 4.5-stable**,
**Dear ImGui Vulkan**, **Khronos Vulkan Samples**, **Filament**, **FidelityFX
SPD**, and **Slang** shaders. It is separate from the compiler's feature fixtures. No engine
code, renderer integration, host application or default shader header is added.

The pinned source is Godot commit
[`876b290332ec6f2e6d173d08162a02aa7e6ca46d`](https://github.com/godotengine/godot/tree/876b290332ec6f2e6d173d08162a02aa7e6ca46d/servers/rendering/renderer_rd/shaders/effects).
Unmodified originals and the MIT license are in [godot/upstream](godot/upstream).
[manifest.json](godot/upstream/manifest.json) records each upstream URL and SHA-256.
The runner verifies those hashes and works offline once the compiler is built.

A separate [256-shader Shadertoy experiment](shadertoy/README.md) preserves all
source records and failed outcomes. Its runner and success criteria are documented
there; it is not included in the engine runner below.

An additional [512-shader complex Shadertoy corpus](shadertoy/complex/README.md)
contains distinct, larger Image shaders with helper functions and loops. All
512 pass at `-O0` and `-O2`, and 486 pass all four levels. The other `-O1` and
`-O3` results, 1,011-candidate screen and failures are retained separately.
This corpus is also outside the engine runner below.

## Run

From the repository root:

```sh
python3 shader-ports/test.py
```

The runner uses `build/bin/clang++`, `spirv-val` and `spirv-dis`. Override their
directory with `--bin-dir /path/to/bin`; use `--output-dir` to relocate artifacts
and `--jobs` to change compilation concurrency.

Successful cases leave `.spv`, `.spvasm` and `.spv.json` files in the ignored
`artifacts/` directory. Every run writes `artifacts/report.json`, including the
compiler version, commands, checks, observed operation counts and diagnostics.
Failed builds cannot reuse stale binaries or disassemblies.

`PASS` means compilation, SPIR-V validation and all case-specific checks passed.
Every case must pass; compilation failures and mismatched code generation return
nonzero. `--strict` remains accepted and has the same behavior as the default.

## Ports and outcomes

| Port | Selected upstream path | `-O0` | `-O1` | `-O2` | `-O3` |
| --- | --- | --- | --- | --- | --- |
| [copy.cpp](godot/copy.cpp), `COPY_SIMPLE` | `copy.glsl`, `MODE_SIMPLE_COPY`; fetch/sample selection, Y flip, luminance and alpha flags | Pass | Pass | Pass | Pass |
| `COPY_DEPTH` | `MODE_LINEARIZE_DEPTH_COPY`; depth reconstruction and Y flip | Pass | Pass | Pass | Pass |
| `COPY_CUBE` | `MODE_CUBEMAP_TO_PANORAMA`; spherical direction and explicit LOD | Pass | Pass | Pass | Pass |
| `COPY_CUBE_ARRAY` | `MODE_CUBEMAP_ARRAY_TO_PANORAMA`; spherical direction and image layer | Pass | Pass | Pass | Pass |
| [luminance_reduce.cpp](godot/luminance_reduce.cpp), `READ_TEXTURE` | `luminance_reduce.glsl`, `READ_TEXTURE`; RGB maximum and shared reduction | Pass | Pass | Pass | Pass |
| `INTERMEDIATE` | Neither upstream define; storage-image reduction | Pass | Pass | Pass | Pass |
| `WRITE_LUMINANCE` | `WRITE_LUMINANCE` without `READ_TEXTURE`; previous exposure, interpolation and clamp | Pass | Pass | Pass | Pass |
| The three luminance variants with `UNROLL_REDUCTION` | Equivalent six fixed reduction levels, expanded with templates | Pass | Pass | Pass | Pass |
| [blur.cpp](godot/blur.cpp), `BLUR_GAUSSIAN` | `blur_raster.glsl`, fragment `MODE_GAUSSIAN_BLUR`; thirteen taps | Pass | Pass | Pass | Pass |
| `BLUR_MIPMAP` | Fragment `MODE_MIPMAP`; four bilinear samples | Pass | Pass | Pass | Pass |
| [tonemap.cpp](godot/tonemap.cpp), `TONEMAP_LINEAR` | `tonemap.glsl`, linear curve, exposure and optional sRGB encoding | Pass | Pass | Pass | Pass |
| `TONEMAP_REINHARD` | Reinhard curve with white normalization | Pass | Pass | Pass | Pass |
| `TONEMAP_FILMIC` | Filmic curve with white normalization | Pass | Pass | Pass | Pass |
| [roughness_limiter.cpp](godot/roughness_limiter.cpp) | `roughness_limiter.glsl`; original `acos` roughness formula | Pass | Pass | Pass | Pass |

All thirteen primary configurations compile and validate at all four optimization
levels. Three template-expanded luminance variants and a minimal whole-record
fragment-output test also pass at all four levels: **68 validated binaries, zero
failures**. See [RESULTS.md](RESULTS.md) for the resolved compiler issues and
regression coverage.

## Dear ImGui Vulkan shaders

[vertex.cpp](imgui/vertex.cpp) and [fragment.cpp](imgui/fragment.cpp) port both
original Vulkan shaders at commit
[`adc0027912d46e42bc5cd8af235e7b325867d410`](https://github.com/ocornut/imgui/tree/adc0027912d46e42bc5cd8af235e7b325867d410/backends/vulkan).
Unmodified shaders and the MIT license are in [imgui/upstream](imgui/upstream);
the separate manifest pins their URLs and hashes. Both stages pass at `-O0`, `-O1`,
`-O2` and `-O3`, bringing the Godot/ImGui suite to **76 validated binaries**.

The vertex port preserves position/UV/color input locations 0/1/2, a 16-byte
push block with scale at offset 0 and translation at offset 8, and the clip-space
formula `(position * scale + translation, 0, 1)`. `__position__` writes the
SPIR-V built-in Position through a C-array pointer. The ordinary record varying
contains color followed by UV; vector marshalling makes these occupy locations
0 and 1, matching the GLSL interface. The fragment port preserves the original
texture at set 0/binding 0 and sampler at set 1/binding 0. It samples at the UV
and multiplies all four color channels by the sampled texel.

Tests check the actual SPIR-V vector types, matching stage interfaces, BuiltIn
Position, resource bindings, push layout, scale/translation operations and one
implicit-LOD sample with four-component color modulation. Entry points retain
normal C++ mangling; reflection supplies their pipeline names. Renderer code,
vertex formats, blending and draw submission are outside these shader-only ports.
There is no ImGui pixel comparison or graphics-pipeline execution test.

## Additional upstream ports

Each directory below contains unmodified selected upstream sources, a license,
and a manifest with an exact commit and SHA-256 hashes. The C++ translations are
shader-only adaptations, with entry interfaces expressed as parameters. All eleven
ports pass compilation, Vulkan 1.3 validation and operation checks at `-O0`, `-O1`,
`-O2` and `-O3`, raising this runner to **120 validated binaries**.

| Project | Ported path | Checked SPIR-V |
| --- | --- | --- |
| [Vulkan Samples](vulkan_samples/upstream/manifest.json) | Separate image/sampler fragment and vertex pair | Separate bindings, image sample, Position and eight matrix-row dot operations |
| Vulkan Samples | N-body particle integration compute pass | Physical particle pointer, 256-lane workgroup and velocity/position math |
| Vulkan Samples | Full nonuniform descriptor-indexing fragment | Runtime texture array, NonUniform index and Flat input decorations, implicit sample |
| Vulkan Samples | Full ray-query shadow and ambient-occlusion fragment | Nine AO queries in nested loops, a shadow query, ray intersection type/distance and camera/light uniform |
| [Filament](filament/upstream/manifest.json) | EVSM layered Gaussian blur | Layered texture fetches, clamped positive/negative taps and 16 C-layout weights |
| [Filament PBR](filament/pbr.cpp) | Standard metallic/roughness fragment | Base/ORM/normal maps, normal mapping, directional and point lights, GGX/Smith/Schlick BRDF, DFG LUT and prefiltered cubemap |
| Filament PBR | Clear-coat fragment | Standard path plus geometric-normal clear-coat direct lobe and an additional prefiltered environment sample |
| [FidelityFX SPD](fidelityfx/upstream/manifest.json) | First-level 2×2 quad reduction | One fetch per lane, three quad swaps and one image write per quad |
| FidelityFX SPD | Two-level `SpdDownsampleMips_0_1_Intrinsics` path | 16 source fetches, 12 quad swaps and eight writes over two destination mip views per 256-lane workgroup |
| [Slang](slang/upstream/manifest.json) | `atomics-groupshared.slang` | Three workgroup atomics and two workgroup barriers |

The Vulkan vertex matrices use C row-major storage; host data from the original
column-major UBO would need transposition. Its N-body source uses a specialization
ID for the workgroup size, while this port fixes it at 256. The particle buffer
uses a physical device-address pointer instead of the source descriptor. Neither
port is a drop-in replacement for its Vulkan sample.

Filament normally generates its material interface. This port supplies a manual
88-byte push record and supports kernel radii 0–15. The 15 positive/negative
weight pairs are expanded with constant indices because optimized dynamic
push-array indexing currently fails pointer legalization. The two PBR variants
port Filament's selected standard surface BRDF and clear-coat equations into a
manual 96-byte frame uniform and 48-byte material push record. They use fixed
one-directional/one-point lighting and a prefiltered cubemap. Filament's froxel
light lists, shadowing, SSR, refraction, anisotropy, iridescence and generated
material variants are outside this slice. C++ `<spirv/vec.hpp>` methods cannot directly
read Uniform/PushConstant receivers yet, so these shaders materialize their
record vectors from scalar components before vector operations. The runner also
validates an unselected build containing both PBR entries in one SPIR-V module
at each optimization level.

FidelityFX now covers two SPD reduction levels for the intrinsic 256-lane path,
using a four-sample average as the caller-provided reduction. The two outputs
are separate storage-image bindings for the two mip views. Later mip levels,
the global last-tile counter and FP16 permutations remain unported. The Slang
port preserves its shared-memory atomic algorithm but has not been compared
numerically with Slang's runtime harness.
The ray-query and descriptor-indexing fragments preserve their complete shader
algorithms, with the ray uniform represented in C layout. These selected cases
establish compilation and structural code generation; rendering, hardware ray
traversal and pixel equivalence remain untested.

## Port adaptations

GLSL's global resource/interface declarations become entry parameters.
Push-constant blocks become ordinary C++ records behind `__push_constant__`
pointers. Layout sizes and every top-level member offset are checked against the
selected original std430 blocks, using both static assertions and reflection.

Combined GLSL samplers are split into textures and samplers. Sampled copy/blur/
tone-map paths keep the original texture at set 0, binding 0 and add the sampler
at set 0, binding 1. Fetch-only paths do not need a sampler; unused entry
resources are removed. Destination and previous-luminance bindings retain the
original sets/bindings. The resulting descriptor ABI needs corresponding host
bindings; these ports are not drop-in engine binaries.

`Float2`, `Float3` and `Color` are ordinary homogeneous C++ records in
[types.h](godot/types.h), an explicitly included project header. Vector arithmetic
uses intrinsic overloads such as `add` and `mul`, or equivalent scalar expressions.
Texture methods use scalar coordinates and return values. Fragment outputs use
ordinary whole-record assignment.

Luminance scratch becomes function-local `static __shared__ float scratch[64]`.
`workgroup_barrier()` combines the original workgroup memory barrier and control
barrier. Its emitted scopes and AcquireRelease WorkgroupMemory semantics are
checked. `UNROLL_REDUCTION` preserves the six strides 32, 16, 8, 4, 2 and 1 and
all seven barriers. Both the literal loop and template-expanded forms compile
and validate at all four optimization levels.

Tone-map curves are selected at compile time rather than by `params.tonemapper`;
the full push-block layout is retained. Exposure, luminance scaling, alpha
preservation and the original optional sRGB conversion are ported. Other tone-map
flags must be unset for this selected configuration: auto exposure, glow, FXAA,
BCS, color correction and debanding are outside this port. ACES/AgX curves are
also outside it. Blur ports include only the two named fragment variants.

Original predicates are preserved, including luminance reduction's
`any(less_than(position, source_size))` and roughness limiting's strict `>` bound.
Their edge-access behavior is an upstream-algorithm caveat, not a compiler fix
made by these ports. No texture boundary behavior is established by compilation.

## What code generation is checked

Each successful build must emit SPIR-V 1.4, validate for Vulkan 1.3 with scalar
block layout, and match its reflected stage and normally mangled C++ entry name.
The suite checks:

- Exact image fetch/read/write/sample instruction counts for each selected path.
- Thirteen implicit-LOD blur taps and the original `0.125`/`0.03125` weights;
  four implicit-LOD mipmap taps.
- Cube versus cube-array image types, explicit LOD and sine/cosine operations.
- Storage-image formats `Rgba16f` and `R32f`, descriptor sets/bindings, input/output
  locations, push sizes/offsets and reflected limits.
- Compute workgroups of 8×8×1, bounds/control flow, 256 bytes of shared reduction
  storage, reduction loop/stride or explicit levels, and barrier scopes/semantics.
- Depth/curve divisions, three sRGB `Pow` operations for the color channels, and
  roughness limiting with native `Acos`, `Pow` and `Sqrt`.
- Four stores to actual Output storage for fragment color, and fully inlined
  helper calls.

These are structural codegen checks against the selected source algorithms.
There is no GPU execution, pixel comparison, differential compilation against
Godot's GLSL compiler, or proof of numerical equivalence. Full shader permutations
and vertex-stage fullscreen positioning are outside this initial suite. Unreal
shaders are not included in this first set.
