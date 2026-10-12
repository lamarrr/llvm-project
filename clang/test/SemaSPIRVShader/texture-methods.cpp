// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader \
// RUN:   -I %S/../../lib/Headers -fsyntax-only -verify %s
#include <spirv/types.hpp>
using namespace spirv;
struct UV {
  float u, v;
};
struct BadUV {
  float u, v, w;
};
static_assert(__is_class(texture_2d<float>));
static_assert(!__is_pointer(texture_2d<float>));
static_assert(__is_same(decltype(min(1, 2)), int));
static_assert(__is_same(decltype(min(1u, 2u)), unsigned));
static_assert(__is_same(decltype(min(1, 2.0f)), float));
static_assert(__is_same(decltype(sqrt(4.0f)), float));
static_assert(__is_same(decltype(sqrt(4.0)), double));
static_assert(__is_same(decltype(sqrt(4)), double));
static_assert(__is_same(decltype(abs(true)), int));
static_assert(__is_same(decltype(min(short(1), short(2))), int));
struct Value {
  int value;
};
Value min(Value a, Value b) { return a.value < b.value ? a : b; }
static_assert(__is_same(decltype(min(Value{1}, Value{2})), Value));
template <class T> auto minimum(T a, T b) { return min(a, b); }
static_assert(__is_same(decltype(minimum(1, 2)), int));
void methods(const texture_2d<float> &texture, sampler sampler) {
  float value, uv[2];
  int xy[2];
  value = texture.load(0, 0);
  value = texture.sample(sampler, 0.0f, 0.0f);
  value = texture.sample(sampler, f32x2(0.5f, 0.5f));
  value = texture.sample(sampler, UV{0.5f, 0.5f});
  value = texture.sample_lod(sampler, 0.0f, 0.0f, 0.0f);
  texture.sample(
      sampler,
      BadUV{
          0.0f, 0.0f,
          // expected-error@+1 {{texture method sample requires 3 scalar arguments}}
          0.0f});
  texture.sample(
      sampler,
      // expected-error@+1 {{texture method sample requires 3 scalar arguments}}
      uv);
  texture.load(
      // expected-error@+1 {{texture method load requires 2 scalar arguments}}
      value);
  texture
      // expected-error@+1 {{texture method sample requires 3 scalar arguments}}
      .sample();
  // expected-error@+1 {{undeclared identifier 'texture_load'}}
  texture_load(value, texture, xy);
  // expected-error@+1 {{undeclared identifier 'vector_min'}}
  vector_min(value, value, value);
  // expected-error@+1 {{undeclared identifier 'matmul'}}
  matmul(value, value, value);
  // expected-error@+1 {{undeclared identifier 'matrix_mul'}}
  matrix_mul(value, value, value);
  // expected-error@+1 {{private member}}
  (void)texture.__handle;
}
void invalid_math() {
  int a[2], b[2], result[2];
  auto result_value = min(a, b);
  // expected-error@+1 {{math operands must be floating-point}}
  sin(a);
  // expected-error@+1 {{too few arguments}}
  sqrt();
  // expected-error@+1 {{too few arguments}}
  min(1);
}

// expected-note@* {{implicitly declared private here}}

// Former resource spellings are ordinary identifiers, not compiler types.
int Texture1D, Texture2D, Texture3D, TextureCube, Texture1DArray,
    Texture2DArray, TextureCubeArray, Texture2DMS, Texture2DMSArray,
    TextureBuffer, StorageTexture1D, StorageTexture2D, StorageTexture3D,
    StorageTexture1DArray, StorageTexture2DArray, StorageTexture2DMS,
    StorageTexture2DMSArray, StorageTextureBuffer, DescriptorArray, Sampler,
    SamplerComparison, AccelerationStructure, RayQuery;
