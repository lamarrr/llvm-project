// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_STAGE -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_CONFLICT -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_DEPTH -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_STENCIL -emit-llvm -o %t.ll -verify %s

#ifdef TEST_STAGE
__compute__ __early_fragment_tests__ __workgroup_size__(1, 1, 1)
// expected-error@+1 {{fragment execution modes require a fragment entry point}}
void wrong_stage() {}
#endif

#ifdef TEST_STENCIL
__fragment__
// expected-error@+1 {{__stencil_ref__ requires one fragment output pointer}}
void wrong_stencil(__stencil_ref__ float *stencil) { *stencil = 1.0f; }
#endif

#ifdef TEST_CONFLICT
__fragment__ __depth_greater__ __depth_less__
// expected-error@+1 {{fragment depth modes are mutually exclusive}}
void conflicting_depth(__frag_depth__ float *depth) { *depth = 0.5f; }
#endif

#ifdef TEST_DEPTH
__fragment__ __depth_unchanged__
// expected-error@+1 {{fragment depth modes require a __frag_depth__ output}}
void missing_depth(__output__(0) float *color) { *color = 1.0f; }
#endif
