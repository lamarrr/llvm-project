// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=qualified_vec_mat -o %t.spv %t.bc
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -O2 -emit-llvm-bc -o %t.opt.bc %s
// RUN: clang-spirv-link --entry=qualified_vec_mat -o %t.opt.spv %t.opt.bc

#include <spirv/types.hpp>

using namespace spirv;

struct Uniforms {
  f32x4 vector;
  f32x4x4 matrix;
};

struct Params {
  f32x4 vector;
  __storage f32x4 *target;
  __storage f32x4x4 *matrix_target;
};

__fragment
void qualified_vec_mat(__binding(0, 0) __uniform const Uniforms *uniform,
                       __push_constant const Params *push,
                       __output(0) float *result) {
  float value = uniform->vector.x() + uniform->vector.yx().x() +
                push->vector.w() + uniform->vector[3] + uniform->matrix[0][0] +
                uniform->vector.length();
  push->target->x() = value;
  push->target->y() = value;
  (*push->target)[3] = value;
  push->target->set_swizzle<1, 2>(uniform->vector.xy());
  (*push->matrix_target)[1][2] = value;
  const __storage f32x4x4 *matrix_source = push->matrix_target;
  *result = value + push->target->zy().x() + (*matrix_source)[1][2];
}
