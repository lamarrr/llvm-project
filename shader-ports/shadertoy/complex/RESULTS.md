# Complex Shadertoy corpus results

The four-level run passed **2,021 of 2,048 builds**: 512 each at `-O0` and
`-O2`, 487 at `-O1`, and 510 at `-O3`. Thus 486 shaders pass all four levels.
The remaining 25 `-O1` builds fail: 24 require mixed-width access to a
byte-addressed local aggregate that logical SPIR-V cannot currently lower,
and one needs a partial logical-object zero fill. Two `-O3` builds exceed the
45-second compile limit. One of those still had not finished after 180 seconds
when compiled alone. The strict command exits nonzero until these gaps close.
Successful builds emit SPIR-V 1.4 and validate for Vulkan 1.3. The generated
[report](../../artifacts/shadertoy_complex/report.json) records every command,
source hash, reflected interface, image-operation count and failure diagnostic.
The [manifest](upstream/manifest.json) and [README](README.md) specify the
source selection and limits of this result.

The broader screen retained 1,011 candidates. Its [log](screening.jsonl) records
535 passes at both levels, 421 `-O2` compile failures, 26 `-O2` timeouts,
16 translation failures, and 13 shaders that passed `-O2` but failed or timed
out at `-O0`. Fourteen passing shaders had proprietary or unknown license
metadata and were excluded. Nine other eligible passes were outside the final
512 after sorting successful candidates by source length.

The selected corpus contains distinct shaders with at least three functions
and a loop. Original source length ranges from 3,049 to 20,482 characters;
median source length is 4,314.5 characters. These checks establish compilation,
validation and structural code generation. They do not establish GLSL/C++
pixel equivalence or shader runtime performance.
