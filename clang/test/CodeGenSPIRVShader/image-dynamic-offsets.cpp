// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=dynamic_offsets -o %t.spv %t.bc

__fragment
void dynamic_offsets(__binding(0, 0) texture_2d<float> image,
                     __binding(0, 1) depth_texture_2d<float> depth,
                     __binding(0, 2) sampler filtering,
                     __binding(0, 3) sampler_comparison comparing,
                     __input(0) __flat int x_offset,
                     __input(1) __flat int y_offset,
                     __output(0) float *result) {
  float sampled = image.sample_offset(filtering, 0.5f, 0.5f, 1, -1);
  auto gathered =
      image.gather_offset(filtering, 0.5f, 0.5f, 0u, x_offset, y_offset);
  auto compared = depth.gather_compare_offset(comparing, 0.5f, 0.5f, 0.75f,
                                              x_offset, y_offset);
  *result = sampled + gathered[0] + compared[0];
}
