struct Output {
  __storage__ unsigned *data;
};

__compute__
__workgroup_size__(1, 1, 1)
void image_sparse(__binding__(0, 0) texture_2d<float> image,
                  __push_constant__ const Output *output) {
  float value;
  unsigned residency = image.sparse_load(0, 0, 0u, &value);
  output->data[0] = sparse_resident(residency) ? 1u : 0u;
  output->data[1] = value > 0.0f ? 1u : 0u;
}
