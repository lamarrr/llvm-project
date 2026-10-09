template <class Image> auto load_value(const Image &image) {
  return image.load(0, 0);
}
float sum(const float (&v)[4]) { return v[0] + v[1] + v[2] + v[3]; }
__compute__
__workgroup_size__(1, 1, 1)
void value_results(
    __binding__(0, 0) texture_2d<float[4]> image, __binding__(0, 1) sampler s,
    __binding__(0, 2) storage_texture_2d<float[4], rgba32f> target) {
  auto value = load_value(image);
  auto sampled = image.sample_lod(s, .5f, .5f, 0.0f);
  const auto dimensions = image.dimensions(0);
  float a[3] = {1, 2, 3}, b[3] = {4, 5, 6};
  auto chained = normalize(add(a, b));
  float matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
  const auto transposed = transpose(matrix);
  float contribution = sum(value.data) + chained[0] + cross(a, b)[0] +
                       transposed[0][1] + dimensions[0] + sampled[0];
  target.store(0, 0, contribution, value[1], value[2], value[3]);
}
