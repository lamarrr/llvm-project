// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -fsyntax-only -verify %s

// expected-no-diagnostics
__fragment__
void compare(__binding__(0, 0) depth_texture_2d<float> depth,
             __binding__(0, 1) sampler_comparison filter,
             __output__(0) float *result) {
  *result = depth.sample_compare(filter, 0.5f, 0.25f, 0.75f);
}
