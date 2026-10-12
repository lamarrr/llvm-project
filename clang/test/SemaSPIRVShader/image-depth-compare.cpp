// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -fsyntax-only -verify %s

// expected-no-diagnostics
__fragment
void compare(__binding(0, 0) depth_texture_2d<float> depth,
             __binding(0, 1) sampler_comparison filter,
             __output(0) float *result) {
  *result = depth.sample_compare(filter, 0.5f, 0.25f, 0.75f);
}
