// Selected fragment paths from tonemap.glsl; see upstream/LICENSE.txt and
// manifest.json.
#include "types.h"

struct alignas(16) TonemapParams {
  float bcs[3];
  unsigned flags;
  Float2 pixel_size;
  unsigned tonemapper, pad;
  unsigned glow_texture_size[2];
  float glow_intensity, glow_map_strength;
  unsigned glow_mode;
  float glow_levels[7];
  float exposure, white, auto_exposure_scale, luminance_multiplier;
};
static_assert(sizeof(TonemapParams) == 96);
static_assert(__builtin_offsetof(TonemapParams, exposure) == 80);

float filmic_curve(float color) {
  constexpr float bias = 2.0f;
  constexpr float a = 0.22f * bias * bias;
  constexpr float b = 0.30f * bias;
  constexpr float c = 0.10f, d = 0.20f, e = 0.01f, f = 0.30f;
  return ((color * (a * color + c * b) + d * e) /
          (color * (a * color + b) + d * f)) -
         e / f;
}

float tonemap_component(float color, float white) {
#ifdef TONEMAP_REINHARD
  color = max(0.0f, color);
  float white_squared = white * white;
  float white_squared_color = white_squared * color;
  return (white_squared_color + color * color) /
         (white_squared_color + white_squared);
#elif defined(TONEMAP_FILMIC)
  return filmic_curve(max(0.0f, color)) / filmic_curve(white);
#else
  return color;
#endif
}

float linear_to_srgb(float color) {
  color = clamp(color, 0.0f, 1.0f);
  return color < 0.0031308f
             ? 12.92f * color
             : (1.0f + 0.055f) * pow(color, 1.0f / 2.4f) - 0.055f;
}

__fragment
void tonemap(__binding(0, 0) texture_2d<Color> source,
             __binding(0, 1) sampler source_sampler, __input(0) Float2 uv,
             __output(0) Color *output,
             __push_constant const TonemapParams *params) {
  Color color = source.sample_lod(source_sampler, uv.x, uv.y, 0.0f);
  color.r = tonemap_component(
      color.r * params->luminance_multiplier * params->exposure, params->white);
  color.g = tonemap_component(
      color.g * params->luminance_multiplier * params->exposure, params->white);
  color.b = tonemap_component(
      color.b * params->luminance_multiplier * params->exposure, params->white);
  if (params->flags & (1u << 7)) {
    color.r = linear_to_srgb(color.r);
    color.g = linear_to_srgb(color.g);
    color.b = linear_to_srgb(color.b);
  }
  *output = color;
}
