// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=sample_vector --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=VECTOR
// RUN: clang-spirv-link --entry=sample_record --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=RECORD
// RUN: clang-spirv-link --entry=sample_scalars --emit-llvm -o - %t.bc \
// RUN:   | FileCheck %s --check-prefix=SCALARS
// VECTOR: call <4 x float> @{{.*}}ImageSampleImplicitLod{{.*}}<2 x float>
// RECORD: call <4 x float> @{{.*}}ImageSampleImplicitLod{{.*}}<2 x float>
// SCALARS: call <4 x float> @{{.*}}ImageSampleImplicitLod{{.*}}<2 x float>
#include <spirv/types.hpp>
using namespace spirv;

struct UV {
  float u, v;
};

__fragment
void sample_vector(__binding(0, 0) texture_2d<f32x4> image,
                   __binding(0, 1) sampler filtering, __input(0) f32x2 uv,
                   __output(0) f32x4 *color) {
  *color = image.sample(filtering, uv);
}

__fragment
void sample_record(__binding(0, 0) texture_2d<f32x4> image,
                   __binding(0, 1) sampler filtering, __input(0) UV uv,
                   __output(0) f32x4 *color) {
  *color = image.sample(filtering, uv);
}

__fragment
void sample_scalars(__binding(0, 0) texture_2d<f32x4> image,
                    __binding(0, 1) sampler filtering, __input(0) f32x2 uv,
                    __output(0) f32x4 *color) {
  *color = image.sample(filtering, uv.x(), uv.y());
}
