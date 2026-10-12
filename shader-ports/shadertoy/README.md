# Shadertoy compilation experiment

This corpus contains 256 distinct real Shadertoy Image shaders and their C++
ports. It tests compilation to SPIR-V 1.4 for Vulkan 1.3. It is separate from the
Godot/ImGui corpus and the compiler feature suite.

The separate [complex corpus](complex/README.md) adds 512 distinct, larger
Image shaders that pass at `-O0` and `-O2`. It retains a full screen
of 1,011 candidates and does not replace this original, unfiltered experiment.

## Source selection and attribution

The source is the public [Shadertoy dataset](https://github.com/Vipitis/shadertoys-dataset),
pinned to commit `7a5d34083b03501a4d42238bbe65183b0b531f00`, file
`data/annotated/api/api_2023-01.jsonl`. Selection takes the first 256 unique IDs
in lexical order with a nonempty Image pass and no Buffer, Sound or Cubemap
render passes. Common code and texture inputs are allowed. Selection is fixed
before compilation and does not exclude failing shaders.

[upstream/manifest.json](upstream/manifest.json) records source URLs, authors,
individual license metadata, the dataset hash and hashes of all original JSON
records. The originals retain Image/Common source text. Their licenses vary;
the compiler project's license does not replace those licenses.

Shader IDs are case-sensitive. Colliding filenames include an ASCII hexadecimal
ID suffix, for example `dtfSRs-647466535273.cpp`, so they remain distinct on
case-insensitive filesystems. The manifest's `file` field determines the matching
port and artifact stems; shader IDs, source URLs and pinned JSON hashes are
unchanged. New snapshots use this suffix for every ID.

## Reproduce

From the repository root, after building Clang, clang-spirv-link, clang-format,
spirv-val and spirv-dis:

```sh
python3 shader-ports/shadertoy/test.py
python3 shader-ports/shadertoy/test.py --strict
```

The default command completes the experiment and records failures. Its zero exit
status does not mean every shader passed. `--strict` requires all 256 shaders to
pass at `-O0`, `-O1`, `-O2` and `-O3`. Compilation has a 45-second limit per
case; on timeout the runner terminates that compiler's entire process group.
Use `--jobs N` to change concurrency (default 8).

The runner verifies original hashes, regenerates and formats ports, and performs
1,024 attempts, including failed translations as failed cases at all four levels.
Artifacts, diagnostics, commands, source/support hashes, compiler version and a
complete report are written under `shader-ports/artifacts/shadertoy/`. Stale
binaries, disassemblies and logs are removed before each attempt.

[RESULTS.md](RESULTS.md) describes the recorded run. The generated
`shader-ports/artifacts/shadertoy/report.json` retains every four-level
outcome, including failures. [results.json](results.json) preserves the earlier
`-O0`/`-O2` run.

## Porting model

`port.py` is a bounded source translator, not a complete GLSL frontend. It handles
ordinary float literals, helper calls, invocation context, GLSL vector swizzles
and simple swizzle assignments. Unhandled writable swizzles are rejected. Other
unsupported constructs may fail during C++ compilation. A successful translation
is not a proof of GLSL/C++ semantic equivalence.

`support.h` is an explicit, project-local C++ compatibility library. It is not a
compiler default header and adds no native compiler types or global interfaces.
The reusable `spirv::vec<T, N>` and `spirv::mat<Rows, Columns>` types come from
installed Clang SPIR-V resource headers, available as `<spirv/vec.hpp>` and
`<spirv/mat.hpp>`. Named scalar, vector, and matrix aliases are in
`<spirv/types.hpp>`. `support.h` includes it for existing ports and exposes
the helpers through `namespace glsl`. Vectors expose a public C array, indexing,
`.x()` through `.w()`, every two-, three- and four-component
named swizzle such as `.xx()`, `.zyx()` and `.wzyx()`, and the generic
`swizzle<indices...>()`. Named swizzles return values; `set_swizzle` handles
writes. Vector arithmetic calls native overloaded math intrinsics. Matrices use
row-major C arrays and `operator[]`, with GLSL constructor ordering. Their
operators call the native `mul` intrinsic, which emits SPIR-V `OpDot` for each
row/column product. Neither type adds implicit padding or
extra alignment: `f32x3` is 12 bytes and `f32x3x3` is 36 bytes. The translator
maps GLSL type spellings to width-explicit aliases such as `u32`, `f32x3`, and
`f32x3x3`. The entire
15-operation GLSL.std.450 angle/trig family uses native compiler functions;
`atan(y, x)` maps to `atan2(y, x)`. Resource wrappers call native texture methods
with scalar coordinates.

The fragment entry takes output, push constants, images and samplers as explicit
parameters. Texture channel N uses set 0/binding N; its sampler uses set 1/binding
N. A 128-byte push block supplies resolution/time, mouse/date, frame/delta/rate,
and channel dimensions/times. Shader-scope GLSL uniforms are emulated by ordinary
invocation-local C++ state populated by the entry; resource bindings are never
global variables. External images/audio/video/keyboard inputs require a host to
provide matching texture content. This experiment does not supply such a host.

Only literal zero-LOD `texelFetch` is mapped to the existing mip-zero load API.
Nonzero/dynamic fetch LOD remains a compilation failure rather than silently
being discarded. Multipass shaders are outside this selected corpus.

## What a pass means

A pass requires compilation, `spirv-val --target-env vulkan1.3
--scalar-block-layout`, the exact SPIR-V 1.4 binary version, fragment entry with
normal C++ mangling, one 16-byte output at location 0, a 128-byte push block when
retained, and no unresolved helper calls. Disassembly and reflection are retained.

The experiment does not render images, compare pixels, execute texture inputs,
or compile the original GLSL as a reference. Unsupported GLSL constructs,
translator defects and compiler/backend defects must be distinguished when
investigating failures. The dedicated compiler suite separately performs
numerical Vulkan checks for the complete trig family.
