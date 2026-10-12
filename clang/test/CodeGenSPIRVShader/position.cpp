// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -I %S/../../lib/Headers -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=vector_position --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=LINK
#include <spirv/types.hpp>
using namespace spirv;
// CHECK: @__spirv_cxx_{{.*}}_arg0 = external addrspace(8) global [4 x float]
// CHECK-SAME: !spirv.Decorations ![[DECORATIONS:[0-9]+]]
// CHECK: @__spirv_cxx_{{.*}}_arg0 = external addrspace(8) global %"struct.spirv::vec"
// CHECK-SAME: !spirv.Decorations ![[DECORATIONS]]
// CHECK: ![[DECORATIONS]] = !{![[POSITION:[0-9]+]]}
// CHECK: ![[POSITION]] = !{i32 11, i32 0}
// LINK: @__spirv_cxx_{{.*}}_arg0.position.interface = external addrspace(8) global <4 x float>
// LINK-SAME: !spirv.Decorations ![[DECORATIONS:[0-9]+]]
// LINK: store float
// LINK: ![[POSITION:[0-9]+]] = !{i32 11, i32 0}
// LINK: ![[DECORATIONS]] = !{![[POSITION]]}
__vertex__
void position(__position__ float (*output)[4]) {
  (*output)[0] = 0;
  (*output)[1] = 0;
  (*output)[2] = 0;
  (*output)[3] = 1;
}

__vertex__
void vector_position(__position__ f32x4 *output) {
  *output = f32x4(0, 0, 0, 1);
}
