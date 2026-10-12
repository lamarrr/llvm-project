__fragment
void image_offsets(__binding(0, 0) texture_2d<float> image,
                   __binding(0, 1) sampler filter,
                   __output(0) float *target) {
  float a = image.sample_offset(filter, 0.5f, 0.5f, 1, -1);
  float b = image.sample_lod_offset(filter, 0.5f, 0.5f, 0.0f, 1, -1);
  float c = image.sample_grad_offset(filter, 0.5f, 0.5f, 0.1f, 0.0f, 0.0f, 0.1f,
                                     1, -1);
  float d = image.sample_bias_offset(filter, 0.5f, 0.5f, 0.0f, 1, -1);
  auto gathered = image.gather_offset(filter, 0.5f, 0.5f, 0u, 1, -1);
  *target = a + b + c + d + gathered[0];
}
