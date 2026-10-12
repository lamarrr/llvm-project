// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=integer_dot_test -o %t.spv %t.bc

#include <spirv/types.hpp>
using namespace spirv;

struct Output {
  __storage__ int *data;
};

__compute__
__workgroup_size__(1, 1, 1)
void integer_dot_test(__push_constant__ const Output *output) {
  i8x4 values(1, 2, 3, 4);
  output->data[0] = integer_dot(values, values);
}
