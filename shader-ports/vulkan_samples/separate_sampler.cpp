// Port of upstream/separate_image_sampler.frag; see upstream/LICENSE.
struct UV {
  float x, y;
};
struct Color {
  float r, g, b, a;
};

__fragment
void vulkan_separate_sampler(__input(0) UV uv,
                             __binding(0, 1) texture_2d<Color> image,
                             __binding(1, 0) sampler image_sampler,
                             __output(0) Color *color) {
  *color = image.sample(image_sampler, uv.x, uv.y);
}
