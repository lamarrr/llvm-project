// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_IMPLICIT -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_MISSING -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_DUPLICATE -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_PARAMETER -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_OVERLAP -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_SCALAR -emit-llvm -o %t.ll -verify %s

// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify=syntax %s

#ifdef TEST_IMPLICIT
struct Mixed {
  float x;
  unsigned y;
};
__vertex
// expected-error@+1 {{interface record members require explicit __location(N); omit the parameter location}}
void invalid(__output(0) Mixed *output) {}
#endif

#ifdef TEST_MISSING
struct Partial {
  float x __location(2);
  float y;
};
__vertex
// expected-error@+1 {{interface record members require explicit, non-overlapping __location(N)}}
void invalid(__output Partial *output) {}
#endif

#ifdef TEST_DUPLICATE
struct Duplicate {
  float x __location(2);
  float y __location(2);
};
__vertex
// expected-error@+1 {{interface record members require explicit, non-overlapping __location(N)}}
void invalid(__output Duplicate *output) {}
#endif

#ifdef TEST_PARAMETER
struct Located {
  float x __location(2);
};
__vertex
// expected-error@+1 {{interface record members require explicit __location(N); omit the parameter location}}
void invalid(__output(0) Located *output) {}
#endif

#ifdef TEST_OVERLAP
struct Located {
  float x __location(2);
};
__vertex
// expected-error@+1 {{shader interface locations overlap}}
void invalid(__input Located input, __input(2) float other) {}
#endif

#ifdef TEST_SCALAR
__vertex
// expected-error@+1 {{interface record members require explicit, non-overlapping __location(N)}}
void invalid(__output float *output) {}
#endif

#ifndef TEST_IMPLICIT
#ifndef TEST_MISSING
#ifndef TEST_DUPLICATE
#ifndef TEST_PARAMETER
#ifndef TEST_OVERLAP
#ifndef TEST_SCALAR
// syntax-error@+1 {{'__location' only applies to non-static data members}}
int global __location(0);
struct Conflicting {
  // syntax-error@+1 {{conflicting shader member locations}}
  int value __location(0) __location(1);
};
// syntax-error@+2 {{'__location' attribute takes one argument}}
struct MissingArgument {
  int value __location();
};
#endif
#endif
#endif
#endif
#endif
#endif
