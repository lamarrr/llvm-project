// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=vector_extended --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=VECTOR
// RUN: clang-spirv-link --entry=vector_extended -o %t.spv %t.bc
// RUN: clang-spirv-link --entry=unsigned_clamp --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=UNSIGNED
// RUN: clang-spirv-link --entry=unsigned_clamp -o %t.unsigned.spv %t.bc
// RUN: clang-spirv-link --entry=signed_clamp --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=SIGNED
// RUN: clang-spirv-link --entry=signed_clamp -o %t.signed.spv %t.bc
// RUN: clang-spirv-link --entry=vector_cross --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=CROSS
// RUN: clang-spirv-link --entry=vector_cross -o %t.cross.spv %t.bc

// VECTOR-DAG: call <2 x float> @llvm.spv.lerp.v2f32
// VECTOR-DAG: call float @llvm.spv.lerp.f32
// VECTOR-DAG: call <2 x float> @_Z6FClamp
// VECTOR-DAG: call <2 x float> @llvm.spv.smoothstep.v2f32
// VECTOR-DAG: call <2 x float> @llvm.spv.rsqrt.v2f32
// VECTOR-DAG: call <2 x float> @llvm.spv.frac.v2f32
// VECTOR-DAG: call <2 x float> @_Z4Step
// VECTOR-DAG: call <2 x float> @_Z5FSign
// VECTOR-DAG: call <2 x float> @_Z12__spirv_FMod
// VECTOR-DAG: call <2 x float> @llvm.spv.normalize.v2f32
// VECTOR-DAG: call <2 x float> @llvm.spv.reflect.v2f32
// VECTOR-DAG: call <2 x float> @llvm.spv.refract.v2f32.f32
// VECTOR-DAG: call <2 x float> @llvm.spv.faceforward.v2f32
// VECTOR-DAG: call float @llvm.spv.length.v2f32
// VECTOR-DAG: call float @llvm.spv.distance.v2f32
// VECTOR-DAG: call <2 x float> @_Z19__spirv_shader_DPdx
// VECTOR-DAG: call <2 x float> @_Z25__spirv_shader_FwidthFine
// VECTOR-DAG: call <2 x float> @_Z27__spirv_shader_FwidthCoarse
// UNSIGNED: call <2 x i32> @_Z6UClamp
// SIGNED: call <2 x i32> @_Z6SClamp
// SIGNED: call <2 x i32> @_Z5SSign
// CROSS: call <3 x float> @_Z5Cross

#include <spirv/types.hpp>
using namespace spirv;

__fragment
void vector_extended(__input(0) f32x2 first, __input(1) f32x2 second,
                     __output(0) f32x2 *result) {
  *result = mix(first, second, 0.25f) +
            f32x2(lerp(first.x(), second.x(), first.y())) +
            clamp(first, 0.0f, 1.0f) +
            smoothstep(0.0f, 1.0f, first) + inverse_sqrt(first) +
            fract(first) + step(0.5f, first) + sign(first) +
            mod(first, second + f32x2(1.0f)) + normalize(first) +
            reflect(first, second) + refract(first, second, 0.5f) +
            faceforward(first, second, first) +
            f32x2(length(first) + distance(first, second)) + ddx(first) +
            fwidth_fine(first) + fwidth_coarse(first);
}

__fragment
void unsigned_clamp(__input(0) __flat u32x2 value,
                    __output(0) u32x2 *result) {
  *result = clamp(value, 1u, 7u);
}

__fragment
void signed_clamp(__input(0) __flat i32x2 value,
                  __output(0) i32x2 *result) {
  *result = clamp(value, -7, 7) + sign(value);
}

__fragment
void vector_cross(__input(0) f32x3 first, __input(1) f32x3 second,
                  __output(0) f32x3 *result) {
  *result = cross(first, second);
}
