// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -fsyntax-only -verify %s
// expected-no-diagnostics
struct Color {
  float r, g, b, a;
};
struct Nested {
  Color color;
  float weights[2];
};

__fragment__
void record_assignment(__input__(0) Color color, __output__(0) Color *output,
                       __output__(1) Nested *nested) {
  *output = color;
  *output = Color{1, 2, 3, 4};
  (*output = color).r = 2.0f;
  output->operator=(color).g = 3.0f;
  static_assert(__is_same(decltype(*output = color), decltype(*output)));
  static_assert(
      __is_same(decltype(output->operator=(color)), decltype(*output)));
  *nested = Nested{color, {5, 6}};
}

void storage_assignment(__storage__ Color *pointer, Color value) {
  *pointer = value;
  static_assert(__is_same(decltype(*pointer = value), decltype(*pointer)));
}
