// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_STAGE -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_DIRECTION -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_PATCH -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_COMPONENTS -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_SCALAR -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_PLACEMENT -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_ENUM -fsyntax-only -verify %s
// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_ARGUMENT -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix=ARGUMENT
// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST_NUMERIC -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix=NUMERIC

#ifdef TEST_STAGE
__vertex
// expected-error@+1 {{builtin interface has an invalid type, direction or stage}}
void wrong_stage(__tess_coord const float (*coord)[3]) {}
#endif

#ifdef TEST_DIRECTION
__tessellation_evaluation __tessellation_mode(22, 1, 5)
// expected-error@+1 {{builtin interface has an invalid type, direction or stage}}
void wrong_direction(__tess_coord float (*coord)[3]) {}
#endif

#ifdef TEST_PATCH
__tessellation_control __output_vertices(3)
// expected-error@+1 {{builtin interface has an invalid type, direction or stage}}
void missing_patch(__tess_level_outer float (*outer)[4]) {}
#endif

#ifdef TEST_COMPONENTS
__mesh __workgroup_size(1, 1, 1) __mesh_output(mesh_topology::lines, 2, 1)
// expected-error@+1 {{builtin interface has an invalid type, direction or stage}}
void wrong_components(__primitive_line_indices unsigned (*indices)[1]) {}
#endif

#ifdef TEST_SCALAR
__fragment
// expected-error@+1 {{builtin interface has an invalid type, direction or stage}}
void wrong_scalar(__primitive_id const float *primitive) {}
#endif

#ifdef TEST_PLACEMENT
// expected-error@+1 {{only applies to parameters}}
__tess_coord float global_coord[3];
#endif

#ifdef TEST_ARGUMENT
void named_with_argument(__tess_coord(13) const float (*coord)[3]);
// ARGUMENT: error: expected ')'
#endif

#ifdef TEST_NUMERIC
void removed_numeric(__builtin__(13) const float (*coord)[3]);
// NUMERIC: error: use of undeclared identifier '__builtin__'
#endif

#ifdef TEST_ENUM
int removed_enum = shader_builtin::tess_coord; // expected-error {{use of undeclared identifier 'shader_builtin'}}
#endif
