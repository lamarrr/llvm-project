// Port of luminance_reduce.glsl. See upstream/LICENSE.txt and
// upstream/manifest.json.
#include "types.h"

struct LuminanceParams {
  int source_size[2];
  float max_luminance, min_luminance, exposure_adjust;
  float pad[3];
};
static_assert(sizeof(LuminanceParams) == 32);
static_assert(__builtin_offsetof(LuminanceParams, exposure_adjust) == 16);

// Fixed 8x8 tiles always have six reduction levels. Explicit template expansion
// also exercises template helpers and optimized shared-array addressing.
// Both this form and the original literal loop are regression-tested.
template <unsigned Stride>
void reduction_step(__shared float *scratch, unsigned t) {
  if (t < Stride)
    scratch[t] += scratch[t + Stride];
  workgroup_barrier();
}

__compute
__workgroup_size(8, 8, 1)
void luminance_reduce(
#ifdef READ_TEXTURE
    __binding(0, 0) texture_2d<Color> source,
#else
    __binding(0, 0) storage_texture_2d<float, r32f> source,
#endif
    __binding(1, 0) storage_texture_2d<float, r32f> destination,
#ifdef WRITE_LUMINANCE
    __binding(2, 0) texture_2d<float> previous,
#endif
    __push_constant const LuminanceParams *params) {
  static __shared float scratch[64];
  unsigned t = local_invocation_id.y * 8 + local_invocation_id.x;
  int x = static_cast<int>(global_invocation_id.x);
  int y = static_cast<int>(global_invocation_id.y);
  // Preserve upstream's any(less_than(...)), including its edge-tile caveat.
  if (x < params->source_size[0] || y < params->source_size[1]) {
#ifdef READ_TEXTURE
    Color color = source.load(x, y);
    scratch[t] = max(color.r, max(color.g, color.b));
#else
    scratch[t] = source.load(x, y);
#endif
  } else {
    scratch[t] = 0.0f;
  }
  workgroup_barrier();
#ifdef UNROLL_REDUCTION
  reduction_step<32>(scratch, t);
  reduction_step<16>(scratch, t);
  reduction_step<8>(scratch, t);
  reduction_step<4>(scratch, t);
  reduction_step<2>(scratch, t);
  reduction_step<1>(scratch, t);
#else
  unsigned size = 32;
  do {
    if (t < size)
      scratch[t] += scratch[t + size];
    workgroup_barrier();
    size >>= 1;
  } while (size >= 1);
#endif
  if (t == 0) {
    int width = min(params->source_size[0] - x, 8);
    int height = min(params->source_size[1] - y, 8);
    float average = scratch[0] / static_cast<float>(width * height);
#ifdef WRITE_LUMINANCE
    float previous_luminance = previous.load(0, 0);
    average = clamp(previous_luminance + (average - previous_luminance) *
                                             params->exposure_adjust,
                    params->min_luminance, params->max_luminance);
#endif
    destination.store(x / 8, y / 8, average);
  }
}
