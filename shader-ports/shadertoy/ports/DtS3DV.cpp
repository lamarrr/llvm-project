// Port of https://www.shadertoy.com/view/DtS3DV; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtS3DV.json.
#include "../support.h"
using namespace glsl;
struct Texture2D {
  texture_2d<f32x4> image;
  sampler sampling;
};
f32x4 texture(Texture2D t, f32x2 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1]);
}
f32x4 textureLod(Texture2D t, f32x2 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], lod);
}
i32x2 textureSize(Texture2D t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x2(size[0], size[1]);
}
f32x4 texelFetch(Texture2D t, i32x2 uv) {
  return t.image.load(uv.data[0], uv.data[1]);
}
struct Texture3D {
  texture_3d<f32x4> image;
  sampler sampling;
};
f32x4 texture(Texture3D t, f32x3 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1], uv.data[2]);
}
f32x4 textureLod(Texture3D t, f32x3 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], uv.data[2],
                            lod);
}
i32x3 textureSize(Texture3D t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x3(size[0], size[1], size[2]);
}
f32x4 texelFetch(Texture3D t, i32x3 uv) {
  return t.image.load(uv.data[0], uv.data[1], uv.data[2]);
}
struct TextureCube {
  texture_cube<f32x4> image;
  sampler sampling;
};
f32x4 texture(TextureCube t, f32x3 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1], uv.data[2]);
}
f32x4 textureLod(TextureCube t, f32x3 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], uv.data[2],
                            lod);
}
i32x2 textureSize(TextureCube t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x2(size[0], size[1]);
}
using sampler2D = Texture2D;
using sampler3D = Texture3D;
using samplerCube = TextureCube;
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

float f_n_rand(Context &__ctx, float n) {
  n = fract(n * .1031f);
  n *= n + 33.33f;
  n *= n + n;
  return fract(n);
}
f32x2 f_o_idx(Context &__ctx, f32x2 o_trn, f32x2 o_scl) {
  float n_idx_flr =
      floor(o_trn.data[0]) + floor(o_trn.data[1]) * floor(o_scl.data[0]);
  return f32x2(n_idx_flr,
               n_idx_flr / (floor(o_scl.data[0]) * floor(o_scl.data[1])));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float n_dscl = 1.f;
  f32x2 o_lires = iResolution.swizzle<0, 1>() * n_dscl;
  f32x2 o_lfc = fragCoord.swizzle<0, 1>() * n_dscl;
  float n_t = iTime * 0.5f;
  float n_ratio_1to_iresx = 1.f / o_lires.data[0];
  f32x2 o_lfc_nor = floor(o_lfc) / o_lires;
  f32x2 o_lfc_nor_off =
      (floor(o_lfc) - 0.5f * o_lires.swizzle<0, 1>()) / o_lires.data[1];
  f32x2 o_fc_nor = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 o_fc_nor_off =
      (fragCoord.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>()) /
      iResolution.data[1];
  f32x2 o_mo_nor_off =
      (iMouse.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>()) /
      iResolution.data[1];
  if (iMouse.data[2] == 0.f) {
    o_mo_nor_off = f32x2(sin(n_t), cos(n_t) * 0.5f + 0.5f);
  }
  o_lfc *= (o_mo_nor_off.data[1]) * 0.3f;
  f32x2 o_scl_krn = f32x2(5.f);
  float n_krn_xty = o_scl_krn.data[0] * o_scl_krn.data[1];
  float n_dmin = 1.f;
  f32x2 o_lfc_min, o_idx_min;
  float n_rnd_min;
  float n_rnd_amp = floor(min(o_scl_krn.data[0], o_scl_krn.data[1]) / 2.f);
  for (float n = 0.f; n < n_krn_xty; n += 1.f) {
    float n_x =
        floor(mod(n, o_scl_krn.data[0]) - floor(o_scl_krn.data[0] / 2.f));
    float n_y = floor(n / o_scl_krn.data[0]) - floor(o_scl_krn.data[1] / 2.f);
    f32x2 o_lfc_krn = o_lfc + f32x2(n_x, n_y);
    f32x2 o_idx = f_o_idx(__ctx, floor(o_lfc_krn), floor(o_lires));
    float n_rand = f_n_rand(__ctx, floor(o_idx.data[0]));
    f32x2 o_a = fract(o_lfc);
    f32x2 o_b = f32x2(n_x, n_y) + (n_rand - 0.5f) * n_rnd_amp;
    float n_dist = length(o_a - o_b) * .5f;
    n_dist *= 1.f + n_rand;
    if (n_dist < n_dmin) {
      n_dmin = n_dist;
      o_lfc_min = o_lfc_krn;
      o_idx_min = o_idx;
      n_rnd_min = n_rand;
    }
  }
  fragColor = f32x4(n_dmin);
  fragColor = sqrt(fragColor);
  if (o_fc_nor_off.data[0] > o_mo_nor_off.data[0]) {
    float n_rnd1 = f_n_rand(__ctx, n_rnd_min * 0.3111f);
    float n_rnd2 = f_n_rand(__ctx, n_rnd1 * 0.1113f);
    fragColor = f32x4(n_rnd_min, n_rnd1, n_rnd2, 1.f);
    fragColor *= 1.f - (float(n_rnd1 > 0.5f) * n_dmin);
  }
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
  iResolution = f32x3(p->resolution_time.data[0], p->resolution_time.data[1],
                      p->resolution_time.data[2]);
  gl_FragCoord =
      f32x4(frag_coord[0], frag_coord[1], frag_coord[2], frag_coord[3]);
  iTime = p->resolution_time.data[3];
  iMouse = f32x4(p->mouse.data[0], p->mouse.data[1], p->mouse.data[2],
                 p->mouse.data[3]);
  iDate =
      f32x4(p->date.data[0], p->date.data[1], p->date.data[2], p->date.data[3]);
  iTimeDelta = p->time_delta;
  iFrameRate = p->frame_rate;
  iFrame = p->frame;
  iSampleRate = p->sample_rate;
  for (unsigned i = 0; i < 4; ++i) {
    iChannelTime[i] = p->channel_time[i];
    iChannelResolution[i] = f32x3(p->channel_resolution[i].data[0],
                                  p->channel_resolution[i].data[1],
                                  p->channel_resolution[i].data[2]);
  }
  Context __ctx{};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
