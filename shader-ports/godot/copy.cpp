// Port of selected copy.glsl variants. See upstream/LICENSE.txt and
// manifest.json.
#include "types.h"

struct alignas(16) CopyParams {
  int section[4];
  int target[2];
  unsigned flags, pad;
  float glow_strength, glow_bloom, glow_hdr_threshold, glow_hdr_scale;
  float glow_exposure, glow_white, glow_luminance_cap, glow_auto_exposure_scale;
  float camera_z_far, camera_z_near;
  unsigned pad2[2];
  Color set_color;
};
static_assert(sizeof(CopyParams) == 96);
static_assert(__builtin_offsetof(CopyParams, camera_z_far) == 64);
static_assert(__builtin_offsetof(CopyParams, set_color) == 80);

#if defined(COPY_CUBE_ARRAY)
using Source = texture_cube_layers<Color>;
#elif defined(COPY_CUBE)
using Source = texture_cube<Color>;
#else
using Source = texture_2d<Color>;
#endif

#if defined(COPY_DEPTH)
using Destination = storage_texture_2d<float, r32f>;
#else
using Destination = storage_texture_2d<Color, rgba16f>;
#endif

__compute__
__workgroup_size__(8, 8, 1)
void copy(__binding__(0, 0) Source source,
          __binding__(0, 1) sampler source_sampler,
          __binding__(3, 0) Destination destination,
          __push_constant__ const CopyParams *params) {
  int x = static_cast<int>(global_invocation_id.x);
  int y = static_cast<int>(global_invocation_id.y);
  if (x >= params->section[2] || y >= params->section[3])
    return;

#if defined(COPY_DEPTH)
  float depth = source.load(x + params->section[0], y + params->section[1]).r;
  depth = depth * 2.0f - 1.0f;
  depth = 2.0f * params->camera_z_near * params->camera_z_far /
          (params->camera_z_far + params->camera_z_near -
           depth * (params->camera_z_far - params->camera_z_near));
  if (params->flags & (1u << 5))
    y = params->section[3] - y - 1;
  destination.store(x + params->target[0], y + params->target[1],
                    depth / params->camera_z_far);
#elif defined(COPY_CUBE) || defined(COPY_CUBE_ARRAY)
  constexpr float pi = 3.14159265359f;
  float u = static_cast<float>(x) / params->section[2];
  float v = static_cast<float>(y) / params->section[3];
  if (params->flags & (1u << 5))
    v = 1.0f - v;
  float phi = u * 2.0f * pi;
  float theta = v * pi;
  float nx = -sin(phi) * sin(theta);
  float ny = cos(theta);
  float nz = -cos(phi) * sin(theta);
#ifdef COPY_CUBE_ARRAY
  Color color =
      source.sample_lod(source_sampler, nx, ny, nz, params->camera_z_far, 0.0f);
#else
  Color color =
      source.sample_lod(source_sampler, nx, ny, nz, params->camera_z_far);
#endif
  destination.store(x + params->target[0], y + params->target[1], color.r,
                    color.g, color.b, color.a);
#else
  Color color;
  if (params->flags & (1u << 7)) {
    float u = static_cast<float>(x) / params->section[2];
    float v = static_cast<float>(y) / params->section[3];
    if (params->flags & (1u << 5))
      v = 1.0f - v;
    color = source.sample_lod(source_sampler, u, v, 0.0f);
  } else {
    color = source.load(x + params->section[0], y + params->section[1]);
    if (params->flags & (1u << 5))
      y = params->section[3] - y - 1;
  }
  if (params->flags & (1u << 6)) {
    float luminance = max(max(color.r, color.g), color.b);
    color.r = color.g = color.b = luminance;
  }
  if (params->flags & (1u << 8))
    color.a = 1.0f;
  destination.store(x + params->target[0], y + params->target[1], color.r,
                    color.g, color.b, color.a);
#endif
}
