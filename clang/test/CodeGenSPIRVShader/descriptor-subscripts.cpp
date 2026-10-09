// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -emit-llvm -o - %s | FileCheck %s
using textures = texture_2d<float>[];
textures &source();
unsigned offset();
// CHECK-LABEL: define{{.*}} void @_Z6accessv()
// CHECK: call{{.*}} @_Z6sourcev()
// CHECK: call{{.*}} @_Z6offsetv()
// CHECK: getelementptr
// CHECK: call{{.*}} @_Z6offsetv()
// CHECK: call{{.*}} @_Z6sourcev()
// CHECK: getelementptr
void access() {
  auto first = source()[offset()];
  auto second = offset()[source()];
}
