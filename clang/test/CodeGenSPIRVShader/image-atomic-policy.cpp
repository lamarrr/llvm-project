// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=image_atomic_policy -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: FileCheck %s --check-prefix=REFLECT < %t.spv.json
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=image_atomic_policy -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: FileCheck %s --check-prefix=REFLECT < %t.spv.json

// CHECK-DAG: OpImageTexelPointer
// CHECK-DAG: OpAtomicLoad
// CHECK-DAG: OpAtomicStore
// CHECK-DAG: OpAtomicIAdd
// CHECK-DAG: OpAtomicISub
// CHECK-DAG: OpAtomicExchange
// CHECK-DAG: OpAtomicCompareExchange
// CHECK-DAG: OpAtomicUMin
// CHECK-DAG: OpAtomicUMax
// CHECK-DAG: OpAtomicSMin
// CHECK-DAG: OpAtomicSMax
// CHECK-DAG: OpAtomicAnd
// CHECK-DAG: OpAtomicOr
// CHECK-DAG: OpAtomicXor
// CHECK-DAG: OpAtomicFAddEXT
// CHECK-DAG: OpAtomicFMinEXT
// CHECK-DAG: OpAtomicFMaxEXT
// CHECK-DAG: OpCapability Int64Atomics
// REFLECT-DAG: "shaderImageFloat32Atomics"
// REFLECT-DAG: "shaderImageFloat32AtomicAdd"
// REFLECT-DAG: "shaderImageFloat32AtomicMinMax"
// REFLECT-DAG: "shaderImageInt64Atomics"

#include "../../../shader-cxx/tests/fixtures/image-atomic-policy.cpp"
