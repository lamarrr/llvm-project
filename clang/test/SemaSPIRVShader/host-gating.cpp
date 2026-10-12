// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fsyntax-only -verify %s
// expected-no-diagnostics
// Shader keywords must not reserve identifiers in ordinary C++ programs.
int __compute
, __vertex
, __fragment
, __workgroup_size
, __binding;
int __storage, __uniform, __push_constant, __shared, __align__;
int __input, __output, __location, __varying, __position, __task, __mesh, __ray_generation;
int __geometry, __geometry_mode, __geometry_invocations, __output_vertices;
int __tessellation_mode, __mesh_output, __patch, __per_primitive;
int __task_payload, __ray_payload, __incoming_ray_payload, __hit_attribute;
int __callable_data, __incoming_callable_data, __shader_record;
struct format {};
int global_invocation_id;
int __flat, __noperspective, __centroid, __sample;

template <class T> struct texture_2d {};
struct sampler {};
struct ray_query {};
int dot, length, add, mul, atomic_add, specialization_constant;

int __primitive_id;
int __invocation_id;
int __layer;
int __viewport_index;
int __tess_level_outer;
int __tess_level_inner;
int __tess_coord;
int __patch_vertices;
int __primitive_point_indices;
int __primitive_line_indices;
int __primitive_triangle_indices;
int __cull_primitive;
