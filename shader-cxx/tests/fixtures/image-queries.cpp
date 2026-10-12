struct P {
  __storage float *data;
};
__fragment
void image_queries(
    __binding(0, 0) texture_2d<float> image,
    __binding(0, 1) texture_2d_ms<float> multisample,
    __binding(0, 2) storage_texture_2d_ms<float, r32f> writable,
    __binding(0, 3) sampler filtering, __push_constant const P *p) {
  auto lod = image.lod(filtering, .25f, .75f);
  p->data[0] = image.levels();
  p->data[1] = multisample.samples();
  p->data[2] = image.load(0, 1, 2);
  p->data[3] = multisample.load(0, 1, 2);
  writable.store(0, 1, 2, 3.0f);
  p->data[6] = writable.load(0, 1, 2);
  p->data[4] = image.sample_bias(filtering, .25f, .75f, 1.0f);
  p->data[5] = lod[0] + lod[1];
}
