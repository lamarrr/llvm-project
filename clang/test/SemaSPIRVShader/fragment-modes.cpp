// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_STAGE -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_CONFLICT -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_DEPTH -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_STENCIL -emit-llvm -o %t.ll -verify %s

#ifdef TEST_STAGE
__compute
__early_fragment_tests
__workgroup_size(1, 1, 1)
// expected-error@+1 {{fragment execution modes require a fragment entry point}}
void wrong_stage() {}
#endif

#ifdef TEST_STENCIL
__fragment
// expected-error@+1 {{__stencil_ref requires one fragment output pointer}}
void wrong_stencil(__stencil_ref float *stencil) { *stencil = 1.0f; }
#endif

#ifdef TEST_CONFLICT
__fragment
__depth_greater
__depth_less
// expected-error@+1 {{fragment depth modes are mutually exclusive}}
void conflicting_depth(__frag_depth float *depth) { *depth = 0.5f; }
#endif

#ifdef TEST_DEPTH
__fragment
__depth_unchanged
// expected-error@+1 {{fragment depth modes require a __frag_depth output}}
void missing_depth(__output(0) float *color) { *color = 1.0f; }
#endif
