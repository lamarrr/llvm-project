// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

// expected-error@+1 {{entry points must be non-variadic free functions returning void}}
__compute
int wrong_return();
// expected-error@+1 {{entry points must be non-variadic free functions returning void}}
__compute
void variadic(int, ...);
// expected-error@+1 {{workgroup dimensions must be positive}}
__workgroup_size(0, 1, 1)
void zero_size();
// expected-error@+1 {{requires exactly 2 arguments}}
__binding(0) int missing_binding;
// expected-error@+1 {{non-negative integral}}
__binding(0, -1) int negative_binding;
void immutable(__uniform int *u, __push_constant int *p) {
  // expected-error@+1 {{read-only}}
  *u = 1;
  // expected-error@+1 {{read-only}}
  *p = 1;
  // expected-error@+1 {{const-qualified}}
  global_invocation_id[0] = 0;
  // expected-error@+1 {{const-qualified}}
  vertex_idx = 0;
  // expected-error@+2 {{constant expression}}
  // expected-note@+1 {{read of non-constexpr}}
  constexpr unsigned runtime = global_invocation_id[0];
}
int integer_format = rgba32f;

// expected-note@* {{variable 'global_invocation_id' declared const here}}
// expected-note@* {{variable 'vertex_idx' declared const here}}
// expected-note@* {{declared here}}
