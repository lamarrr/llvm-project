// Fragment-only port of blur_raster.glsl; see upstream/LICENSE.txt and
// manifest.json.
#include "types.h"

struct alignas(16) BlurParams {
  Float2 pixel_size;
  unsigned flags, pad;
  float glow_strength, glow_bloom, glow_hdr_threshold, glow_hdr_scale;
  float glow_exposure, glow_white, glow_luminance_cap, glow_auto_exposure_scale;
  float luminance_multiplier, res1, res2, res3;
};
static_assert(sizeof(BlurParams) == 64);
static_assert(__builtin_offsetof(BlurParams, luminance_multiplier) == 48);

__fragment
void blur(__binding(0, 0) texture_2d<Color> source,
          __binding(0, 1) sampler source_sampler, __input(0) Float2 uv,
          __output(0) Color *output,
          __push_constant const BlurParams *params) {
#ifdef BLUR_MIPMAP
  Color a = source.sample(source_sampler, uv.x - 0.5f * params->pixel_size.x,
                          uv.y - 0.5f * params->pixel_size.y);
  Color b = source.sample(source_sampler, uv.x + 0.5f * params->pixel_size.x,
                          uv.y - 0.5f * params->pixel_size.y);
  Color c = source.sample(source_sampler, uv.x + 0.5f * params->pixel_size.x,
                          uv.y + 0.5f * params->pixel_size.y);
  Color d = source.sample(source_sampler, uv.x - 0.5f * params->pixel_size.x,
                          uv.y + 0.5f * params->pixel_size.y);
  Color total = add(add(add(a, b), c), d);
  Color quarter{0.25f, 0.25f, 0.25f, 0.25f};
  Color result = mul(total, quarter);
#else
  // Explicitly retain the original thirteen coordinates and weighting groups.
#define TAP(X, Y)                                                              \
  source.sample(source_sampler, uv.x + (X) * params->pixel_size.x,             \
                uv.y + (Y) * params->pixel_size.y)
  Color a = TAP(-1.0f, -1.0f);
  Color b = TAP(0.0f, -1.0f);
  Color c = TAP(1.0f, -1.0f);
  Color d = TAP(-0.5f, -0.5f);
  Color e = TAP(0.5f, -0.5f);
  Color f = TAP(-1.0f, 0.0f);
  Color g = source.sample(source_sampler, uv.x, uv.y);
  Color h = TAP(1.0f, 0.0f);
  Color i = TAP(-0.5f, 0.5f);
  Color j = TAP(0.5f, 0.5f);
  Color k = TAP(-1.0f, 1.0f);
  Color l = TAP(0.0f, 1.0f);
  Color m = TAP(1.0f, 1.0f);
#undef TAP
  Color base_weight{0.125f, 0.125f, 0.125f, 0.125f};
  Color lesser_weight{0.03125f, 0.03125f, 0.03125f, 0.03125f};
  Color result = mul(add(add(add(d, e), i), j), base_weight);
  result = add(result, mul(add(add(add(a, b), g), f), lesser_weight));
  result = add(result, mul(add(add(add(b, c), h), g), lesser_weight));
  result = add(result, mul(add(add(add(f, g), l), k), lesser_weight));
  result = add(result, mul(add(add(add(g, h), m), l), lesser_weight));
#endif
  *output = result;
}
