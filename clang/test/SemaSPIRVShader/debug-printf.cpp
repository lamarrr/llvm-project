// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -I %S/../../lib/Headers -fsyntax-only -verify %s

#include <spirv/types.hpp>

using namespace spirv;

void valid(float value) {
  printf("constant text");
  printf("value %f", value);
  printf(u8"UTF-8 format");
  printf("vector %v2f", f32x2(value, 2.0f));
  printf("wide %f", double(value));
  static_assert(__is_same(decltype(printf("text")), void));
}

void invalid(const char *format, float value) {
  float too_wide[5] = {};
  printf();               // expected-error {{requires a format string literal}}
  printf(format, value);  // expected-error {{requires a narrow string literal}}
  printf(L"wide", value); // expected-error {{requires a narrow string literal}}
  printf("line\nbreak",
         value); // expected-error@-1 {{cannot contain control characters}}
  printf(
      "bool %d",
      true); // expected-error@+1 {{must be 32-bit or 64-bit numeric scalars or vectors}}
  printf(
      "short %d",
      short(
          1)); // expected-error@+1 {{must be 32-bit or 64-bit numeric scalars or vectors}}
  printf(
      "pointer %p",
      &value); // expected-error@+1 {{must be 32-bit or 64-bit numeric scalars or vectors}}
  printf(
      "matrix",
      f32x2x2{}); // expected-error@+1 {{must be 32-bit or 64-bit numeric scalars or vectors}}
  printf(
      "wide vector",
      too_wide); // expected-error@-15 {{must be 32-bit or 64-bit numeric scalars or vectors}}
  debug_printf(
      "old spelling"); // expected-error@-1 {{undeclared identifier 'debug_printf'}}
}
