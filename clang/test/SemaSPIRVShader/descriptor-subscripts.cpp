// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
using textures = texture_2d<float>[4];
using samplers = sampler[];
enum index { first = 0 };
enum class scoped_index { first = 0 };
textures &images();
samplers &filters();
static_assert(__is_array(textures));
static_assert(sizeof(textures) == 4 * sizeof(texture_2d<float>));
static_assert(__is_same(decltype(images()[0]), texture_2d<float>));
static_assert(__is_same(decltype(filters()[0]), sampler));
static_assert(__is_same(decltype(0 [images()]), texture_2d<float>));
template <class T, unsigned long N>
constexpr unsigned long extent(const T (&)[N]) {
  return N;
}
static_assert(__is_same(decltype(extent(images())), unsigned long));
template <class A, class I> auto lookup(A array, I index) {
  return array[index];
}
static_assert(__is_same(decltype(lookup(images(), 0)), texture_2d<float>));
void access(const textures &images, samplers &filters,
            unsigned long long wide) {
  auto a = images[first];
  auto b = images[wide];
  auto c = images[true];
  auto d = filters[0];
  // expected-error@+1 {{subscript must have an integral type}}
  (void)images[1.0f];
  (void)images[scoped_index::
                   // expected-error@+1 {{subscript must have an integral type}}
               first];
  // expected-error@+1 {{expression is not assignable}}
  filters[0] = d;
  // expected-error@+1 {{undeclared identifier 'descriptor_at'}}
  descriptor_at(images, 0);
  // expected-error@+1 {{undeclared identifier 'descriptor_subscript'}}
  descriptor_subscript(images, 0);
}
void invalid_elements(ray_query queries[4], textures nested[4]) {
  (void)
      // expected-error@+1 {{must contain native descriptor resources}}
      queries[0];
  (void)
      // expected-error@+1 {{must contain native descriptor resources}}
      nested[0];
}
// expected-error@+1 {{no template named 'descriptor_array'}}
descriptor_array<texture_2d<float>, 4> old;
