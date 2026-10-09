// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -emit-llvm -o - %s | FileCheck %s
// CHECK: @__spirv_cxx_{{.*}}_arg0 = external addrspace(8) global [4 x float]
// CHECK-SAME: !spirv.Decorations ![[DECORATIONS:[0-9]+]]
// CHECK: ![[DECORATIONS]] = !{![[POSITION:[0-9]+]]}
// CHECK: ![[POSITION]] = !{i32 11, i32 0}
__vertex__
void position(__position__ float (*output)[4]) {
  (*output)[0] = 0;
  (*output)[1] = 0;
  (*output)[2] = 0;
  (*output)[3] = 1;
}
