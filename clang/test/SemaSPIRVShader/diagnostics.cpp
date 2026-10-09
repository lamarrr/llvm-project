// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

__compute__
int wrong_return(); // expected-error@-1 {{entry points must be non-variadic free functions returning void}}
__compute__
void variadic(int, ...); // expected-error@-1 {{entry points must be non-variadic free functions returning void}}
__workgroup_size__(0, 1, 1)
void zero_size();                                              // expected-error@-1 {{workgroup dimensions must be positive}}
__binding__(0) int missing_binding;                            // expected-error {{requires exactly 2 arguments}}
__binding__(0, -1) int negative_binding;                       // expected-error {{non-negative integral}}
void alignment(__storage__ __align__(3) float *bad_alignment); // expected-error {{__align__ requires a positive power of two}}
void immutable(__uniform__ int *u, __push_constant__ int *p) {
  *u = 1;                                               // expected-error {{read-only}}
  *p = 1;                                               // expected-error {{read-only}}
  global_invocation_id[0] = 0;                          // expected-error {{const-qualified}}
  vertex_idx = 0;                                       // expected-error {{const-qualified}}
  constexpr unsigned runtime = global_invocation_id[0]; // expected-error {{constant expression}} expected-note {{read of non-constexpr}}
}
int integer_format = rgba32f;

// expected-note@* {{variable 'global_invocation_id' declared const here}}
// expected-note@* {{variable 'vertex_idx' declared const here}}
// expected-note@* {{declared here}}
