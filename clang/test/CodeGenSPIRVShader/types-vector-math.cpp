// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=vector_math -o %t.spv %t.bc
// RUN: clang-spirv-link --entry=integer_math -o %t.int.spv %t.bc
// RUN: clang-spirv-link --entry=packing_math -o %t.pack.spv %t.bc
// RUN: clang-spirv-link --entry=unorm_math -o %t.unorm.spv %t.bc
// RUN: clang-spirv-link --entry=integer_operators -o %t.ops.spv %t.bc
// RUN: clang-spirv-link --entry=integer_operators --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=OPS
// RUN: clang-spirv-link --entry=signed_shift -o %t.shift.spv %t.bc
// RUN: clang-spirv-link --entry=signed_shift --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=SIGNED
// RUN: clang-spirv-link --entry=vector3_shift -o %t.shift3.spv %t.bc
// RUN: clang-spirv-link --entry=vector3_shift --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=VECTOR3
// RUN: clang-spirv-link --entry=vector4_shift -o %t.shift4.spv %t.bc
// RUN: clang-spirv-link --entry=vector4_shift --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=VECTOR4
// RUN: clang-spirv-link --entry=logical_operators -o %t.logic.spv %t.bc
// RUN: clang-spirv-link --entry=logical_operators --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=LOGICAL
// RUN: clang-spirv-link --entry=vector3_logic -o %t.logic3.spv %t.bc
// RUN: clang-spirv-link --entry=vector3_logic --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=LOGICAL3
// OPS: shl <2 x i32>
// OPS: lshr <2 x i32>
// OPS: urem <2 x i32>
// SIGNED: ashr <2 x i32>
// VECTOR3: shl <3 x i32>
// VECTOR4: shl <4 x i32>
// LOGICAL-DAG: or <2 x i1>
// LOGICAL-DAG: and <2 x i1>
// LOGICAL-DAG: xor <2 x i1>
// LOGICAL3: or <3 x i1>
#include <spirv/types.hpp>
using namespace spirv;

__fragment
void vector_math(__input(0) f32x2 first, __input(1) f32x2 second,
                 __output(0) f32x2 *result) {
  f32x2 blended = mix(first, second, 0.5f);
  f32x2 bounded = clamp(blended, 0.0f, 1.0f);
  f32x2 remainder = mod(bounded, 0.75f);
  f32x2 chosen = mix(remainder, second, less_than(first, second));
  *result = smoothstep(0.0f, 1.0f, chosen) + min(first, 0.5f) +
            max(0.25f, second);
}

__fragment
void integer_math(__input(0) __flat u32x2 value,
                  __output(0) u32x2 *result) {
  u32x2 carry;
  u32x2 borrow;
  u32x2 low;
  u32x2 high;
  auto sum = uadd_carry(value, value, carry);
  auto difference = usub_borrow(sum, value, borrow);
  umul_extended(difference, value, low, high);
  *result = bitfield_reverse(bitfield_extract(low, 0, 16)) +
            bitfield_insert(high, value, 0, 8) + u32x2(bit_count(value)) +
            carry + borrow;
}

__fragment
void packing_math(__input(0) f32x2 value, __output(0) f32x2 *result) {
  *result = unpack_half2x16(pack_half2x16(value));
}

__fragment
void unorm_math(__input(0) __flat u32 value,
                __output(0) u32 *result) {
  *result = pack_unorm2x16(unpack_unorm2x16(value));
}

__fragment
void integer_operators(__input(0) __flat u32x2 value,
                       __output(0) u32x2 *result) {
  u32x2 bits = (value << u32x2(1u, 2u)) | (1u << (value & 3u));
  bits ^= ~value & 0xffu;
  bits >>= 1u;
  bits %= 7u;
  *result = bits;
}

__fragment
void signed_shift(__input(0) __flat i32x2 value,
                  __output(0) i32x2 *result) {
  *result = value >> 1;
}

__fragment
void vector3_shift(__input(0) __flat u32x3 value,
                   __output(0) u32x3 *result) {
  *result = (value << u32x3(1u, 2u, 3u)) ^ (value & u32x3(7u));
}

__fragment
void vector4_shift(__input(0) __flat u32x4 value,
                   __output(0) u32x4 *result) {
  *result = value << 2u;
}

__fragment
void logical_operators(__input(0) __flat u32x2 value,
                       __output(0) u32x2 *result) {
  b8x2 lower = less_than(value, u32x2(5u));
  b8x2 upper = greater_than(value, u32x2(1u));
  b8x2 mask = (lower || upper) && !(lower ^ upper);
  mask |= lower & upper;
  *result = mix(u32x2(0u), u32x2(1u), mask);
}

__fragment
void vector3_logic(__input(0) __flat u32x3 value,
                   __output(0) u32x3 *result) {
  b8x3 mask = less_than(value, u32x3(5u)) || greater_than(value, u32x3(1u));
  *result = mix(u32x3(0u), u32x3(1u), mask);
}
