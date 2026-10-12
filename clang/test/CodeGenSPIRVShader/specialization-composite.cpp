// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=specialization_composite -o %t.spv %t.bc

struct Output {
  __storage__ float *data;
};

__compute__
__workgroup_size__(1, 1, 1)
void specialization_composite(__push_constant__ const Output *output) {
  auto pair = specialization_composite2(specialization_constant(0, 1.0f),
                                        specialization_constant(1, 2.0f));
  output->data[0] = pair[0] + pair[1];
}
