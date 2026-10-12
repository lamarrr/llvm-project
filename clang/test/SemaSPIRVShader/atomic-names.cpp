// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
// expected-no-diagnostics

void matching_names(__storage unsigned *pointer,
                    storage_texture_2d<unsigned, r32ui> image) {
  static_assert(__is_same(decltype(atomic_load(pointer)), unsigned));
  static_assert(__is_same(decltype(image.atomic_load(0, 0)), unsigned));
  static_assert(__is_same(decltype(atomic_store(pointer, 1u)), void));
  static_assert(__is_same(decltype(image.atomic_store(0, 0, 1u)), void));
#define CHECK_OPERATION(name)                                                   \
  static_assert(__is_same(decltype(name(pointer, 1u)), unsigned));                \
  static_assert(__is_same(decltype(image.name(0, 0, 1u)), unsigned))
  CHECK_OPERATION(atomic_add);
  CHECK_OPERATION(atomic_sub);
  CHECK_OPERATION(atomic_exchange);
  CHECK_OPERATION(atomic_min);
  CHECK_OPERATION(atomic_max);
  CHECK_OPERATION(atomic_and);
  CHECK_OPERATION(atomic_or);
  CHECK_OPERATION(atomic_xor);
#undef CHECK_OPERATION
  static_assert(__is_same(decltype(atomic_compare_exchange(pointer, 1u, 2u)),
                         unsigned));
  static_assert(__is_same(decltype(image.atomic_compare_exchange(0, 0, 1u, 2u)),
                         unsigned));
}
