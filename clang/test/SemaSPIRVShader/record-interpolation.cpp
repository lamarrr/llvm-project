// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_INTEGER -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_FLOAT -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_STAGE -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_SAMPLING -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_COMPONENT -emit-llvm -o %t.ll -verify %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify=syntax %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -DTEST_DOUBLE -emit-llvm -o %t.ll -verify %s

#ifdef TEST_INTEGER
struct Integers {
  unsigned value __location(0);
};
__fragment
// expected-error@+1 {{integer fragment inputs require __flat}}
void invalid(__input Integers input) {}
#elif defined(TEST_FLOAT)
struct Integers {
  unsigned value __location(0) __varying;
};
__vertex
// expected-error@+1 {{interpolated inputs and outputs require floating-point values}}
void invalid(__output Integers *output) {}
#elif defined(TEST_STAGE)
struct Floats {
  float value __location(0) __varying;
};
__vertex
// expected-error@+1 {{interpolation qualifiers require vertex outputs or fragment inputs}}
void invalid(__input Floats input) {}
#elif defined(TEST_SAMPLING)
struct Floats {
  float value __location(0) __centroid;
};
__fragment
// expected-error@+1 {{conflicting interpolation qualifiers}}
void invalid(__input __flat Floats input) {}
#elif defined(TEST_COMPONENT)
struct Components {
  float first __varying;
  float second;
};
__vertex
// expected-error@+1 {{interface record members require explicit __location(N)}}
void invalid(__output(0) Components *output) {}
#elif defined(TEST_DOUBLE)
struct Doubles {
  double value __location(0) __varying;
};
__fragment
// expected-error@+1 {{double-precision fragment inputs require __flat}}
void invalid(__input Doubles input) {}
#else
struct Conflicts {
  // syntax-error@+1 {{conflicting interpolation qualifiers}}
  float flat_varying __location(0) __flat __varying;
  // syntax-error@+1 {{conflicting interpolation qualifiers}}
  float varying_linear __location(1) __varying __noperspective;
  // syntax-error@+1 {{conflicting interpolation qualifiers}}
  float flat_centroid __location(2) __flat __centroid;
  // syntax-error@+1 {{conflicting interpolation qualifiers}}
  float flat_sample __location(3) __flat __sample;
  // syntax-error@+1 {{conflicting interpolation qualifiers}}
  float sample_centroid __location(4) __sample __centroid;
};
// syntax-error@+1 {{interpolation qualifiers must be declared on entry-point parameters or interface record fields}}
float global __varying;
#endif
