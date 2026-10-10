#include <types.hpp>

struct TransformRecord {
  float projection[16];
  float model[16];
  float texture[16];
  float radius[2];
  u32 sampler_index;
  u32 texture_index;
};

struct TransformParams {
  __storage__ const TransformRecord *records;
};

template <class Pointer> f32x4x4 load_matrix(Pointer values) {
  f32x4x4 result(0.0f);
  for (unsigned row = 0; row < 4; ++row)
    for (unsigned column = 0; column < 4; ++column)
      result[row][column] = (*values)[row * 4 + column];
  return result;
}

__vertex__
void pointer_link_and_loops(__push_constant__ const TransformParams *params,
                            __position__ float (*position)[4],
                            __output__(0) float (*uv)[2],
                            __flat__ __output__(1) u32 *record_index) {
  const auto *record = &params->records[instance_idx];
  f32x4 point(float(vertex_idx & 1), float((vertex_idx >> 1) & 1), 0.0f, 1.0f);
  f32x4 world = load_matrix(&record->model) * point;
  f32x4 clip = load_matrix(&record->projection) * world;
  f32x4 texcoord = load_matrix(&record->texture) * point;
  (*position)[0] = clip.x();
  (*position)[1] = clip.y();
  (*position)[2] = clip.z();
  (*position)[3] = clip.w();
  (*uv)[0] = texcoord.x();
  (*uv)[1] = texcoord.y();
  *record_index = instance_idx;
}

__fragment__
void pointer_link_and_loops_fragment(
    __push_constant__ const TransformParams *params,
    __binding__(0, 0) sampler samplers[],
    __binding__(0, 1) texture_2d<f32x4> textures[],
    __input__(0) const float (*uv)[2], __flat__ __input__(1) u32 record_index,
    __output__(0) float (*color)[4]) {
  const auto *record = &params->records[record_index];
  f32x2 coord((*uv)[0], (*uv)[1]);
  f32x4 sampled = textures[record->texture_index].sample(
      samplers[record->sampler_index], coord.x(), coord.y());
  (*color)[0] = sampled.x();
  (*color)[1] = sampled.y();
  (*color)[2] = sampled.z();
  (*color)[3] = sampled.w();
}
