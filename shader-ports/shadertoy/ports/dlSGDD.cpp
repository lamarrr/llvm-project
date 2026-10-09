// Port of https://www.shadertoy.com/view/dlSGDD; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlSGDD.json.
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
  n = n + 2.213f * n;
  n = n * n;
  n = sin(11235.f * n + n * 3.13f) * 1.3425f;
  return fract(n);
}
f32x2 f_o_rotd(Context &__ctx, f32x2 o, float n_rot_nor) {
  float n_radians = 6.283158f * n_rot_nor;
  float n_cos = cos(n_radians);
  float n_sin = sin(n_radians);
  return f32x2(n_cos * o.data[0] - n_sin * o.data[1],
               n_sin * o.data[0] + n_cos * o.data[1]);
}
float f_n_dist_segment(Context &__ctx, f32x2 o, float n_ang_nor,
                       float n_radius_start, float n_radius_end) {
  float n_tau = 6.28318530f;
  float n_ang_nor_current =
      fract(((atan(o.data[1], o.data[0]) + (n_tau / 2.f)) / n_tau) + 0.5f);
  float n_an = min(n_ang_nor, n_ang_nor_current);
  n_an = n_ang_nor_current;
  float n_len = length(o);
  float n_radius = 0.5f;
  n_radius = max(n_len, n_radius_start);
  n_radius = min(n_radius, n_radius_end);
  f32x2 o_p = f32x2(cos(n_an * n_tau) * n_radius, sin(n_an * n_tau) * n_radius);
  f32x2 o_pstart =
      f32x2(cos(0.f * n_tau) * n_radius, sin(0.f * n_tau) * n_radius);
  f32x2 o_pend = f32x2(cos(n_ang_nor * n_tau) * n_radius,
                       sin(n_ang_nor * n_tau) * n_radius);
  float n_len_start = length(o_pstart - o);
  float n_len_end = length(o_pend - o);
  float n_dist = length(o_p - o);
  if (n_ang_nor_current > n_ang_nor) {
    return min(n_len_start, n_len_end);
  }
  return n_dist;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float n_ratio_1to_iresx = 1.f / iResolution.data[0];
  f32x2 o_trn_fc_nor = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 o_trn_mo_nor = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float n_ratio_x_to_y = iResolution.data[0] / iResolution.data[1];
  f32x2 o_trn_fc_nor_offset =
      (o_trn_fc_nor - f32x2(0.5f)) * f32x2(n_ratio_x_to_y, 1.f);
  f32x2 o_trn_mo_nor_offset =
      (o_trn_mo_nor - f32x2(0.5f)) * f32x2(n_ratio_x_to_y, 1.f);
  o_trn_fc_nor_offset *= 5.5f;
  float n_tau = 6.2831f;
  float n_dist_prod = 1.f;
  float n_dist_prod2 = 1.f;
  float n_dist_min = 1.f;
  float n_it_nor_dmin = 0.f;
  float n_its = 20.f;
  float n_dist_center = length(o_trn_fc_nor_offset);
  float n_one_it_nor = (1.f / n_its);
  float n_it = floor(n_dist_center / n_one_it_nor);
  float n_it_nor = n_it / n_its;
  float n_radiu_start = n_it_nor + n_one_it_nor * 0.5f;
  float n_radiu_end = n_radiu_start;
  float n_ang_nor = n_it_nor * 0.2f;
  float n_dist_segment1 = f_n_dist_segment(
      __ctx, f_o_rotd(__ctx, o_trn_fc_nor_offset, sin(iTime + n_it_nor)),
      n_ang_nor, n_radiu_start, n_radiu_end);
  n_dist_segment1 = pow(n_dist_segment1, 1.f / 3.f);
  float nss = 1.f / n_its;
  n_dist_segment1 =
      smoothstep(nss, nss * 7.f + n_ratio_1to_iresx * 20.f, n_dist_segment1);
  fragColor = f32x4(f32x3(1.f - n_dist_segment1), 1.f);
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
