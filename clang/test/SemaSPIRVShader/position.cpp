// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -fsyntax-only -verify %s
__position__ float global[4];
// expected-error@-1 {{shader inputs and outputs must be declared on entry-point parameters}}
void helper() {
  __position__ float local[4];
  // expected-error@-1 {{shader inputs and outputs must be declared on entry-point parameters}}
}
__vertex__
void position_type(__position__ float (*position)[4]) {
  (*position)[0] = 0;
  (*position)[1] = 0;
  (*position)[2] = 0;
  (*position)[3] = 1;
  static_assert(__is_same(decltype((*position)[0]),
                         decltype((*position)[vertex_idx & 3])));
}
