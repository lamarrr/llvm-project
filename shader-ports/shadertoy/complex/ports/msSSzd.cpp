// Port of https://www.shadertoy.com/view/msSSzd; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msSSzd.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

float f_n_rand(Context &__ctx, f32x2 o) {
  return fract(
      sin(o.data[0] * 32.123f + o.data[1] * 13.17f + o.data[0] * o.data[1]) *
      323.231f);
}
f32x2 o_scale = f32x2(0.02f);
float n_tau = 6.2831f;
float f_n_rand_interpolated(Context &__ctx, f32x2 fragCoord,
                            f32x3 iResolution) {
  f32x2 o_fragCoord_scaled = ((fragCoord.swizzle<0, 1>()) * o_scale);
  f32x2 o_iResolution_scaled = (iResolution.swizzle<0, 1>() * o_scale);
  f32x2 o_fragCoord_scaled_floor = floor(o_fragCoord_scaled);
  f32x2 o_iResolution_scaled_floor = floor(o_iResolution_scaled);
  f32x2 o_fragCoord_scaled_fract = fract(o_fragCoord_scaled);
  f32x2 o_iResolution_scaled_fract = fract(o_iResolution_scaled);
  f32x2 o_trpino_scldfloor =
      (o_fragCoord_scaled_floor.swizzle<0, 1>() -
       o_iResolution_scaled_floor.swizzle<0, 1>() * 0.5f) /
      o_iResolution_scaled_floor.data[1];
  float n_index = floor(o_fragCoord_scaled_floor.data[1] *
                            o_iResolution_scaled_floor.data[0] +
                        o_fragCoord_scaled_floor.data[0]);
  float n_index_nor = n_index / (o_iResolution_scaled_floor.data[0] *
                                 o_iResolution_scaled_floor.data[1]);
  float n_col_xp1yp0 =
      f_n_rand(__ctx, o_fragCoord_scaled_floor + f32x2(1.f, 0.f));
  float n_col_xp0yp1 =
      f_n_rand(__ctx, o_fragCoord_scaled_floor + f32x2(0.f, 1.f));
  float n_col_xp0yp0 = f_n_rand(__ctx, o_fragCoord_scaled_floor);
  float n_col_xp1yp1 =
      f_n_rand(__ctx, o_fragCoord_scaled_floor + f32x2(1.f, 1.f));
  float n_mixed_xp0yp0andxp1yp0 =
      mix(n_col_xp0yp0, n_col_xp1yp0, o_fragCoord_scaled_fract.data[0]);
  float n_mixed_xp0yp1andxp1yp1 =
      mix(n_col_xp0yp1, n_col_xp1yp1, o_fragCoord_scaled_fract.data[0]);
  float n_mixed_xp0yp0andxp1yp0_and_n_mixed_xp0yp1andxp1yp1 =
      mix(n_mixed_xp0yp0andxp1yp0, n_mixed_xp0yp1andxp1yp1,
          o_fragCoord_scaled_fract.data[1]);
  return n_mixed_xp0yp0andxp1yp0_and_n_mixed_xp0yp1andxp1yp1;
}
float f_n_rand_interpolated_layered(Context &__ctx, f32x2 fragCoord,
                                    f32x3 iResolution) {
  float n_start = 1.f;
  f32x2 o_freq_amp1 = f32x2(n_start * 1.f, n_start / 1.f);
  f32x2 o_freq_amp2 = f32x2(n_start * 2.f, n_start / 2.f);
  f32x2 o_freq_amp3 = f32x2(n_start * 4.f, n_start / 4.f);
  f32x2 o_freq_amp_sum = o_freq_amp1 + o_freq_amp2 + o_freq_amp3;
  float n_rand_interpolated1 =
      f_n_rand_interpolated(__ctx, fragCoord * o_freq_amp1.data[0],
                            iResolution) *
      o_freq_amp1.data[1];
  float n_rand_interpolated2 =
      f_n_rand_interpolated(__ctx, fragCoord * o_freq_amp2.data[0],
                            iResolution) *
      o_freq_amp2.data[1];
  float n_rand_interpolated3 =
      f_n_rand_interpolated(__ctx, fragCoord * o_freq_amp3.data[0],
                            iResolution) *
      o_freq_amp3.data[1];
  float n_sum_rand_interpolated =
      n_rand_interpolated1 + n_rand_interpolated2 + n_rand_interpolated3;
  float n_rand_interpolated_layered =
      n_sum_rand_interpolated / o_freq_amp_sum.data[1];
  return n_rand_interpolated_layered;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 o_trpino =
      (fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * 0.5f) /
      iResolution.data[1];
  f32x2 o_trpino_nooffset =
      (fragCoord.swizzle<0, 1>()) / iResolution.swizzle<0, 1>();
  f32x2 o_trmono =
      (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * 0.5f) /
      iResolution.data[1];
  f32x2 o_trmono_nooffset = (iMouse.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 o_fragCoord_scaled = ((fragCoord.swizzle<0, 1>()) * o_scale);
  f32x2 o_iResolution_scaled = (iResolution.swizzle<0, 1>() * o_scale);
  f32x2 o_fragCoord_scaled_floor = floor(o_fragCoord_scaled);
  f32x2 o_iResolution_scaled_floor = floor(o_iResolution_scaled);
  f32x2 o_fragCoord_scaled_fract = fract(o_fragCoord_scaled);
  f32x2 o_iResolution_scaled_fract = fract(o_iResolution_scaled);
  f32x2 o_trpino_scldfloor =
      (o_fragCoord_scaled_floor.swizzle<0, 1>() -
       o_iResolution_scaled_floor.swizzle<0, 1>() * 0.5f) /
      o_iResolution_scaled_floor.data[1];
  float n_index = floor(o_fragCoord_scaled_floor.data[1] *
                            o_iResolution_scaled_floor.data[0] +
                        o_fragCoord_scaled_floor.data[0]);
  float n_index_nor = n_index / (o_iResolution_scaled_floor.data[0] *
                                 o_iResolution_scaled_floor.data[1]);
  float n_rand_interpolated_layered =
      f_n_rand_interpolated_layered(__ctx, fragCoord, iResolution);
  f32x2 o_delta = o_trpino - f32x2(0.f);
  float n_ang = atan(o_delta.data[1], o_delta.data[0]) + (n_tau / 2.f);
  float n_ang_nor = n_ang / n_tau;
  float n_rnd_intrplt_lrd = f_n_rand_interpolated_layered(
      __ctx,
      f32x2(n_ang_nor * iResolution.data[0],
            1.f - n_ang_nor * iResolution.data[1] + iTime * 111.f),
      iResolution);
  fragColor = f32x4(n_rand_interpolated_layered);
  float n_dist = length(o_trpino);
  float n_speed = 0.9f;
  float n_radius = fract(iTime * n_speed);
  n_radius = float(fract(iTime * n_speed / 2.f) > 0.5f) - n_radius;
  n_radius *= 2.f;
  n_radius = pow(n_radius, 2.f);
  n_dist = n_dist + (n_rnd_intrplt_lrd - 0.5f) * n_dist;
  float n_mask = smoothstep(n_radius, n_radius + 0.05f, n_dist);
  f32x4 o_col_text = texture(__ctx.channel0, o_trpino_nooffset);
  f32x4 o_col_text_inverted = 1.f - o_col_text;
  fragColor =
      f32x4(n_mask) * o_col_text + f32x4(1.f - n_mask) * o_col_text_inverted;
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
