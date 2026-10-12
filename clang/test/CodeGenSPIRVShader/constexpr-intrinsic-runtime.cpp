// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=intrinsic_runtime --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s
// RUN: clang-spirv-link --entry=intrinsic_runtime -o %t.spv %t.bc

// CHECK: call float @llvm.sin.f32

constexpr float constant = sin(0.0f);
static_assert(constant == 0.0f);

__fragment void intrinsic_runtime(__input(0) float input,
                                    __output(0) float *output) {
  *output = sin(input);
}
