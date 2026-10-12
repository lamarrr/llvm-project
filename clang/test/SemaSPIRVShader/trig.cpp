// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fshader-host-math -fsyntax-only -verify %s
struct R {
  float x, y;
};
void types(float x, float (&a)[2], int (&i)[2], R r) {
  static_assert(__is_same(decltype(sin(x)), float));
  static_assert(__is_same(decltype(sin(r)), R));
  auto sin_array = sin(a);
  static_assert(__is_same(decltype(sin_array.data), float[2]));
  sin(&x);   // expected-error {{math operands must be floating-point}}
  sin(i);    // expected-error {{math operands must be floating-point}}
  sin(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(cos(x)), float));
  static_assert(__is_same(decltype(cos(r)), R));
  auto cos_array = cos(a);
  static_assert(__is_same(decltype(cos_array.data), float[2]));
  cos(&x);   // expected-error {{math operands must be floating-point}}
  cos(i);    // expected-error {{math operands must be floating-point}}
  cos(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(tan(x)), float));
  static_assert(__is_same(decltype(tan(r)), R));
  auto tan_array = tan(a);
  static_assert(__is_same(decltype(tan_array.data), float[2]));
  tan(&x);   // expected-error {{math operands must be floating-point}}
  tan(i);    // expected-error {{math operands must be floating-point}}
  tan(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(asin(x)), float));
  static_assert(__is_same(decltype(asin(r)), R));
  auto asin_array = asin(a);
  static_assert(__is_same(decltype(asin_array.data), float[2]));
  asin(&x);   // expected-error {{math operands must be floating-point}}
  asin(i);    // expected-error {{math operands must be floating-point}}
  asin(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(acos(x)), float));
  static_assert(__is_same(decltype(acos(r)), R));
  auto acos_array = acos(a);
  static_assert(__is_same(decltype(acos_array.data), float[2]));
  acos(&x);   // expected-error {{math operands must be floating-point}}
  acos(i);    // expected-error {{math operands must be floating-point}}
  acos(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(atan(x)), float));
  static_assert(__is_same(decltype(atan(r)), R));
  auto atan_array = atan(a);
  static_assert(__is_same(decltype(atan_array.data), float[2]));
  atan(&x);   // expected-error {{math operands must be floating-point}}
  atan(i);    // expected-error {{math operands must be floating-point}}
  atan(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(sinh(x)), float));
  static_assert(__is_same(decltype(sinh(r)), R));
  auto sinh_array = sinh(a);
  static_assert(__is_same(decltype(sinh_array.data), float[2]));
  sinh(&x);   // expected-error {{math operands must be floating-point}}
  sinh(i);    // expected-error {{math operands must be floating-point}}
  sinh(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(cosh(x)), float));
  static_assert(__is_same(decltype(cosh(r)), R));
  auto cosh_array = cosh(a);
  static_assert(__is_same(decltype(cosh_array.data), float[2]));
  cosh(&x);   // expected-error {{math operands must be floating-point}}
  cosh(i);    // expected-error {{math operands must be floating-point}}
  cosh(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(tanh(x)), float));
  static_assert(__is_same(decltype(tanh(r)), R));
  auto tanh_array = tanh(a);
  static_assert(__is_same(decltype(tanh_array.data), float[2]));
  tanh(&x);   // expected-error {{math operands must be floating-point}}
  tanh(i);    // expected-error {{math operands must be floating-point}}
  tanh(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(asinh(x)), float));
  static_assert(__is_same(decltype(asinh(r)), R));
  auto asinh_array = asinh(a);
  static_assert(__is_same(decltype(asinh_array.data), float[2]));
  asinh(&x);   // expected-error {{math operands must be floating-point}}
  asinh(i);    // expected-error {{math operands must be floating-point}}
  asinh(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(acosh(x)), float));
  static_assert(__is_same(decltype(acosh(r)), R));
  auto acosh_array = acosh(a);
  static_assert(__is_same(decltype(acosh_array.data), float[2]));
  acosh(&x);   // expected-error {{math operands must be floating-point}}
  acosh(i);    // expected-error {{math operands must be floating-point}}
  acosh(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(atanh(x)), float));
  static_assert(__is_same(decltype(atanh(r)), R));
  auto atanh_array = atanh(a);
  static_assert(__is_same(decltype(atanh_array.data), float[2]));
  atanh(&x);   // expected-error {{math operands must be floating-point}}
  atanh(i);    // expected-error {{math operands must be floating-point}}
  atanh(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(radians(x)), float));
  static_assert(__is_same(decltype(radians(r)), R));
  auto radians_array = radians(a);
  static_assert(__is_same(decltype(radians_array.data), float[2]));
  radians(&x);   // expected-error {{math operands must be floating-point}}
  radians(i);    // expected-error {{math operands must be floating-point}}
  radians(x, x); // expected-error {{too many arguments}}
  static_assert(__is_same(decltype(degrees(x)), float));
  static_assert(__is_same(decltype(degrees(r)), R));
  auto degrees_array = degrees(a);
  static_assert(__is_same(decltype(degrees_array.data), float[2]));
  degrees(&x);    // expected-error {{math operands must be floating-point}}
  degrees(i);     // expected-error {{math operands must be floating-point}}
  degrees(x, x);  // expected-error {{too many arguments}}
  atan2(x);       // expected-error {{too few arguments}}
  atan2(x, x, x); // expected-error {{too many arguments}}
  float b[3]{};
  atan2(a, b); // expected-error {{vector operand dimensions must match}}
  static_assert(__is_same(decltype(atan2(r, r)), R));
  constexpr float c = sinh(0.5f);
  static_assert(c > 0.5f && c < 0.6f);
}
