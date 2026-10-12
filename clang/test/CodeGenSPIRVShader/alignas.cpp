// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=aligned_objects -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=aligned_objects -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O3 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=aligned_objects -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s

struct alignas(16) Item {
  float value;
};
struct Params {
  __storage Item *items;
  alignas(16) __storage float *scalar;
};
static_assert(alignof(Item) == 16 && sizeof(Item) == 16);
static_assert(alignof(Params) == 16 && sizeof(Params) == 32);

__compute
__workgroup_size(1, 1, 1)
void aligned_objects(__push_constant const Params *params) {
  params->items[0].value = 1.0f;
  params->items[1].value = 2.0f;
  *params->scalar = 3.0f;
}

// CHECK: OpStore {{.*}} Aligned 16
// CHECK: OpStore {{.*}} Aligned 16
// CHECK: OpStore {{.*}} Aligned 4
