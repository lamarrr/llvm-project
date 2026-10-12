// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
static_assert(__is_class(ray_query));
static_assert(!__is_pointer(ray_query));
static_assert(__is_same(decltype(make_ray_query()), ray_query));
static_assert(__is_same(decltype(((ray_query *)nullptr)->proceed()), bool));
static_assert(__is_same(
    decltype(((const ray_query *)nullptr)->intersection_t(1)), float));
static_assert(__is_same(
    decltype(((const ray_query *)nullptr)->primitive_idx(1)), unsigned));
static_assert(__is_same(decltype(((const ray_query *)nullptr)->front_facing(1)),
                        bool));
template <class Q> bool advance(Q &q) { return q.proceed(); }
void methods(ray_query &q, acceleration_structure scene, int dynamic) {
  float origin[3], direction[3];
  q.initialize(scene, 0, 255, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
               1000.0f);
  advance(q);
  ray_query *pointer = &q;
  pointer->terminate();
  q.proceed(
      1); // expected-error@-1 {{ray query method proceed requires 0 arguments}}
  q.initialize(); // expected-error {{ray query method initialize requires 11 arguments}}
  q.generate_intersection(); // expected-error {{ray query method generate_intersection requires 1 arguments}}
  q.primitive_idx(dynamic);  // expected-error {{selector must be the constant}}
  q.intersection_type(2);    // expected-error {{selector must be the constant}}
  q.initialize(
      1, 0, 255, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      1000.0f); // expected-error {{requires a native acceleration_structure}}
  (void)q.__handle; // expected-error {{private member}}
  ray_query_initialize(
      q, scene, 0, 255, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      1000.0f); // expected-error@-2 {{undeclared identifier 'ray_query_initialize'}}
  ray_query_proceed(
      q); // expected-error@-1 {{undeclared identifier 'ray_query_proceed'}}
  ray_query_terminate(
      q); // expected-error@-1 {{undeclared identifier 'ray_query_terminate'}}
  ray_query_confirm_intersection(
      q); // expected-error@-1 {{undeclared identifier 'ray_query_confirm_intersection'}}
  ray_query_generate_intersection(
      q,
      1.0f); // expected-error@-2 {{undeclared identifier 'ray_query_generate_intersection'}}
  ray_query_intersection_type(
      q,
      0); // expected-error@-2 {{undeclared identifier 'ray_query_intersection_type'}}
  ray_query_intersection_t(
      q,
      0); // expected-error@-2 {{undeclared identifier 'ray_query_intersection_t'}}
  ray_query_primitive_idx(
      q,
      0); // expected-error@-2 {{undeclared identifier 'ray_query_primitive_idx'}}
  ray_query_instance_custom_idx(
      q,
      0); // expected-error@-2 {{undeclared identifier 'ray_query_instance_custom_idx'}}
  ray_query_geometry_idx(
      q,
      0); // expected-error@-2 {{undeclared identifier 'ray_query_geometry_idx'}}
  ray_query_front_facing(
      q,
      0); // expected-error@-2 {{undeclared identifier 'ray_query_front_facing'}}
}
void readonly(const ray_query &q) {
  q.intersection_type(0);
  q.intersection_t(1);
  q.primitive_idx(1);
  q.instance_custom_idx(1);
  q.geometry_idx(1);
  q.front_facing(1);
  q.confirm_intersection(); // expected-error {{ray query mutation requires a non-const receiver}}
  q.generate_intersection(
      1.0f); // expected-error {{ray query mutation requires a non-const receiver}}
  q.proceed(); // expected-error {{ray query mutation requires a non-const receiver}}
  q.terminate(); // expected-error {{ray query mutation requires a non-const receiver}}
}
// expected-note@* {{implicitly declared private here}}
