// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 \
// RUN:   -fshader-host-math -emit-llvm -o - %s | FileCheck %s

// CHECK-LABEL: define{{.*}} @_Z5blendfff(
// CHECK-DAG: fsub float 1.000000e+00
// CHECK-DAG: fmul float
// CHECK-DAG: fmul float
// CHECK: fadd float
// CHECK-LABEL: define{{.*}} @_Z5boundfff(
// CHECK: call float @llvm.maximum.f32
// CHECK: call float @llvm.minimum.f32

float blend(float first, float second, float weight) {
  return lerp(first, second, weight);
}

float bound(float value, float lower, float upper) {
  return clamp(value, lower, upper);
}
