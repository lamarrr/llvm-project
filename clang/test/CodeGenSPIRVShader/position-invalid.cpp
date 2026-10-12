// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -emit-llvm -o %t.ll %s 2>&1 \
// RUN:   | FileCheck %s
#include <spirv/types.hpp>
using namespace spirv;

__vertex__
void invalid_position(__position__ f32x3 *output) {
  *output = f32x3(0, 0, 1);
}

// CHECK: __position__ requires one vertex output pointer to float[4] or an equivalent four-float record
