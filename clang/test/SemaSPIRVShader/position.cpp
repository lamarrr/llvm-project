// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -I %S/../../lib/Headers -fsyntax-only -verify %s
#include <spirv/types.hpp>
using namespace spirv;
__position float global[4];
// expected-error@-1 {{shader inputs and outputs must be declared on entry-point parameters}}
void helper() {
  __position float local[4];
  // expected-error@-1 {{shader inputs and outputs must be declared on entry-point parameters}}
}
__vertex
void position_type(__position float (*position)[4]) {
  (*position)[0] = 0;
  (*position)[1] = 0;
  (*position)[2] = 0;
  (*position)[3] = 1;
  static_assert(__is_same(decltype((*position)[0]),
                         decltype((*position)[vertex_idx & 3])));
}
__vertex
void vector_position_type(__position f32x4 *position) {
  *position = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
}
