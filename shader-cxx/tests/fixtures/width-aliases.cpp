#include <types.hpp>

struct Params {
  __storage__ f32 *out;
};

__compute__
__workgroup_size__(1, 1, 1)
void width_aliases(__push_constant__ const Params *p) {
  f32 input = p->out[0];

  u8x2 unsigned_8(u8(input), u8(1));
  u16x2 unsigned_16(u16(input), u16(1));
  u32x2 unsigned_32(u32(input), u32(1));
  u64x2 unsigned_64(u64(input), u64(1));
  i8x2 signed_8(i8(input), i8(1));
  i16x2 signed_16(i16(input), i16(1));
  i32x2 signed_32(i32(input), i32(1));
  i64x2 signed_64(i64(input), i64(1));
  f16x2 half_precision(f16(input), f16(1));
  f32x2 single_precision(input, 1.0f);
  f64x2 double_precision(f64(input), f64(1));

  p->out[0] = f32((unsigned_8 + unsigned_8).x());
  p->out[1] = f32((unsigned_16 + unsigned_16).x());
  p->out[2] = f32((unsigned_32 + unsigned_32).x());
  p->out[3] = f32((unsigned_64 + unsigned_64).x());
  p->out[4] = f32((signed_8 + signed_8).x());
  p->out[5] = f32((signed_16 + signed_16).x());
  p->out[6] = f32((signed_32 + signed_32).x());
  p->out[7] = f32((signed_64 + signed_64).x());
  p->out[8] = f32((half_precision + half_precision).x());
  p->out[9] = (single_precision + single_precision).x();
  p->out[10] = f32((double_precision + double_precision).x());
}
