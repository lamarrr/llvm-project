// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -fsyntax-only -verify %s

void invalid(float *plain, __shared float *shared,
             const __shared float *readonly, unsigned dynamic_dimension) {
  // expected-error@+1 {{operands require shared float pointers}}
  cooperative_matrix_multiply(plain, shared, shared, 16, 16, 16);
  // expected-error@+1 {{operands require shared float pointers}}
  cooperative_matrix_multiply(shared, shared, readonly, 16, 16, 16);
  // expected-error@+1 {{dimensions must be constant values from 1 through 256}}
  cooperative_matrix_multiply(shared, shared, shared, dynamic_dimension, 16,
                              16);
  // expected-error@+1 {{dimensions must be constant values from 1 through 256}}
  cooperative_matrix_multiply(shared, shared, shared, 16, 0, 16);
  // expected-error@+1 {{dimensions must be constant values from 1 through 256}}
  cooperative_matrix_multiply(shared, shared, shared, 16, 16, 257);
}
