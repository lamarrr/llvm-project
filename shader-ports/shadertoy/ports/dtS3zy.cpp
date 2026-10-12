// Port of https://www.shadertoy.com/view/dtS3zy; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtS3zy.json.
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
f32x2 f_o_rotd(Context &__ctx, f32x2 o, float n_angle_nor) {
  float n_tau = 6.283185f;
  float n_ang_rad = n_angle_nor * n_tau;
  return f32x2(cos(n_ang_rad) * o.data[0] - sin(n_ang_rad) * o.data[1],
               sin(n_ang_rad) * o.data[0] + cos(n_ang_rad) * o.data[1]);
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
  o_trn_fc_nor_offset = f_o_rotd(__ctx, o_trn_fc_nor_offset, (1.f / 4.f) * 3.f);
  o_trn_fc_nor_offset *= 1.2f;
  f32x2 o_fc_nor = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float n_tau = 6.283185f;
  float n_freq_sec = fract(iTime) * n_tau;
  float n_ang_nor = fract(
      atan(o_trn_fc_nor_offset.data[1], o_trn_fc_nor_offset.data[0]) / n_tau);
  f32x4 o_col = f32x4(0.f);
  float n_t_yea = (iDate.data[0]);
  float n_t_mon = (iDate.data[1]);
  float n_t_day = (iDate.data[2]);
  float n_t_sec_since_midnight = (iDate.data[3]);
  float n_w, n_dcenter, n_radius, n_pointerstart, n_pointerend, n,
      n_radians_nor, n_divisor, n_glow;
  f32x2 o;
  n_dcenter = length(f32x2(0.f) - o_trn_fc_nor_offset);
  float n_aa = n_ratio_1to_iresx * 20.f;
  n_pointerstart = 0.f;
  n_pointerend = 0.5f;
  n_radius = max(n_dcenter, n_pointerstart);
  n_radius = min(n_radius, n_pointerend);
  n_radians_nor = -n_t_sec_since_midnight / 60.f;
  n_radians_nor = -floor(n_t_sec_since_midnight) / 60.f;
  o = f32x2(cos(n_radians_nor * n_tau) * n_radius,
            sin(n_radians_nor * n_tau) * n_radius);
  float nsec = length(o - o_trn_fc_nor_offset);
  n_w = 0.01f;
  nsec = smoothstep(n_w + n_aa, n_w, nsec);
  n_pointerstart = 0.0f;
  n_pointerend = 0.3f;
  n_radius = max(n_dcenter, n_pointerstart);
  n_radius = min(n_radius, n_pointerend);
  n_radians_nor = -n_t_sec_since_midnight / n_divisor;
  n_radians_nor = -floor(n_t_sec_since_midnight / 60.f) / 60.f;
  o = f32x2(cos(n_radians_nor * n_tau) * n_radius,
            sin(n_radians_nor * n_tau) * n_radius);
  float nmin = length(o - o_trn_fc_nor_offset);
  n_w = 0.01f;
  nmin = smoothstep(n_w + n_aa, n_w, nmin);
  n_pointerstart = 0.0f;
  n_pointerend = 0.2f;
  n_radius = max(n_dcenter, n_pointerstart);
  n_radius = min(n_radius, n_pointerend);
  n_radians_nor = -(n_t_sec_since_midnight / 60.f / 60.f / 12.f);
  n_radians_nor = -floor(n_t_sec_since_midnight / 60.f / 60.f) / 12.f;
  o = f32x2(cos(n_radians_nor * n_tau) * n_radius,
            sin(n_radians_nor * n_tau) * n_radius);
  float nhou = length(o - o_trn_fc_nor_offset);
  n_w = 0.01f;
  nhou = smoothstep(n_w + n_aa, n_w, nhou);
  n_pointerstart = 0.45f;
  n_pointerend = 0.5f;
  n_radius = max(n_dcenter, n_pointerstart);
  n_radius = min(n_radius, n_pointerend);
  n_radians_nor = floor(n_ang_nor * 60.f + 0.5f) / 60.f;
  o = f32x2(cos(n_radians_nor * n_tau) * n_radius,
            sin(n_radians_nor * n_tau) * n_radius);
  float nminind = length(o - o_trn_fc_nor_offset);
  n_w = 0.001f;
  n_aa = n_ratio_1to_iresx * 20.f;
  nminind = smoothstep(n_w + n_aa, n_w, nminind);
  n_pointerstart = 0.35f;
  n_pointerend = 0.5f;
  n_radius = max(n_dcenter, n_pointerstart);
  n_radius = min(n_radius, n_pointerend);
  n_radians_nor = floor(n_ang_nor * 12.f + 0.5f) / 12.f;
  o = f32x2(cos(n_radians_nor * n_tau) * n_radius,
            sin(n_radians_nor * n_tau) * n_radius);
  float nhouind = length(o - o_trn_fc_nor_offset);
  n_w = 0.001f;
  n_aa = n_ratio_1to_iresx * 20.f;
  nhouind = smoothstep(n_w + n_aa, n_w, nhouind);
  o = f32x2(cos(n_radians_nor * n_tau) * n_radius,
            sin(n_radians_nor * n_tau) * n_radius);
  float nborder = length(f32x2(0.f) - o_trn_fc_nor_offset);
  nborder = sin(nborder * n_tau + 1.5f) * 0.5f + 0.5f;
  nborder = pow(nborder, 1.f / 3.f);
  n_w = 0.1f;
  n_aa = n_ratio_1to_iresx * 123.f;
  nborder = smoothstep(n_w + n_aa, n_w, nborder);
  n = nsec;
  n += nmin;
  n += nhou;
  n += nminind;
  n += nhouind;
  n += nborder;
  fragColor = f32x4(n);
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
