// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=fragment_builtins -o %t.spv %t.bc

// CHECK: !spirv.ExecutionMode = !{![[MODE:[0-9]+]]}
// CHECK: ![[MODE]] = !{ptr @{{.*fragment_builtins.*}}, i32 12}

__fragment
void fragment_builtins(__frag_depth float *depth,
                       __sample_mask unsigned (*mask)[1],
                       __output(0) float *color) {
  *depth = 0.5f;
  (*mask)[0] = 1u;
  *color = 1.0f;
}
