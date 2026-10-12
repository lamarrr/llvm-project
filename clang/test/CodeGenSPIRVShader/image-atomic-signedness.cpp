// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 \
// RUN:   -DSIGNED_FIRST -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_signedness -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 \
// RUN:   -DSIGNED_FIRST -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_signedness -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O3 \
// RUN:   -DSIGNED_FIRST -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_signedness -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 \
// RUN:   -DUNSIGNED_FIRST -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_signedness -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 \
// RUN:   -DUNSIGNED_FIRST -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_signedness -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O3 \
// RUN:   -DUNSIGNED_FIRST -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_signedness -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s

// CHECK-DAG: [[S32:%[^ ]+]] = OpTypeInt 32 1
// CHECK-DAG: [[U32:%[^ ]+]] = OpTypeInt 32 0
// CHECK-DAG: [[S64:%[^ ]+]] = OpTypeInt 64 1
// CHECK-DAG: [[U64:%[^ ]+]] = OpTypeInt 64 0
// CHECK-DAG: [[SP32:%[^ ]+]] = OpTypePointer Image [[S32]]
// CHECK-DAG: [[UP32:%[^ ]+]] = OpTypePointer Image [[U32]]
// CHECK-DAG: [[SP64:%[^ ]+]] = OpTypePointer Image [[S64]]
// CHECK-DAG: [[UP64:%[^ ]+]] = OpTypePointer Image [[U64]]
// CHECK-DAG: [[SI32:%[^ ]+]] = OpTypeImage [[S32]] 2D 0 0 0 2 R32i
// CHECK-DAG: [[UI32:%[^ ]+]] = OpTypeImage [[U32]] 2D 0 0 0 2 R32ui
// CHECK-DAG: [[SI64:%[^ ]+]] = OpTypeImage [[S64]] 2D 0 0 0 2 R64i
// CHECK-DAG: [[UI64:%[^ ]+]] = OpTypeImage [[U64]] 2D 0 0 0 2 R64ui
// CHECK-DAG: [[SIP32:%[^ ]+]] = OpTypePointer UniformConstant [[SI32]]
// CHECK-DAG: [[UIP32:%[^ ]+]] = OpTypePointer UniformConstant [[UI32]]
// CHECK-DAG: [[SIP64:%[^ ]+]] = OpTypePointer UniformConstant [[SI64]]
// CHECK-DAG: [[UIP64:%[^ ]+]] = OpTypePointer UniformConstant [[UI64]]
// CHECK-DAG: [[SIGNED32:%[^ ]+]] = OpVariable [[SIP32]] UniformConstant
// CHECK-DAG: [[UNSIGNED32:%[^ ]+]] = OpVariable [[UIP32]] UniformConstant
// CHECK-DAG: [[SIGNED64:%[^ ]+]] = OpVariable [[SIP64]] UniformConstant
// CHECK-DAG: [[UNSIGNED64:%[^ ]+]] = OpVariable [[UIP64]] UniformConstant
// CHECK-DAG: [[ST32:%[^ ]+]] = OpImageTexelPointer [[SP32]] [[SIGNED32]]
// CHECK-DAG: OpAtomicSMin [[S32]] [[ST32]]
// CHECK-DAG: [[UT32:%[^ ]+]] = OpImageTexelPointer [[UP32]] [[UNSIGNED32]]
// CHECK-DAG: OpAtomicUMin [[U32]] [[UT32]]
// CHECK-DAG: [[ST64:%[^ ]+]] = OpImageTexelPointer [[SP64]] [[SIGNED64]]
// CHECK-DAG: OpAtomicSMax [[S64]] [[ST64]]
// CHECK-DAG: [[UT64:%[^ ]+]] = OpImageTexelPointer [[UP64]] [[UNSIGNED64]]
// CHECK-DAG: OpAtomicUMax [[U64]] [[UT64]]

__compute __workgroup_size(1, 1, 1)
void atomic_signedness(
    __binding(0, 0) storage_texture_2d<int, r32i> signed32,
    __binding(0, 1) storage_texture_2d<unsigned, r32ui> unsigned32,
    __binding(0, 2) storage_texture_2d<long long, r64i> signed64,
    __binding(0, 3) storage_texture_2d<unsigned long long, r64ui> unsigned64) {
#ifdef SIGNED_FIRST
  signed32.atomic_min(0, 0, -1);
  unsigned32.atomic_min(0, 0, 1u);
  signed64.atomic_max(0, 0, -1ll);
  unsigned64.atomic_max(0, 0, 1ull);
#else
  unsigned32.atomic_min(0, 0, 1u);
  signed32.atomic_min(0, 0, -1);
  unsigned64.atomic_max(0, 0, 1ull);
  signed64.atomic_max(0, 0, -1ll);
#endif
}
