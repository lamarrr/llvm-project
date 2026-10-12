// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -fsyntax-only -verify %s

#include <spirv/types.hpp>
using namespace spirv;

void invalid(i8x3 short_vector, f32x4 float_vector, float dynamic_value) {
  // expected-error@+1 {{four-component 8-bit integer vectors}}
  integer_dot(short_vector, short_vector);
  // expected-error@+1 {{four-component 8-bit integer vectors}}
  integer_dot(float_vector, float_vector);
  // expected-error@+1 {{composite elements must have one type}}
  specialization_composite2(1.0f, 2);
  // expected-error@+1 {{composite elements must be constants}}
  specialization_composite2(dynamic_value, 2.0f);
}
