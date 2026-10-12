// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

void invalid_arithmetic() {
  (void)add_carry(
      1, 2); // expected-error@-1 {{carry and borrow require unsigned elements}}
  (void)sub_borrow(
      1u, 2ull); // expected-error@-1 {{matching 32-bit or 64-bit integer}}
  (void)mul_extended(
      1.0f, 2.0f); // expected-error@-1 {{extended integer arithmetic requires}}
  (void)is_nan(1u); // expected-error {{floating classification requires}}
}

void invalid_subgroups(unsigned size) {
  (void)subgroup_clustered_add(
      1u, 3u); // expected-error@-1 {{positive constant power of two}}
  (void)subgroup_clustered_add(
      1u, size); // expected-error@-1 {{positive constant power of two}}
  (void)subgroup_logical_and(
      1u); // expected-error@-1 {{logical subgroup operations require bool}}
  (void)subgroup_inclusive_and(
      1.0f); // expected-error@-1 {{requires integer elements}}
}

void invalid_offsets(texture_2d<float> image, texture_cube<float> cube,
                     sampler filter, int offset) {
  (void)image.sample_offset(
      filter, 0.5f, 0.5f, offset,
      0); // expected-error {{dynamic image offsets require a gather operation}}
  (void)cube.sample_offset(filter, 0.5f, 0.5f, 0.5f, 1, 0,
                           0); // expected-error {{non-cube, non-buffer image}}
}

void invalid_atomic_policy(__storage unsigned *value, memory_order order) {
  (void)atomic_load(
      value, memory_order::seq_cst,
      memory_scope::
          device); // expected-error@-3 {{sequentially consistent atomics are unavailable}}
  (void)atomic_load(
      value, memory_order::release,
      memory_scope::device); // expected-error@+1 {{memory order is invalid}}
  atomic_store(
      value, 1u, memory_order::acquire,
      memory_scope::device); // expected-error@-5 {{memory order is invalid}}
  (void)atomic_compare_exchange(
      value, 0u, 1u,
      memory_order::
          relaxed, // expected-error@-3 {{compare-exchange failure order}}
      memory_order::acquire, memory_scope::device);
  (void)atomic_add(
      value, 1u, order,
      memory_scope::
          device); // expected-error@+1 {{atomic order and scope must be constant}}
  (void)atomic_add(
      value, 1u, 2u,
      memory_scope::
          device); // expected-error@-7 {{atomic order and scope must be constant}}
  static __shared unsigned scratch = 0;
  (void)atomic_add(
      &scratch, 1u,
      memory_order::
          acquire_release, // expected-error@-3 {{atomic memory scope is invalid}}
      memory_scope::device);
}
