// Port of upstream/glsl_shader.vert; see upstream/LICENSE.txt.
#include "types.h"

__vertex
void imgui_vertex(__input(0) Float2 pos, __input(1) Float2 uv,
                  __input(2) Color color,
                  __push_constant const Params *params,
                  __output(0) Varyings *varying,
                  __position float (*position)[4]) {
  *varying = Varyings{color, uv};
  (*position)[0] = pos.x * params->scale[0] + params->translate[0];
  (*position)[1] = pos.y * params->scale[1] + params->translate[1];
  (*position)[2] = 0.0f;
  (*position)[3] = 1.0f;
}
