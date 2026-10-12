// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -fsyntax-only -verify %s

#include <spirv/types.hpp>
using namespace spirv;

__fragment__
void interpolation(__input__(0) const f32x2 *uv, __output__(0) float *color) {
  f32x2 centroid = interpolate_at_centroid(uv);
  f32x2 sample = interpolate_at_sample(uv, 0u);
  f32x2 offset = interpolate_at_offset(uv, f32x2(0.25f, 0.0f));
  *color = centroid.x() + sample.y() + offset.x();
  // expected-error@+1 {{interpolation requires a fragment input pointer}}
  (void)interpolate_at_centroid(1.0f);
  // expected-error@+1 {{interpolation offset requires two floats}}
  (void)interpolate_at_offset(uv, 1.0f);
}
