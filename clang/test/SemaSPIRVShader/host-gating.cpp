// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fsyntax-only -verify %s
// expected-no-diagnostics
// Shader keywords must not reserve identifiers in ordinary C++ programs.
int __compute__, __vertex__, __fragment__, __workgroup_size__, __binding__;
int __storage__, __uniform__, __push_constant__, __shared__, __align__;
int __input__, __output__, __task__, __mesh__, __ray_generation__;
struct ImageFormat {};
int global_invocation_id;
