// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=printf_test -o %t.spv %t.bc
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -O2 -emit-llvm-bc -o %t.opt.bc %s
// RUN: clang-spirv-link --entry=printf_test -o %t.opt.spv %t.opt.bc

#include <spirv/types.hpp>

using namespace spirv;

struct Params {
  float value;
  int index;
};

__compute
__workgroup_size(1, 1, 1)
void printf_test(__push_constant const Params *params) {
  printf("value %f", params->value);
  printf("value %f", params->value);
  printf("index %d vector %v2f", params->index, f32x2(params->value, 2.0f));
  printf("wide %lu", 1ull);
  printf("literal %% and quote \"%d\" $", params->index);
  printf("path C:\\data");
  printf("no values");
}
