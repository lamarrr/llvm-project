// Port of upstream/nonuniform-quads.frag; see upstream/LICENSE.
struct UV {
  float x, y;
};
struct Color {
  float r, g, b, a;
};

__fragment__
void vulkan_nonuniform_quads(__binding__(0, 0) texture_2d<Color> textures[],
                             __binding__(1, 0) sampler immutable_sampler,
                             __input__(0) UV uv,
                             __input__(1) __flat__ int texture_idx,
                             __output__(0) Color *color) {
  *color =
      textures[nonuniform(texture_idx)].sample(immutable_sampler, uv.x, uv.y);
}
