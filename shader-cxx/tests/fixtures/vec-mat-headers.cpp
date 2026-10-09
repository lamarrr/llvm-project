#include <types.hpp>

static_assert(sizeof(f32x2) == 2 * sizeof(f32));
static_assert(sizeof(f32x3) == 3 * sizeof(f32));
static_assert(sizeof(f32x4) == 4 * sizeof(f32));
static_assert(alignof(f32x3) == alignof(f32));
static_assert(__builtin_offsetof(f32x3, data) == 0);
static_assert(sizeof(f32x2x3) == 6 * sizeof(f32));
static_assert(sizeof(f32x3x3) == 9 * sizeof(f32));
static_assert(alignof(f32x3x3) == alignof(f32));

struct Params {
  __storage__ float *out;
};

__compute__
__workgroup_size__(1, 1, 1)
void vec_mat_headers(__push_constant__ const Params *p) {
  float input = p->out[0];
  f32x4 value(input, 2.0f, 3.0f, 4.0f);
  value.x() = value.y();
  const f32x4 read_only = value;
  f32x2 pair = read_only.yx();
  f32x3 triple = read_only.zyx();
  f32x4 repeated = read_only.xxyw();
  f32x4 shuffled = read_only.wzyx();
  f32x3 normalized = ::normalize(f32x3(input, 2.0f, 3.0f));
  f32x4 added = value + f32x4(input);

  f32x2x3 lhs(input, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
  f32x3x2 rhs(input, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
  f32x2x2 product = lhs * rhs;
  f32x2 transformed = lhs * f32x3(1.0f, 2.0f, input);
  f32x3 opposite = f32x2(1.0f, input) * lhs;

  p->out[0] = pair[0] + triple.z() + repeated[2] + shuffled[0];
  p->out[1] = normalized.x() + ::dot(added, read_only);
  p->out[2] = product[0][1] + transformed.x() + opposite.z();
}
