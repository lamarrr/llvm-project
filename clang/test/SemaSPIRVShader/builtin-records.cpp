// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_COUNT -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_MIXED -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_PADDED -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_NONTRIVIAL -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_UNION -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_BASE -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_BITS -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_DEPTH -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_STENCIL -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_MASK -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_COORD -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_OUTER -emit-llvm -o %t.ll -verify %s

#if defined(TEST_COUNT)
struct Bad { float x, y, z; };
#elif defined(TEST_MIXED)
struct Bad { float x, y, z; unsigned w; };
#elif defined(TEST_PADDED)
struct alignas(32) Bad { float values[4]; };
#elif defined(TEST_NONTRIVIAL)
struct Bad { float values[4]; ~Bad() {} };
#elif defined(TEST_UNION)
union Bad { float values[4]; };
#elif defined(TEST_BASE)
struct Parent { float x; };
struct Bad : Parent { float y, z, w; };
#endif

#if defined(TEST_COUNT) || defined(TEST_MIXED) || defined(TEST_PADDED) || \
    defined(TEST_NONTRIVIAL) || defined(TEST_UNION) || defined(TEST_BASE)
__vertex__
// expected-error@+1 {{__position__ requires}}
void invalid(__position__ Bad *position) {}
#endif

#ifdef TEST_BITS
struct Bits { unsigned value : 32; };
__fragment__
// expected-error@+1 {{__sample_mask__ requires}}
void invalid(__sample_mask__ Bits *mask) {}
#endif

#ifdef TEST_DEPTH
struct Double { double value; };
__fragment__
// expected-error@+1 {{__frag_depth__ requires}}
void invalid(__frag_depth__ Double *depth) {}
#endif

#ifdef TEST_STENCIL
struct Wide { long long value; };
__fragment__
// expected-error@+1 {{__stencil_ref__ requires}}
void invalid(__stencil_ref__ Wide *stencil) {}
#endif

#ifdef TEST_MASK
struct Signed { int value; };
__fragment__
// expected-error@+1 {{__sample_mask__ requires}}
void invalid(__sample_mask__ Signed *mask) {}
#endif

#ifdef TEST_COORD
struct Float2 { float x, y; };
__tessellation_evaluation__ __tessellation_mode__(22, 1, 5)
// expected-error@+1 {{builtin interface has an invalid type, direction or stage}}
void invalid(__builtin__(13) const Float2 *coord) {}
#endif

#ifdef TEST_OUTER
struct Float4 { float x, y, z, w; };
struct alignas(64) PaddedVertices { Float4 values[3]; };
__mesh__ __workgroup_size__(1, 1, 1) __mesh_output__(mesh_topology::triangles, 3, 1)
// expected-error@+1 {{__position__ requires}}
void invalid(__position__ PaddedVertices *positions) {}
#endif
