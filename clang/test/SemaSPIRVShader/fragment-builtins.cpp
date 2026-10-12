// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_DEPTH -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -DTEST_MASK -emit-llvm -o %t.ll -verify %s

__fragment__
void valid(__frag_depth__ float *depth, __sample_mask__ unsigned (*mask)[1]) {
  *depth = 0.5f;
  (*mask)[0] = 1u;
}

#ifdef TEST_DEPTH
__vertex__
// expected-error@+1 {{__frag_depth__ requires one fragment output pointer}}
void wrong_stage(__frag_depth__ float *depth) { *depth = 0.5f; }
#endif

#ifdef TEST_MASK
__fragment__
// expected-error@+1 {{__sample_mask__ requires at most one fragment input pointer}}
void wrong_mask(__sample_mask__ float *mask) { *mask = 1.0f; }
#endif
