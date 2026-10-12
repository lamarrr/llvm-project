// Faithful port attempt of roughness_limiter.glsl; currently blocked by missing
// acos. See upstream/LICENSE.txt and upstream/manifest.json. No approximation is
// substituted.
#include "types.h"

struct RoughnessParams {
  int screen_size[2];
  float curve;
  unsigned pad;
};

__compute
__workgroup_size(8, 8, 1)
void roughness_limiter(
    __binding(0, 0) texture_2d<Color> source,
    __binding(1, 0) storage_texture_2d<float, r8> destination,
    __push_constant const RoughnessParams *params) {
  int x = static_cast<int>(global_invocation_id.x);
  int y = static_cast<int>(global_invocation_id.y);
  if (x > params->screen_size[0] || y > params->screen_size[1])
    return;
  Float3 accumulated{0.0f, 0.0f, 0.0f};
  float count = 0.0f;
  for (int i = 0; i <= 1; ++i) {
    for (int j = 0; j <= 1; ++j) {
      Color texel = source.load(x + i, y + j);
      Float3 normal{texel.r * 2.0f - 1.0f, texel.g * 2.0f - 1.0f,
                    texel.b * 2.0f - 1.0f};
      accumulated = add(accumulated, normalize(normal));
      count += 1.0f;
    }
  }
  Float3 divisor{count, count, count};
  accumulated = div(accumulated, divisor);
  float r = length(accumulated);
  float limit = 0.0f;
  if (r < 1.0f)
    limit =
        min(sqrt(pow(acos(max(0.0f, r)) / 1.5707963267948966f, params->curve)),
            0.4f);
  destination.store(x, y, limit);
}
