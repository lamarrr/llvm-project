// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=depth_compare -o %t.spv %t.bc

__fragment
void depth_compare(__binding(0, 0) depth_texture_2d<float> depth,
                   __binding(0, 1) sampler_comparison compare,
                   __binding(0, 2) depth_texture_cube<float> cube,
                   __binding(0, 3) depth_texture_2d_layers<float> layers,
                   __binding(0, 4) depth_texture_cube_layers<float> cube_layers,
                   __output(0) float *result) {
  float implicit = depth.sample_compare(compare, 0.5f, 0.25f, 0.75f);
  float explicit_lod =
      depth.sample_compare_lod(compare, 0.5f, 0.25f, 0.75f, 0.0f);
  float offset =
      depth.sample_compare_offset(compare, 0.5f, 0.25f, 0.75f, 1, -1);
  auto gather = depth.gather_compare(compare, 0.5f, 0.25f, 0.75f);
  auto gather_offset =
      depth.gather_compare_offset(compare, 0.5f, 0.25f, 0.75f, 1, -1);
  float cube_value = cube.sample_compare(compare, 1.0f, 0.0f, 0.0f, 0.75f);
  float layer_value = layers.sample_compare(compare, 0.5f, 0.25f, 0.0f, 0.75f);
  float cube_layer_value =
      cube_layers.sample_compare(compare, 1.0f, 0.0f, 0.0f, 0.0f, 0.75f);
  *result = implicit + explicit_lod + offset + gather[0] + gather_offset[0] +
            cube_value + layer_value + cube_layer_value;
}
