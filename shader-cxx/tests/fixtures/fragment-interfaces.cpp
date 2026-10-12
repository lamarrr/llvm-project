#include <spirv/types.hpp>
using namespace spirv;

__fragment__
void fragment_interfaces(__clip_distance__ const float (*clip)[2],
                         __cull_distance__ const float (*cull)[2],
                         __input__(0) const f32x2 *uv,
                         __frag_depth__ float *depth,
                         __sample_mask__ unsigned (*mask)[1],
                         __output__(0) float *color) {
  f32x2 centroid = interpolate_at_centroid(uv);
  f32x2 sample = interpolate_at_sample(uv, 0u);
  f32x2 offset = interpolate_at_offset(uv, f32x2(0.25f, 0.0f));
  *depth = 0.5f;
  (*mask)[0] = 1u;
  *color = centroid.x() + sample.y() + offset.x() + uv->x() + (*clip)[0] +
           (*cull)[0] + fwidth_fine(centroid.x()) + fwidth_coarse(sample.y());
}
