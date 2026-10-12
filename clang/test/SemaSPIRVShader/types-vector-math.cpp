// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 \
// RUN:   -fshader-host-math -I %S/../../lib/Headers -fsyntax-only -verify %s
// expected-no-diagnostics
#include <spirv/types.hpp>
using namespace spirv;

static_assert(__is_same(spirv::f32x2, spirv::vec<float, 2>));
static_assert(__is_same(spirv::f32x3x3, spirv::mat<3, 3>));
static_assert(__is_same(spirv::b8, bool));
static_assert(__is_same(spirv::b8x2, spirv::vec<bool, 2>));
static_assert(__is_same(spirv::b8x3, spirv::vec<bool, 3>));
static_assert(__is_same(spirv::b8x4, spirv::vec<bool, 4>));

f32x2 math(f32x2 first, f32x2 second, float weight) {
  static_assert(__is_same(decltype(mix(first, second, weight)), f32x2));
  static_assert(__is_same(decltype(mix(first, second, first)), f32x2));
  static_assert(__is_same(decltype(mix(first, second, b8x2(true))), f32x2));
  static_assert(__is_same(decltype(lerp(first, second, weight)), f32x2));
  static_assert(__is_same(decltype(inverse_sqrt(first)), f32x2));
  static_assert(__is_same(decltype(mod(first, weight)), f32x2));
  static_assert(__is_same(decltype(mod(weight, first)), f32x2));
  static_assert(__is_same(decltype(step(first, weight)), f32x2));
  static_assert(__is_same(decltype(step(weight, first)), f32x2));
  static_assert(__is_same(decltype(smoothstep(0.0f, 1.0f, first)), f32x2));
  static_assert(__is_same(decltype(min(first, weight)), f32x2));
  static_assert(__is_same(decltype(max(weight, first)), f32x2));
  static_assert(__is_same(decltype(pow(first, weight)), f32x2));
  static_assert(__is_same(decltype(clamp(first, 0.0f, second)), f32x2));
  static_assert(__is_same(decltype(sin(first)), f32x2));
  static_assert(__is_same(decltype(fract(first)), f32x2));
  static_assert(__is_same(decltype(atan(first, second)), f32x2));
  static_assert(__is_same(decltype(length(weight)), float));
  static_assert(__is_same(decltype(distance(weight, 1.0f)), float));
  static_assert(__is_same(decltype(less_than(first, second)), b8x2));
  static_assert(__is_same(decltype(float_bits_to_uint(first)), u32x2));
  static_assert(__is_same(decltype(uint_bits_to_float(u32x2(0))), f32x2));
  static_assert(__is_same(decltype(is_nan(first)), b8x2));
  static_assert(__is_same(decltype(is_inf(first)), b8x2));
  static_assert(__is_same(decltype(round_even(first)), f32x2));
  f32x2 result = mix(first, second, weight);
  result += mix(first, second, first);
  result += mix(first, second, b8x2(true));
  result += lerp(first, second, weight);
  result += inverse_sqrt(first);
  result += mod(first, weight) + mod(weight, first);
  result += step(first, weight) + step(weight, first);
  result += smoothstep(0.0f, 1.0f, first);
  result += min(first, weight) + max(weight, first);
  result += pow(first, weight) + clamp(first, 0.0f, second);
  result += atan(first, second);
  auto mask = less_than(first, second);
  (void)any(mask);
  (void)all(mask);
  (void)less_than_equal(first, second);
  (void)greater_than(first, second);
  (void)greater_than_equal(first, second);
  (void)equal(first, second);
  (void)not_equal(first, second);
  (void)logical_not(b8x2(true));
  (void)float_bits_to_uint(first);
  (void)float_bits_to_int(first);
  (void)uint_bits_to_float(u32x2(0));
  (void)int_bits_to_float(i32x2(0));
  (void)is_nan(first);
  (void)is_inf(first);
  (void)round_even(first);
  f32x2 whole;
  i32x2 exponent;
  (void)modf(first, whole);
  (void)frexp(first, exponent);
  (void)ldexp(first, exponent);
  return result;
}

f32x2x2 matrices(f32x2x2 first, f32x2x2 second) {
  static_assert(__is_same(decltype(matrix_comp_mult(first, second)), f32x2x2));
  static_assert(
      __is_same(decltype(outer_product(f32x2(1.0f), f32x3(1.0f))), f32x2x3));
  static_assert(__is_same(decltype(transpose(f32x2x3(0.0f))), f32x3x2));
  static_assert(__is_same(decltype(inverse(first)), f32x2x2));
  (void)matrix_comp_mult(first, second);
  (void)outer_product(f32x2(1.0f), f32x3(1.0f));
  (void)transpose(f32x2x3(0.0f));
  return inverse(first);
}

#ifdef __SPIRV__
f32x2 derivatives(f32x2 value) {
  return ddx(value) + ddy(value) + fwidth(value) + ddx_fine(value) +
         ddy_fine(value) + ddx_coarse(value) + ddy_coarse(value) +
         fwidth_fine(value) + fwidth_coarse(value);
}

void integer_and_packing(u32x2 value, f32x2 coordinates) {
  static_assert(__is_same(decltype(bit_count(value)), i32x2));
  static_assert(__is_same(decltype(find_lsb(value)), i32x2));
  (void)bitfield_reverse(value);
  (void)bit_count(value);
  (void)find_lsb(value);
  (void)find_msb(value);
  (void)bitfield_extract(value, 0, 8);
  (void)bitfield_insert(value, value, 0, 8);
  (void)pack_snorm4x8(f32x4(0.0f));
  (void)pack_unorm4x8(f32x4(0.0f));
  (void)pack_snorm2x16(coordinates);
  (void)pack_unorm2x16(coordinates);
  (void)pack_half2x16(coordinates);
  (void)pack_double2x32(value);
  (void)unpack_snorm4x8(0u);
  (void)unpack_unorm4x8(0u);
  (void)unpack_snorm2x16(0u);
  (void)unpack_unorm2x16(0u);
  (void)unpack_half2x16(0u);
  (void)unpack_double2x32(0.0);
  u32x2 carry;
  u32x2 borrow;
  u32x2 low;
  u32x2 high;
  (void)uadd_carry(value, value, carry);
  (void)usub_borrow(value, value, borrow);
  umul_extended(value, value, low, high);
  i32x2 signed_value(1);
  i32x2 signed_low;
  i32x2 signed_high;
  imul_extended(signed_value, signed_value, signed_low, signed_high);
}
#endif
