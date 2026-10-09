# 512 additional complex Shadertoy ports

This corpus adds **512 distinct Image shaders** to the earlier 256-shader
experiment. Every selected C++ port compiles to SPIR-V 1.4 and validates for
Vulkan 1.3 at `-O0` and `-O2`; 486 also pass `-O1` and `-O3`. It exercises
substantial real shader code, including helper functions, loops,
vector/matrix operations and texture inputs.

The source is the public [Shadertoy dataset](https://github.com/Vipitis/shadertoys-dataset)
at commit `7a5d34083b03501a4d42238bbe65183b0b531f00`, using its 24 monthly
`data/annotated/api/api_2022-*.jsonl` and `api_2023-*.jsonl` files. The
[manifest](upstream/manifest.json) pins every dataset-file hash, selected shader
ID, original record hash, author, license, source URL and complexity measures.
Individual unmodified records are retained in [upstream](upstream); licenses
vary by shader. Records marked `proprietary-license` or
`unknown-license-reference` are excluded. The earlier 256 IDs and duplicate
source code are also excluded.

Selection requires one Image pass and no Buffer, Sound or Cubemap passes, at
least 3,000 characters of Image/Common code, three function definitions and one
loop. Candidates were screened in ascending size, first at `-O2`, then at `-O0`
if `-O2` passed. Of 1,011 screened candidates, 535 passed both; 521 of those
had allowed license metadata. The longest 512 eligible passing candidates were
frozen. [screening.jsonl](screening.jsonl) retains every
attempted candidate and failure. This is a **compile-screened showcase**, not
an unbiased estimate of the compiler's pass rate on all shaders.

The selected originals range from 3,049 to 20,482 source characters (median
4,314.5), with a median of nine function definitions and two loops. The set
contains 292 shaders from the 2022 files and 220 from the 2023 files.

Run the strict corpus check from the repository root:

```sh
python3 shader-ports/shadertoy/test.py \
  --corpus-dir shader-ports/shadertoy/complex \
  --expected-count 512 --strict --jobs 8
```

The test verifies the source hashes, uniqueness and selection metrics,
regenerates all [C++ ports](ports), compiles every shader at all four levels, and
checks the SPIR-V version, Vulkan validation, entry interface and reflection.
Artifacts and the complete [report](../../artifacts/shadertoy_complex/report.json)
are generated in the ignored `shader-ports/artifacts/shadertoy_complex/` directory.
See [RESULTS.md](RESULTS.md) for the recorded failures and strict-run outcome.

To reproduce the initial source screen from the pinned dataset checkout, run
`python3 shader-ports/shadertoy/screen_complex.py --dataset-dir
/path/to/shadertoys-dataset/data/annotated/api`. The script resumes the retained
screening log and freezes the selected manifest once it has 512 passes.

The translator is the bounded GLSL-to-C++ adapter in [port.py](../port.py),
using the explicit project [support.h](../support.h). It does not introduce a
compiler default header or a GLSL layout mode. Compilation and validation do
not prove that each port renders the same pixels as its original; texture
contents, host bindings and numerical image comparison are outside this test.
