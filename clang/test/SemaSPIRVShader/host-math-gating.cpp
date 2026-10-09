// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fshader-host-math -fsyntax-only -verify %s
void invalid() {
  workgroup_barrier(); // expected-error {{host math mode does not provide shader resources or device operations}}
  specialization_constant(0, 1); // expected-error {{host math mode does not provide shader resources or device operations}}
  make_ray_query(); // expected-error {{host math mode does not provide shader resources or device operations}}
  unsigned x = global_invocation_id.x; // expected-error {{undeclared identifier}}
}
void valid() {
  float a[3] = {1, 2, 3}, b[3] = {4, 5, 6}, out[3];
  auto sum = add(a, b);
  auto normalized = normalize(a);
  float value = dot(a, b);
  float matrix[2][2] = {{1, 0}, {0, 1}}, result[2][2];
  auto inverted = inverse(matrix);
}
