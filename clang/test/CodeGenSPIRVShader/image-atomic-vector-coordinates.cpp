// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -I %S/../../lib/Headers -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=vector_atomics -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -I %S/../../lib/Headers -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=vector_atomics -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s

// CHECK-DAG: OpTypeVector {{.*}} 2
// CHECK-DAG: OpTypeVector {{.*}} 3
// CHECK-DAG: OpImageTexelPointer
// CHECK-DAG: OpAtomicLoad
// CHECK-DAG: OpAtomicStore
// CHECK-DAG: OpAtomicISub
// CHECK-DAG: OpAtomicIAdd
// CHECK-DAG: OpAtomicExchange
// CHECK-DAG: OpAtomicUMin
// CHECK-DAG: OpAtomicUMax
// CHECK-DAG: OpAtomicAnd
// CHECK-DAG: OpAtomicOr
// CHECK-DAG: OpAtomicXor
// CHECK-DAG: OpAtomicCompareExchange
// CHECK-DAG: OpAtomicFAddEXT

#include <spirv/types.hpp>

struct Coordinates {
  int x, y;
};
struct Output {
  __storage__ unsigned *values;
  __storage__ float *floating;
};

__compute__ __workgroup_size__(1, 1, 1)
void vector_atomics(
    __binding__(0, 0) storage_texture_2d<unsigned, r32ui> image,
    __binding__(0, 1) storage_texture_3d<unsigned, r32ui> volume,
    __binding__(0, 2) storage_texture_2d_ms_layers<unsigned, r32ui> multisample,
    __binding__(0, 3) storage_texture_2d<float, r32f> floating,
    __push_constant__ const Output *output) {
  spirv::i32x2 coord(1, 2);
  image.atomic_store(coord, 1u, memory_order::release, memory_scope::device);
  output->values[0] = image.atomic_load(coord, memory_order::acquire,
                                        memory_scope::device);
  output->values[1] = image.atomic_sub(coord, 1u);
  output->values[2] = image.atomic_add(spirv::u32x2(1u, 2u), 1u);
  output->values[3] = image.atomic_exchange(Coordinates{1, 2}, 2u);
  output->values[4] = image.atomic_min(coord, 1u);
  output->values[5] = image.atomic_max(coord, 3u);
  output->values[6] = image.atomic_and(coord, 7u);
  output->values[7] = image.atomic_or(coord, 8u);
  output->values[8] = image.atomic_xor(coord, 2u);
  output->values[9] = image.atomic_compare_exchange(
      coord, 3u, 4u, memory_order::acquire_release, memory_order::acquire,
      memory_scope::device);
  output->values[10] = volume.atomic_sub(spirv::i32x3(1, 2, 3), 1u);
  output->values[11] = multisample.atomic_sub(
      spirv::i32x3(1, 2, 0), 1u, 1u, memory_order::relaxed,
      memory_scope::device);
  output->floating[0] = floating.atomic_sub(coord, 1.0f);
  output->values[12] = image.atomic_sub(1, 2, 1u);
}
