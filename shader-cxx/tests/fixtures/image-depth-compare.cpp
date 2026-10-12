__fragment
void image_depth_compare(__binding(0, 0) depth_texture_2d<float> depth,
                         __binding(0, 1) sampler_comparison filtering,
                         __input(0) __flat int offset_x,
                         __output(0) float *color) {
  float implicit = depth.sample_compare(filtering, 0.5f, 0.5f, 0.75f);
  float explicit_lod =
      depth.sample_compare_lod(filtering, 0.5f, 0.5f, 0.75f, 0.0f);
  float constant_offset =
      depth.sample_compare_offset(filtering, 0.5f, 0.5f, 0.75f, 1, 0);
  float lod_offset =
      depth.sample_compare_lod_offset(filtering, 0.5f, 0.5f, 0.75f, 0.0f, 1, 0);
  auto gathered = depth.gather_compare(filtering, 0.5f, 0.5f, 0.75f);
  auto gathered_offset =
      depth.gather_compare_offset(filtering, 0.5f, 0.5f, 0.75f, offset_x, 0);
  *color = implicit + explicit_lod + constant_offset + lod_offset +
           gathered[0] + gathered_offset[0];
}
