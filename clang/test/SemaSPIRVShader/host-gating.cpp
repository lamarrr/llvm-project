// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fsyntax-only -verify %s
// expected-no-diagnostics
// Shader keywords must not reserve identifiers in ordinary C++ programs.
int __compute__
, __vertex__
, __fragment__
, __workgroup_size__
, __binding__;
int __storage__, __uniform__, __push_constant__, __shared__, __align__;
int __input__, __output__, __position__, __task__, __mesh__, __ray_generation__;
int __geometry__, __geometry_mode__, __geometry_invocations__, __output_vertices__;
int __tessellation_mode__, __mesh_output__, __builtin__, __patch__, __per_primitive__;
int __task_payload__, __ray_payload__, __incoming_ray_payload__, __hit_attribute__;
int __callable_data__, __incoming_callable_data__, __shader_record__;
struct format {};
int global_invocation_id;
int __flat__, __noperspective__, __centroid__, __sample__;

template <class T> struct texture_2d {};
struct sampler {};
struct ray_query {};
int dot, length, add, mul, atomic_add, specialization_constant;
