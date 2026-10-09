// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
void invalid(int dynamic, __storage__ const unsigned *readonly) {
  float a[3], b[2], out[3];
  add(a, b); // expected-error {{vector operand dimensions must match}}
  dot(a); // expected-error {{too few arguments}}
  int integers[3];
  dot(integers, integers); // expected-error {{math operands must be floating-point}}
  struct FrozenRecord { const float x; float y, z; } frozenRecord{0, 0, 0};
  auto record_value = normalize(frozenRecord);
  const float frozen[3] = {};
  auto array_value = normalize(frozen);
  cross(b, b); // expected-error {{cross requires three components}}
  float left[2][3], right[2][2], result[2][2];
  mul(left, right); // expected-error {{matrix multiplication dimensions do not match}}
  mul(left, b); // expected-error {{matrix-vector multiplication dimensions do not match}}
  int integer_matrix[2][2];
  mul(integer_matrix, integer_matrix); // expected-error {{matrix multiplication requires matching floating element types}}
  constexpr unsigned specialized = specialization_constant(0, 4u); // expected-error {{must be initialized by a constant expression}}
  inverse(left); // expected-error {{inverse and determinant require matching square matrices}}
  transpose(result, left); // expected-error {{too many arguments}}
  specialization_constant(dynamic, 1); // expected-error {{constant unsigned 32-bit ID}}
  specialization_constant(0, dynamic); // expected-error {{constant unsigned 32-bit ID}}
  specialization_constant(-1, 1); // expected-error {{constant unsigned 32-bit ID}}
  specialization_constant(0x100000000ULL, 1); // expected-error {{constant unsigned 32-bit ID}}
  int local;
  atomicLoad(&local); // expected-error {{writable 32-bit or 64-bit shared or storage integers}}
  atomicAdd(readonly, 1u); // expected-error {{writable 32-bit or 64-bit shared or storage integers}}
  atomicAdd(1, 1); // expected-error {{atomic operations require a pointer}}
  subgroup_add(a);
  subgroup_and(1.0f); // expected-error {{requires integer elements}}
  subgroup_ballot(out, true); // expected-error {{too many arguments}}
  subgroup_ballot_bit_count(a); // expected-error {{requires unsigned[4]}}
  nonuniform(a); // expected-error {{requires an integer descriptor index}}
  descriptor_at(local, 0); // expected-error {{undeclared identifier 'descriptor_at'}}
  ddx(local); // expected-error {{derivatives require a floating-point scalar}}
}
void bad_resources(texture_2d<float> texture, sampler sampler,
                   storage_texture_2d<float, r32f> storage) {
  float value, uv[2], gradient[3];
  int xy[2];
  sampler.load(value, xy); // expected-error {{member reference type 'sampler'}} expected-error {{no member named 'load'}}
  texture.store(0, 0, value); // expected-error {{requires a storage texture}}
  storage.sample(sampler, 0.0f, 0.0f); // expected-error {{requires a sampled non-multisampled texture}}
  texture.sample(texture, 0.0f, 0.0f); // expected-error {{requires a sampled non-multisampled texture}}
  texture.load(xy, 0); // expected-error {{must be scalars}}
  texture.sample_grad(sampler, 0.0f, 0.0f, gradient, 0.0f, 0.0f, 0.0f); // expected-error {{must be scalars}}
  ray_query query = make_ray_query();
  query.intersection_type(2); // expected-error {{selector must be the constant}}
}

void legacy_atomic_names(__storage__ unsigned *p) {
  atomic_load(p); // expected-error {{undeclared identifier 'atomic_load'}}
  atomic_store(p, 1u); // expected-error {{undeclared identifier 'atomic_store'}}
  atomic_add(p, 1u); // expected-error {{undeclared identifier 'atomic_add'}}
  atomic_sub(p, 1u); // expected-error {{undeclared identifier 'atomic_sub'}}
  atomic_exchange(p, 1u); // expected-error {{undeclared identifier 'atomic_exchange'}}
  atomic_min(p, 1u); // expected-error {{undeclared identifier 'atomic_min'}}
  atomic_max(p, 1u); // expected-error {{undeclared identifier 'atomic_max'}}
  atomic_and(p, 1u); // expected-error {{undeclared identifier 'atomic_and'}}
  atomic_or(p, 1u); // expected-error {{undeclared identifier 'atomic_or'}}
  atomic_xor(p, 1u); // expected-error {{undeclared identifier 'atomic_xor'}}
  atomic_compare_exchange(p, 1u, 2u); // expected-error {{undeclared identifier 'atomic_compare_exchange'}}
  static_assert(__is_same(decltype(atomicLoad(p)), unsigned));
  static_assert(__is_same(decltype(atomicStore(p, 1u)), void));
  static_assert(__is_same(decltype(atomicCAS(p, 1u, 2u)), unsigned));
  atomicLoad(); // expected-error {{atomic operation requires its existing arguments or explicit order and scope arguments}}
  atomicStore(p); // expected-error {{atomic operation requires its existing arguments or explicit order and scope arguments}}
  atomicCAS(p, 1u); // expected-error {{atomic operation requires its existing arguments or explicit order and scope arguments}}
}

// expected-note@* {{'atomicLoad' declared here}}
