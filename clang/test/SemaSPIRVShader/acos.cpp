// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 \
// RUN:   -fshader-host-math -fsyntax-only -verify %s
void types(float f, double d, float (&array)[3]) {
  static_assert(__is_same(decltype(acos(f)), float));
  static_assert(__is_same(decltype(acos(d)), double));
  static_assert(__is_same(decltype(acos(1)), double));
  auto value = acos(array);
  static_assert(__is_same(decltype(value.data), float[3]));
  struct Three {
    float x, y, z;
  } record{};
  static_assert(__is_same(decltype(acos(record)), Three));
  acos();     // expected-error {{too few arguments}}
  acos(f, f); // expected-error {{too many arguments}}
  acos(&f);   // expected-error {{math operands must be floating-point}}
  int integers[3]{};
  acos(integers); // expected-error {{math operands must be floating-point}}
  float matrix[2][2]{};
  acos(matrix); // expected-error {{math operands must be floating-point}}
  constexpr float constant = acos(0.5f);
  static_assert(constant > 1.0f && constant < 1.1f);
}
