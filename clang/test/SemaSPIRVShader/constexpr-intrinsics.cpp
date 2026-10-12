// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 \
// RUN:   -fshader-host-math -fsyntax-only -verify %s
// expected-no-diagnostics

struct Pair {
  float x, y;
};
struct IntegerPair {
  int x, y;
};

constexpr float angle = sin(0.0f);
static_assert(angle == 0.0f);
static_assert(sqrt(4.0f) == 2.0f);
static_assert(clamp(4.0f, 0.0f, 3.0f) == 3.0f);
static_assert(add(1.0f, 2.0f) == 3.0f);
static_assert(add(1, 2) == 3);
static_assert(min(-2, 1) == -2);
static_assert(clamp(5u, 0u, 3u) == 3u);
constexpr IntegerPair integer_sums =
    add(IntegerPair{1, 2}, IntegerPair{3, 4});
static_assert(integer_sums.x == 4 && integer_sums.y == 6);
static_assert(round_even(2.5f) == 2.0f);
static_assert(modf(2.75f).fraction == 0.75f);
static_assert(frexp(8.0f).exponent == 4);

constexpr Pair angles = sin(Pair{0.0f, 0.0f});
static_assert(angles.x == 0.0f && angles.y == 0.0f);
constexpr Pair sums = add(Pair{1.0f, 2.0f}, Pair{3.0f, 4.0f});
static_assert(sums.x == 4.0f && sums.y == 6.0f);
constexpr Pair limits = clamp(Pair{-1.0f, 3.0f}, Pair{0.0f, 0.0f},
                              Pair{2.0f, 2.0f});
static_assert(limits.x == 0.0f && limits.y == 2.0f);

constexpr float inputs[2] = {0.0f, 0.0f};
constexpr auto outputs = sin(inputs);
static_assert(outputs.data[0] == 0.0f && outputs.data[1] == 0.0f);
#ifdef __SPIRV__
static_assert(add_carry(0xffffffffu, 1u).carry == 1u);
static_assert(sub_borrow(0u, 1u).borrow == 1u);
static_assert(mul_extended(0xffffffffu, 2u).high == 1u);
constexpr unsigned bit_values[2] = {7u, 8u};
static_assert(bit_count(bit_values).data[0] == 3);
static_assert(unpack_unorm2x16(0xffff0000u).data[1] == 1.0f);
static_assert(pack_unorm2x16(Pair{0.0f, 1.0f}) == 0xffff0000u);
#endif
