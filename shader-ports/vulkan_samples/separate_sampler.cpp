// Port of upstream/separate_image_sampler.frag; see upstream/LICENSE.
struct UV {
  float x, y;
};
struct Color {
  float r, g, b, a;
};

__fragment__
void vulkan_separate_sampler(__input__(0) UV uv,
                             __binding__(0, 1) texture_2d<Color> image,
                             __binding__(1, 0) sampler image_sampler,
                             __output__(0) Color *color) {
  *color = image.sample(image_sampler, uv.x, uv.y);
}
