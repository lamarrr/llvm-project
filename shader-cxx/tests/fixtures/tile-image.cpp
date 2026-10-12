struct TilePair {
  int first;
  int second;
};

__fragment
__noncoherent_color_attachment_read
__noncoherent_depth_attachment_read
__noncoherent_stencil_attachment_read
void tile_image_reads(tile_image<float, 0> color,
                      tile_image<TilePair, 1> signed_color,
                      tile_image<unsigned, 2> unsigned_color,
                      __output(0) float *output) {
  TilePair pair = signed_color.load(1u);
  *output = color.load() + static_cast<float>(pair.first) +
            static_cast<float>(unsigned_color.load()) +
            depth_attachment_read() +
            static_cast<float>(stencil_attachment_read(1u));
}
