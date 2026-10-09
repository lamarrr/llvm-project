// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
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
struct Value { int value; };
Value min(Value a, Value b) { return a.value < b.value ? a : b; }
static_assert(__is_same(decltype(min(Value{1}, Value{2})), Value));
template <class T> auto minimum(T a, T b) { return min(a, b); }
static_assert(__is_same(decltype(minimum(1, 2)), int));
void methods(const texture_2d<float> &texture, sampler sampler) {
  float value, uv[2];
  int xy[2];
  value = texture.load(0, 0);
  value = texture.sample(sampler, 0.0f, 0.0f);
  value = texture.sample_lod(sampler, 0.0f, 0.0f, 0.0f);
  texture.load(value); // expected-error {{texture method load requires 2 scalar arguments}}
  texture.sample(); // expected-error {{texture method sample requires 3 scalar arguments}}
  texture_load(value, texture, xy); // expected-error {{undeclared identifier 'texture_load'}}
  vector_min(value, value, value); // expected-error {{undeclared identifier 'vector_min'}}
  matmul(value, value, value); // expected-error {{undeclared identifier 'matmul'}}
  matrix_mul(value, value, value); // expected-error {{undeclared identifier 'matrix_mul'}}
  (void)texture.__handle; // expected-error {{private member}}
}
void invalid_math() {
  int a[2], b[2], result[2];
  auto result_value = min(a, b);
  sin(a); // expected-error {{math operands must be floating-point}}
  sqrt(); // expected-error {{too few arguments}}
  min(1); // expected-error {{too few arguments}}
}

// expected-note@* {{implicitly declared private here}}

// Former resource spellings are ordinary identifiers, not compiler types.
int Texture1D, Texture2D, Texture3D, TextureCube, Texture1DArray, Texture2DArray,
    TextureCubeArray, Texture2DMS, Texture2DMSArray, TextureBuffer,
    StorageTexture1D, StorageTexture2D, StorageTexture3D, StorageTexture1DArray,
    StorageTexture2DArray, StorageTexture2DMS, StorageTexture2DMSArray,
    StorageTextureBuffer, DescriptorArray, Sampler, SamplerComparison,
    AccelerationStructure, RayQuery;
