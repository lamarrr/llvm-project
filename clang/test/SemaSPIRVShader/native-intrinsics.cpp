// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
void invalid(int dynamic, __storage const unsigned *readonly) {
  float a[3], b[2], out[3];
  // expected-error@+1 {{vector operand dimensions must match}}
  add(a, b);
  // expected-error@+1 {{too few arguments}}
  dot(a);
  int integers[3];
  // expected-error@+1 {{math operands must be floating-point}}
  dot(integers, integers);
  struct FrozenRecord {
    const float x;
    float y, z;
  } frozenRecord{0, 0, 0};
  auto record_value = normalize(frozenRecord);
  const float frozen[3] = {};
  auto array_value = normalize(frozen);
  // expected-error@+1 {{cross requires three components}}
  cross(b, b);
  float left[2][3], right[2][2], result[2][2];
  // expected-error@+1 {{matrix multiplication dimensions do not match}}
  mul(left, right);
  // expected-error@+1 {{matrix-vector multiplication dimensions do not match}}
  mul(left, b);
  int integer_matrix[2][2];
  // expected-error@+1 {{matrix multiplication requires matching floating element types}}
  mul(integer_matrix, integer_matrix);
  // expected-error@+1 {{must be initialized by a constant expression}}
  constexpr unsigned specialized = specialization_constant(0, 4u);
  // expected-error@+1 {{inverse and determinant require matching square matrices}}
  inverse(left);
  // expected-error@+1 {{too many arguments}}
  transpose(result, left);
  // expected-error@+1 {{constant unsigned 32-bit ID}}
  specialization_constant(dynamic, 1);
  // expected-error@+1 {{constant unsigned 32-bit ID}}
  specialization_constant(0, dynamic);
  // expected-error@+1 {{constant unsigned 32-bit ID}}
  specialization_constant(-1, 1);
  // expected-error@+1 {{constant unsigned 32-bit ID}}
  specialization_constant(0x100000000ULL, 1);
  int local;
  // expected-error@+1 {{writable 32-bit or 64-bit shared or storage integers}}
  atomic_load(&local);
  // expected-error@+1 {{writable 32-bit or 64-bit shared or storage integers}}
  atomic_add(readonly, 1u);
  // expected-error@+1 {{atomic operations require a pointer}}
  atomic_add(1, 1);
  subgroup_add(a);
  // expected-error@+1 {{requires integer elements}}
  subgroup_and(1.0f);
  // expected-error@+1 {{too many arguments}}
  subgroup_ballot(out, true);
  // expected-error@+1 {{requires unsigned[4]}}
  subgroup_ballot_bit_count(a);
  // expected-error@+1 {{requires an integer descriptor index}}
  nonuniform(a);
  // expected-error@+1 {{undeclared identifier 'descriptor_at'}}
  descriptor_at(local, 0);
  // expected-error@+1 {{derivatives require a floating-point scalar or vector}}
  ddx(local);
}
void bad_resources(texture_2d<float> texture, sampler sampler,
                   storage_texture_2d<float, r32f> storage) {
  float value, uv[2], gradient[3];
  int xy[2];
  // expected-error@+2 {{member reference type 'sampler'}}
  // expected-error@+1 {{no member named 'load'}}
  sampler.load(value, xy);
  // expected-error@+1 {{requires a storage texture}}
  texture.store(0, 0, value);
  storage.sample(
      sampler, 0.0f,
      // expected-error@+1 {{requires a sampled non-multisampled texture}}
      0.0f);
  texture.sample(
      texture, 0.0f,
      // expected-error@+1 {{requires a sampled non-multisampled texture}}
      0.0f);
  // expected-error@+1 {{must be scalars}}
  texture.load(xy, 0);
  texture.sample_grad(sampler, 0.0f, 0.0f, gradient, 0.0f, 0.0f,
                      // expected-error@+1 {{must be scalars}}
                      0.0f);
  ray_query query = make_ray_query();
  query.intersection_type(
      // expected-error@+1 {{selector must be the constant}}
      2);
  // expected-error@+1 {{selector must be the constant}}
  query.object_to_world(2);
}

void legacy_atomic_names(__storage unsigned *p) {
  // expected-error@+1 {{undeclared identifier 'atomicLoad'}}
  atomicLoad(p);
  // expected-error@+1 {{undeclared identifier 'atomicStore'}}
  atomicStore(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicAdd'}}
  atomicAdd(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicSub'}}
  atomicSub(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicExch'}}
  atomicExch(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicMin'}}
  atomicMin(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicMax'}}
  atomicMax(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicAnd'}}
  atomicAnd(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicOr'}}
  atomicOr(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicXor'}}
  atomicXor(p, 1u);
  // expected-error@+1 {{undeclared identifier 'atomicCAS'}}
  atomicCAS(p, 1u, 2u);
  static_assert(__is_same(decltype(atomic_load(p)), unsigned));
  static_assert(__is_same(decltype(atomic_store(p, 1u)), void));
  static_assert(
      __is_same(decltype(atomic_compare_exchange(p, 1u, 2u)), unsigned));
  // expected-error@+1 {{atomic operation requires its existing arguments or explicit order and scope arguments}}
  atomic_load();
  // expected-error@+1 {{atomic operation requires its existing arguments or explicit order and scope arguments}}
  atomic_store(p);
  // expected-error@+1 {{atomic operation requires its existing arguments or explicit order and scope arguments}}
  atomic_compare_exchange(p, 1u);
}

// expected-note@* {{'atomic_load' declared here}}
