// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=operations -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=operations -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DATOMIC_TYPES -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_types -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s --check-prefixes=CHECK,TYPES
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DATOMIC_TYPES -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=atomic_types -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s --check-prefixes=CHECK,TYPES

// CHECK-DAG: OpAtomicLoad
// CHECK-DAG: OpAtomicStore
// CHECK-DAG: OpAtomicIAdd
// CHECK-DAG: OpAtomicISub
// CHECK-DAG: OpAtomicExchange
// CHECK-DAG: OpAtomicUMin
// CHECK-DAG: OpAtomicUMax
// CHECK-DAG: OpAtomicAnd
// CHECK-DAG: OpAtomicOr
// CHECK-DAG: OpAtomicXor
// CHECK-DAG: OpAtomicCompareExchange
// TYPES-DAG: OpAtomicSMin
// TYPES-DAG: OpAtomicSMax
// TYPES-DAG: OpCapability Int64Atomics

#ifdef ATOMIC_TYPES
#include "../../../shader-cxx/tests/fixtures/atomic-types.cpp"
#else
#include "../../../shader-cxx/tests/fixtures/operations.cpp"
#endif
