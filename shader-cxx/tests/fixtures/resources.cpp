template <class Image> float load_image(const Image &image) {
  return image.load(0, 0);
}
texture_2d<float> copy_image(texture_2d<float> image) { return image; }
struct Texel {
  float r, g, b, a;
};
__compute__
__workgroup_size__(1, 1, 1)
void resources(
    __binding__(0, 0) texture_2d<float> t, __binding__(0, 1) sampler s,
    __binding__(0, 2) storage_texture_2d<float, r32f> target,
    __binding__(0, 3) texture_cube<Texel> cube,
    __binding__(0, 4) texture_cube_layers<float> cubeArray,
    __binding__(0, 5) sampler_comparison comparison,
    __binding__(0, 6) storage_texture_2d<unsigned, r32ui> unsignedTarget,
    __binding__(0, 7) storage_texture_2d<int, r32i> signedTarget) {
  float value;
  value = t.sample_lod(s, .5f, .5f, 0.0f);
  auto copy = copy_image(t);
  const auto frozen = copy;
  value = load_image(frozen);
  auto *pointer = &copy;
  value = pointer->load(0, 0);

  target.store(0, 0, value);
  value = target.load(0, 0);
  Texel texel = cube.sample_lod(s, 0.0f, 0.0f, 1.0f, 0.0f);

  value = cubeArray.sample_lod(s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  unsigned u = 1;
  int i = -1;
  unsignedTarget.store(0, 0, u);
  signedTarget.store(0, 0, i);
  i = signedTarget.load(0, 0);
  u = unsignedTarget.load(0, 0);
  target.store(0, 0, static_cast<float>(i) + static_cast<float>(u));
}
