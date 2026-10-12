template <class Array, class Index> auto lookup(Array resources, Index index) {
  return resources[index];
}
struct P {
  __storage float *data;
};
__compute
__workgroup_size(1, 1, 1)
void sample_descriptors(
    __binding(0, 0) texture_2d<float> textures[], __binding(0, 1) sampler s,
    __binding(0, 2) texture_2d<float> fixed[4],
    __binding(0, 3) sampler samplers[4],
    __binding(0, 4) storage_texture_2d<float, r32f> targets[4],
    __push_constant const P *p) {
  float value;
  auto t = textures[global_invocation_id.x];
  value = t.sample_lod(s, 0.0f, 0.0f, 0.0f);
  value = textures[nonuniform(global_invocation_id.x)].load(0, 0);
  value = lookup(fixed, 1).sample_lod(2 [samplers], 0.0f, 0.0f, 0.0f);
  value =
      textures[static_cast<unsigned long long>(global_invocation_id.x)].load(0,
                                                                             0);
  targets[0].store(0, 0, value);
  *p->data = value;
}
