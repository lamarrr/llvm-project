// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -fsyntax-only -verify %s

void invalid(storage_texture_2d<int, r32i> signed_image,
             storage_texture_2d<float, r64ui> wrong_wide,
             texture_2d<int> integer_texture) {
  static_assert(__is_same(decltype(signed_image.atomic_add(0, 0, 1)), int));
  // expected-error@+1 {{image atomics require scalar R32i, R32ui, R32f, R64i or R64ui storage texels}}
  wrong_wide.atomic_add(0, 0, 1.0f);
  // expected-error@+1 {{matching R64 storage texels}}
  wrong_wide.load(0, 0);
  float texel;
  // expected-error@+1 {{sparse_load requires texture_2d<float>}}
  integer_texture.sparse_load(0, 0, 0u, &texel);
}
