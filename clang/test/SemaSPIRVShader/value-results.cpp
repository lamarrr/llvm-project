// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
struct texel { float r, g, b, a; };
void values(texture_2d<float> scalar, texture_1d<float[1]> single,
            texture_2d<texel> record,
            texture_2d<float[4]> array, sampler s,
            storage_texture_2d<float[4], rgba32f> target) {
  static_assert(__is_same(decltype(scalar.load(0, 0)), float));
  static_assert(__is_same(decltype(record.sample_lod(s, .5f, .5f, 0.0f)), texel));
  auto one = single.load(0);
  static_assert(__is_same(decltype(one.data), float[1]));
  auto rgba = array.load(0, 0);
  static_assert(__is_same(decltype(rgba.data), float[4]));
  static_assert(__is_trivially_copyable(decltype(rgba)));
  const auto frozen = rgba;
  static_assert(__is_same(decltype((frozen[0])), const float &));
  const float (&borrowed)[4] = frozen.data;
  auto dimensions = scalar.dimensions();
  static_assert(__is_same(decltype(dimensions.data), unsigned[2]));
  auto gathered = scalar.gather(s, .5f, .5f, 0);
  static_assert(__is_same(decltype(gathered.data), float[4]));
  target.store(0, 0, rgba[0], rgba[1], rgba[2], rgba[3]);
  float coordinates[2];
  scalar.load(rgba, coordinates); // expected-error {{must be scalars}}
  scalar.sample_lod(s, coordinates, 0.0f); // expected-error {{requires 4 scalar arguments}}
  scalar.sample_grad(s, 0.0f, 0.0f, coordinates, 0.0f, 0.0f, 0.0f); // expected-error {{must be scalars}}
  scalar.dimensions(dimensions, 0); // expected-error {{dimensions requires no arguments or one mip level}}
  target.store(0, 0, rgba); // expected-error {{requires 6 scalar arguments}}
}
void math() {
  const float a[3] = {1, 2, 3}, b[3] = {4, 5, 6};
  auto v = add(a, b);
  static_assert(__is_same(decltype(v.data), float[3]));
  auto chained = normalize(add(a, b));
  float component = cross(a, b)[0];
  float m[2][3], n[3][4];
  auto product = mul(m, n);
  static_assert(__is_same(decltype(product.data), float[2][4]));
  auto transposed = transpose(m);
  static_assert(__is_same(decltype(transposed.data), float[3][2]));
  auto mask = subgroup_ballot(true);
  static_assert(__is_same(decltype(mask.data), unsigned[4]));
  subgroup_ballot_bit_count(mask);
  subgroup_ballot(a); // expected-error {{requires a scalar condition}}
  normalize(v, a); // expected-error {{too many arguments}}
  cross(v, a, b); // expected-error {{too many arguments}}
  add(v, a, b); // expected-error {{too many arguments}}
  inverse(product, product); // expected-error {{too many arguments}}
}
