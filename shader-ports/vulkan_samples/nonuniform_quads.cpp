// Port of upstream/nonuniform-quads.frag; see upstream/LICENSE.
struct UV {
  float x, y;
};
struct Color {
  float r, g, b, a;
};

__fragment
void vulkan_nonuniform_quads(__binding(0, 0) texture_2d<Color> textures[],
                             __binding(1, 0) sampler immutable_sampler,
                             __input(0) UV uv,
                             __input(1) __flat int texture_idx,
                             __output(0) Color *color) {
  *color =
      textures[nonuniform(texture_idx)].sample(immutable_sampler, uv.x, uv.y);
}
