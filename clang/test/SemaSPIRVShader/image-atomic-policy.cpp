// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

struct Int2 {
  int x, y;
};
struct Int3 {
  int x, y, z;
};
struct Float2 {
  float x, y;
};
struct Wide2 {
  long long x, y;
};
struct Bool2 {
  bool x, y;
};

void vector_coordinates(storage_texture_2d<unsigned, r32ui> image,
                        storage_texture_2d_ms<unsigned, r32ui> multisample) {
  image.atomic_sub(Int2{1, 2}, 1u);
  multisample.atomic_sub(Int2{1, 2}, 0u, 1u);
  // expected-error@+1 {{image atomic coordinate records require matching 32-bit integer components}}
  image.atomic_sub(Int3{1, 2, 3}, 1u);
  // expected-error@+1 {{image atomic coordinate records require matching 32-bit integer components}}
  image.atomic_sub(Float2{1, 2}, 1u);
  // expected-error@+1 {{image atomic coordinate records require matching 32-bit integer components}}
  image.atomic_sub(Wide2{1, 2}, 1u);
  // expected-error@+1 {{image atomic coordinate records require matching 32-bit integer components}}
  image.atomic_sub(Bool2{true, false}, 1u);
  // expected-error@+1 {{image atomics require coordinates, a multisample selector when applicable}}
  multisample.atomic_sub(Int2{1, 2}, 1u);
  // expected-error@+1 {{image atomic coordinates and samples must be integers}}
  multisample.atomic_sub(Int2{1, 2}, 0.5f, 1u);
  // expected-error@+1 {{memory order is invalid for this atomic operation}}
  image.atomic_load(Int2{1, 2}, memory_order::release, memory_scope::device);
}

void invalid(storage_texture_2d<unsigned, r32ui> image,
             storage_texture_2d<float, r32f> floating,
             storage_texture_2d_ms<unsigned, r32ui> multisample) {
  // expected-error@+1 {{memory order is invalid for this atomic operation}}
  image.atomic_load(0, 0, memory_order::release, memory_scope::device);
  // expected-error@+1 {{memory order is invalid for this atomic operation}}
  image.atomic_store(0, 0, 1u, memory_order::acquire, memory_scope::device);
  // expected-error@+2 {{compare-exchange failure order cannot release or be stronger than success order}}
  image.atomic_compare_exchange(0, 0, 1u, 2u, memory_order::relaxed,
                                memory_order::acquire, memory_scope::device);
  // expected-error@+1 {{floating image atomics support load, store, exchange, add, sub, min and max}}
  floating.atomic_and(0, 0, 1.0f);
  // expected-error@+1 {{image atomics require coordinates, a multisample selector when applicable}}
  multisample.atomic_add(0, 0, 1u);
  // expected-error@+1 {{image atomic coordinates and samples must be integers}}
  image.atomic_add(0.5f, 0, 1u);
}

struct IntTexel2 {
  int x, y;
};

void invalid_formats(texture_2d<int> sampled,
                     storage_texture_2d<int, r32ui> signed_unsigned_format,
                     storage_texture_2d<unsigned, r32i> unsigned_signed_format,
                     storage_texture_2d<IntTexel2, rg32i> vector_texel,
                     storage_texture_2d<long long, r32i> wide_narrow_format,
                     storage_texture_2d<float, r32f> floating) {
  // expected-error@+1 {{image atomics require scalar R32i, R32ui, R32f, R64i or R64ui storage texels}}
  sampled.atomic_add(0, 0, 1);
  // expected-error@+1 {{image atomics require scalar R32i, R32ui, R32f, R64i or R64ui storage texels}}
  signed_unsigned_format.atomic_add(0, 0, 1);
  // expected-error@+1 {{image atomics require scalar R32i, R32ui, R32f, R64i or R64ui storage texels}}
  unsigned_signed_format.atomic_add(0, 0, 1u);
  // expected-error@+1 {{image atomics require scalar R32i, R32ui, R32f, R64i or R64ui storage texels}}
  vector_texel.atomic_add(0, 0, IntTexel2{1, 2});
  // expected-error@+1 {{image atomics require scalar R32i, R32ui, R32f, R64i or R64ui storage texels}}
  wide_narrow_format.atomic_add(0, 0, 1ll);
  // expected-error@+1 {{floating image atomics support load, store, exchange, add, sub, min and max}}
  floating.atomic_compare_exchange(0, 0, 0.0f, 1.0f);
}

void expected_types(storage_texture_2d<int, r32i> signed32,
                    storage_texture_2d<unsigned, r32ui> unsigned32,
                    storage_texture_2d<long long, r64i> signed64,
                    storage_texture_2d<unsigned long long, r64ui> unsigned64,
                    storage_texture_2d<float, r32f> floating) {
  static_assert(__is_same(decltype(signed32.atomic_load(0, 0)), int));
  static_assert(__is_same(decltype(unsigned32.atomic_load(0, 0)), unsigned));
  static_assert(__is_same(decltype(signed64.atomic_load(0, 0)), long long));
  static_assert(
      __is_same(decltype(unsigned64.atomic_load(0, 0)), unsigned long long));
  static_assert(__is_same(decltype(floating.atomic_load(0, 0)), float));
  static_assert(__is_same(decltype(signed32.atomic_store(0, 0, -1)), void));
}
