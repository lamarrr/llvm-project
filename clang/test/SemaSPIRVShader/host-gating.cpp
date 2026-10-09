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
struct format {};
int global_invocation_id;
int __flat__, __noperspective__, __centroid__, __sample__;

template <class T> struct texture_2d {};
struct sampler {};
struct ray_query {};
int dot, length, add, mul, atomicAdd, specialization_constant;
