// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -fsyntax-only -verify %s

unsigned dynamic_id;

__compute__
__workgroup_size_id__(0, 4, 1, 2, 2, 1)
void valid();

__compute__
// expected-error@+1 {{workgroup dimension defaults must be positive}}
__workgroup_size_id__(0, 0, 1, 2, 2, 1)
void zero_default();

__compute__
// expected-error@+1 {{requires parameter 1 to be an integer constant}}
__workgroup_size_id__(dynamic_id, 4, 1, 2, 2, 1)
void dynamic_id_error();

__compute__
__workgroup_size__(4, 2, 1)
// expected-error@+1 {{conflicting workgroup sizes}}
__workgroup_size_id__(0, 4, 1, 2, 2, 1)
void conflict();
