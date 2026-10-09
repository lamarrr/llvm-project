// Port of upstream/glsl_shader.frag; see upstream/LICENSE.txt.
#include "types.h"

__fragment__
void imgui_fragment(__input__(0) Varyings varying,
                    __binding__(0, 0) texture_2d<Color> texture,
                    __binding__(1, 0) sampler sampling,
                    __output__(0) Color *output) {
  *output =
      mul(varying.color, texture.sample(sampling, varying.uv.x, varying.uv.y));
}
