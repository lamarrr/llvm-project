// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_float_policy -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: FileCheck %s --check-prefix=REFLECT < %t.spv.json
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_float_policy -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: FileCheck %s --check-prefix=REFLECT < %t.spv.json

// CHECK-DAG: OpAtomicLoad
// CHECK-DAG: OpAtomicStore
// CHECK-DAG: OpAtomicExchange
// CHECK-DAG: OpCapability AtomicFloat16AddEXT
// CHECK-DAG: OpCapability AtomicFloat16MinMaxEXT
// REFLECT-DAG: "shaderBufferFloat32Atomics"
// REFLECT-DAG: "shaderBufferFloat64Atomics"
// REFLECT-DAG: "shaderSharedFloat32Atomics"
// REFLECT-DAG: "shaderBufferFloat16Atomics"
// REFLECT-DAG: "shaderBufferFloat16AtomicAdd"
// REFLECT-DAG: "shaderBufferFloat16AtomicMinMax"

#include "../../../shader-cxx/tests/fixtures/atomic-float-policy.cpp"
