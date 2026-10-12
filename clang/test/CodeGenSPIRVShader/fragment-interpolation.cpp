// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=distance_vertex -o %t.vert.spv %t.bc
// RUN: clang-spirv-link --entry=interpolate_fragment -o %t.frag.spv %t.bc

#include <spirv/types.hpp>
using namespace spirv;

__vertex
void distance_vertex(__position float (*position)[4],
                     __point_size float *point_size,
                     __clip_distance float (*clip)[2],
                     __cull_distance float (*cull)[2]) {
  (*position)[0] = 0.0f;
  (*position)[1] = 0.0f;
  (*position)[2] = 0.0f;
  (*position)[3] = 1.0f;
  *point_size = 1.0f;
  (*clip)[0] = 1.0f;
  (*clip)[1] = 1.0f;
  (*cull)[0] = 1.0f;
  (*cull)[1] = 1.0f;
}

__fragment
void interpolate_fragment(__clip_distance const float (*clip)[2],
                          __cull_distance const float (*cull)[2],
                          __input(0) const f32x2 *uv,
                          __output(0) float *color) {
  f32x2 centroid = interpolate_at_centroid(uv);
  f32x2 sample = interpolate_at_sample(uv, 0u);
  f32x2 offset = interpolate_at_offset(uv, f32x2(0.25f, 0.0f));
  *color = centroid.x() + sample.y() + offset.x() + uv->x() + (*clip)[0] +
           (*cull)[0];
}
