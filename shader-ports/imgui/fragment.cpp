// Port of upstream/glsl_shader.frag; see upstream/LICENSE.txt.
#include "types.h"

__fragment
void imgui_fragment(__input(0) Varyings varying,
                    __binding(0, 0) texture_2d<Color> texture,
                    __binding(1, 0) sampler sampling,
                    __output(0) Color *output) {
  *output =
      mul(varying.color, texture.sample(sampling, varying.uv.x, varying.uv.y));
}
