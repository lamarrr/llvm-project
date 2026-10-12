// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 \
// RUN:   -fshader-host-math -I %S/../../lib/Headers -fsyntax-only -verify %s
// expected-no-diagnostics

#include <spirv/types.hpp>
using namespace spirv;

constexpr f32x2 first(1.0f, 2.0f);
constexpr f32x2 second(3.0f, 4.0f);
static_assert(sin(first - first).x() == 0.0f);
static_assert((first + second).x() == 4.0f);
static_assert((second - first).y() == 2.0f);
static_assert((first * second).x() == 3.0f);
static_assert((second / first).y() == 2.0f);
static_assert(mix(first, second, 0.5f).x() == 2.0f);
static_assert(lerp(first, second, f32x2(0.0f, 1.0f)).y() == 4.0f);
static_assert(min(first, second).y() == 2.0f);
static_assert(max(first, second).x() == 3.0f);
static_assert(clamp(second, 0.0f, 3.0f).y() == 3.0f);
static_assert(step(2.0f, first).x() == 0.0f);
static_assert(smoothstep(0.0f, 4.0f, second).y() == 1.0f);
static_assert(dot(first, second) == 11.0f);
static_assert(cross(f32x3(1.0f, 0.0f, 0.0f),
                    f32x3(0.0f, 1.0f, 0.0f)).z() == 1.0f);
static_assert(reflect(f32x2(1.0f, -1.0f), f32x2(0.0f, 1.0f)).y() == 1.0f);
static_assert(faceforward(f32x2(0.0f, 1.0f), f32x2(0.0f, -1.0f),
                          f32x2(0.0f, 1.0f)).y() == 1.0f);
static_assert(all(less_than(first, second)));
static_assert(any(greater_than(second, first)));
static_assert(logical_not(b8x2(false)).x());
static_assert((b8x2(false, true) || b8x2(true, false)).x());
static_assert((b8x2(true, false) && true).x());
static_assert((false || b8x2(false, true)).y());
static_assert((true && b8x2(true, false)).x());
static_assert((!b8x2(true, false)).y());
static_assert((b8x2(true, false) | b8x2(false, true)).y());
static_assert((b8x2(true, false) & b8x2(false, true)).x() == false);
static_assert((b8x2(true, false) ^ true).y());
constexpr bool boolean_operators() {
  b8x2 value(true, false);
  value |= b8x2(false, true);
  value &= true;
  value ^= b8x2(true, false);
  return !value.x() && value.y();
}
static_assert(boolean_operators());
static_assert(float_bits_to_uint(1.0f) == 0x3f800000u);
static_assert(uint_bits_to_float(0x3f800000u) == 1.0f);
constexpr u32x2 bits(0x12u, 0x30u);
static_assert((bits << 1).x() == 0x24u);
static_assert((bits >> u32x2(1u, 2u)).y() == 0xcu);
static_assert((1u << u32x2(1u, 3u)).y() == 8u);
static_assert((bits & 0x10u).x() == 0x10u);
static_assert((bits | u32x2(1u)).x() == 0x13u);
static_assert((bits ^ 0x10u).x() == 0x2u);
static_assert((~u32x2(0u)).x() == 0xffffffffu);
static_assert((u32x2(11u) % u32x2(5u)).x() == 1u);
static_assert((i32x2(-8) >> 2).x() == -2);
static_assert((u32x3(1u, 2u, 3u) << u32x3(1u, 2u, 3u)).z() == 24u);
static_assert((u32x4(1u, 2u, 3u, 4u) << 2).w() == 16u);
static_assert((u64x2(1ull, 2ull) << 32).y() == 0x200000000ull);
static_assert((u8x2(3u) << 2).x() == 12u);
static_assert((~u8x2(0u)).x() == 0xffu);
constexpr bool integer_operators() {
  u32x2 value(1u, 2u);
  value <<= 1u;
  value |= u32x2(4u);
  value ^= 1u;
  value &= 7u;
  value >>= 1u;
  value %= 3u;
  return value.x() == 0u && value.y() == 2u;
}
static_assert(integer_operators());
static_assert(inverse_sqrt(f32x2(4.0f, 1.0f)).x() == 0.5f);
static_assert(mod(f32x2(5.0f), 2.0f).x() == 1.0f);
static_assert(round_even(f32x2(2.5f, 3.5f)).x() == 2.0f);
static_assert(!is_nan(f32x2(1.0f, 2.0f)).x());
static_assert(!is_inf(1.0f));
static_assert(pow(f32x2(2.0f), 3.0f).x() == 8.0f);
static_assert(atan(f32x2(0.0f), f32x2(1.0f)).x() == 0.0f);
constexpr bool decomposition_math() {
  float whole = 0.0f;
  float fraction = modf(2.75f, whole);
  int exponent = 0;
  float significand = frexp(8.0f, exponent);
  f32x2 vector_whole;
  f32x2 vector_fraction = modf(f32x2(1.25f, 2.5f), vector_whole);
  i32x2 vector_exponent;
  f32x2 vector_significand =
      frexp(f32x2(8.0f, 4.0f), vector_exponent);
  return fraction == 0.75f && whole == 2.0f &&
         significand == 0.5f && exponent == 4 &&
         vector_fraction.x() == 0.25f && vector_whole.y() == 2.0f &&
         vector_significand.x() == 0.5f && vector_exponent.y() == 3;
}
static_assert(decomposition_math());

#ifdef __SPIRV__
static_assert(bit_count(u32x2(7u, 8u)).x() == 3);
static_assert(find_lsb(u32x2(8u, 0u)).x() == 3);
static_assert(find_msb(i32x2(-2, 8)).x() == 0);
static_assert(bitfield_reverse(1u) == 0x80000000u);
static_assert(bitfield_extract(0xff00u, 8, 8) == 0xffu);
static_assert(bitfield_extract(-128, 4, 4) == -8);
static_assert(bitfield_insert(0u, 0xffu, 8, 8) == 0xff00u);
static_assert(find_msb(-1) == -1);
constexpr bool extended_math() {
  u32 carry = 0;
  u32 sum = uadd_carry(0xffffffffu, 1u, carry);
  u32 borrow = 0;
  u32 difference = usub_borrow(0u, 1u, borrow);
  u32 low = 0, high = 0;
  umul_extended(0xffffffffu, 2u, low, high);
  i32 signed_low = 0, signed_high = 0;
  imul_extended(-2, 3, signed_low, signed_high);
  u32x2 vector_carry;
  u32x2 vector_sum = uadd_carry(u32x2(0xffffffffu, 1u),
                                u32x2(1u, 2u), vector_carry);
  return sum == 0 && carry == 1 && difference == 0xffffffffu &&
         borrow == 1 && low == 0xfffffffeu && high == 1 &&
         signed_low == -6 && signed_high == -1 &&
         vector_sum.x() == 0 && vector_carry.x() == 1 &&
         vector_sum.y() == 3;
}
static_assert(extended_math());
#endif

#ifdef __SPIRV__
static_assert(unpack_unorm2x16(0x8000ffffu).x() == 1.0f);
static_assert(unpack_unorm2x16(0x8000ffffu).y() > 0.5f);
static_assert(unpack_snorm4x8(0x7f008081u).w() == 1.0f);
static_assert(unpack_half2x16(0x3c000000u).y() == 1.0f);
static_assert(pack_unorm2x16(f32x2(0.0f, 1.0f)) == 0xffff0000u);
static_assert(pack_unorm4x8(f32x4(0.5f)) == 0x80808080u);
static_assert(pack_snorm4x8(f32x4(-1.0f, 0.0f, 0.0f, 1.0f)) ==
              0x7f000081u);
static_assert(pack_half2x16(f32x2(0.0f, 1.0f)) == 0x3c000000u);
static_assert(unpack_double2x32(
                  pack_double2x32(u32x2(0u, 0x3ff00000u)))
                  .y() == 0x3ff00000u);
#endif

constexpr f32x2x2 diagonal(2.0f);
static_assert((diagonal * first).x() == 2.0f);
static_assert((first * diagonal).y() == 4.0f);
static_assert((diagonal * diagonal)[1][1] == 4.0f);
static_assert(matrix_comp_mult(diagonal, diagonal)[0][0] == 4.0f);
static_assert(outer_product(first, second)[1][0] == 6.0f);
static_assert(transpose(outer_product(first, second))[0][1] == 6.0f);
static_assert(inverse(diagonal)[0][0] == 0.5f);
