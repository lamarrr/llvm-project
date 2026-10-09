// Port of https://www.shadertoy.com/view/dtlGDn; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtlGDn.json.
#include "../../support.h"
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

float f_n_rand(Context &__ctx, float p) {
  p = fract(p * .1031f);
  p *= p + 33.33f;
  p *= p + p;
  return fract(p);
}
f32x4 f_o_index2d_by_o_fc_o_ires(Context &__ctx, f32x2 o_fc, f32x2 o_ires) {
  float n_index_x =
      floor(o_fc.data[0]) + floor(o_fc.data[1]) * (o_ires.data[0]);
  float n_index_x_nor =
      (floor(n_index_x) / (floor(o_ires.data[0]) * floor(o_ires.data[1])));
  float n_index_y =
      floor(o_fc.data[1]) + floor(o_fc.data[0]) * (o_ires.data[1]);
  float n_index_y_nor =
      (floor(n_index_y) / (floor(o_ires.data[1]) * floor(o_ires.data[0])));
  return f32x4(n_index_x, n_index_x_nor, n_index_y, n_index_y_nor);
}
float f_n_rand_interpolated(Context &__ctx, f32x2 o_fc, f32x2 o_ires) {
  f32x4 o_index_xp0yp0 =
      f_o_index2d_by_o_fc_o_ires(__ctx, o_fc + f32x2(0.f), o_ires);
  f32x4 o_index_xp1yp0 =
      f_o_index2d_by_o_fc_o_ires(__ctx, o_fc + f32x2(+1.f, +0.f), o_ires);
  f32x4 o_index_xp0yp1 =
      f_o_index2d_by_o_fc_o_ires(__ctx, o_fc + f32x2(+0.f, +1.f), o_ires);
  f32x4 o_index_xp1yp1 =
      f_o_index2d_by_o_fc_o_ires(__ctx, o_fc + f32x2(+1, +1.f), o_ires);
  float n_rand_xp0yp0 = f_n_rand(__ctx, floor(o_index_xp0yp0[0]));
  float n_rand_xp1yp0 = f_n_rand(__ctx, floor(o_index_xp1yp0[0]));
  float n_rand_xp0yp1 = f_n_rand(__ctx, floor(o_index_xp0yp1[0]));
  float n_rand_xp1yp1 = f_n_rand(__ctx, floor(o_index_xp1yp1[0]));
  float n = mix(n_rand_xp0yp0, n_rand_xp1yp0, fract(o_fc.data[0]));
  float n2 = mix(n_rand_xp0yp1, n_rand_xp1yp1, fract(o_fc.data[0]));
  float n3 = mix(n, n2, fract(o_fc.data[1]));
  return (n3);
}
f32x4 f_o_smoothnoise(Context &__ctx, f32x2 o_fc, f32x2 o_ires) {
  float n_product = 1.f;
  float n_sum = 0.f;
  float n_sum_noamp = 0.f;
  float n_its = 5.f;
  float n_amp = 1.f;
  float n_freq = 1.f;
  float n_min = 1.f;
  float n_max = 0.f;
  for (float n_it = 0.f; n_it < n_its; n_it += 1.f) {
    n_amp = n_amp / 2.f;
    n_freq = n_freq * 2.f;
    float n_rand_interpolated =
        f_n_rand_interpolated(__ctx, o_fc * n_freq, o_ires * n_freq);
    n_sum += n_rand_interpolated * n_amp;
    n_sum_noamp += n_rand_interpolated;
    n_min = min(n_min, n_rand_interpolated);
    n_max = max(n_max, n_rand_interpolated);
  }
  float n_sum_noamp_nor = n_sum_noamp / n_its;
  return f32x4(n_sum, n_sum_noamp_nor, n_min, n_max);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 o_1fc_nor = 1.f / iResolution.swizzle<0, 1>();
  f32x2 o_fc_nor = o_1fc_nor * fragCoord.swizzle<0, 1>();
  f32x2 o_fc_nor_offset = o_fc_nor - 0.5f;
  float n_y = o_fc_nor_offset.data[1];
  float n_rand_x = f_n_rand(__ctx, (o_fc_nor).data[0]);
  float n_tau = 6.2831f;
  float n =
      sin(o_fc_nor.data[0] * n_tau * sin(o_fc_nor.data[0] * n_tau) * 10.f);
  float n_amp = 0.3f + n * 0.1f;
  float n_freq = o_fc_nor_offset.data[0] * 20.f;
  n_y = n_y + sin(n_freq) * n_amp;
  float n_dist = length(n_y);
  n_dist = n_dist * 1.f / 0.01f;
  f32x2 o_m_nor = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  if (iMouse.data[2] == 0.f) {
    o_m_nor = f32x2((sin(iTime) * 0.5f + 0.5f), 0.f);
  }
  float n_factor = 0.02f;
  f32x4 o_smoothnoise =
      f_o_smoothnoise(__ctx, fragCoord.swizzle<0, 1>() * n_factor,
                      iResolution.swizzle<0, 1>() * n_factor);
  float n_index = floor(o_fc_nor.data[1] / (1.f / 4.f));
  fragColor = f32x4(o_smoothnoise[int(n_index)]);
  if (o_fc_nor.data[0] > o_m_nor.data[0]) {
    fragColor = f32x4(o_smoothnoise.data[1]);
  }
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output) {
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
