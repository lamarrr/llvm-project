// Port of https://www.shadertoy.com/view/msG3Wm; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msG3Wm.json.
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

float f_n_by_o(Context &__ctx, f32x2 o) {
  o.data[0] += o.data[0] * 0.1f;
  float n =
      sin(o.data[0] * o.data[1] + 1.234f * 3.13347f) * 1.2341f * o.data[0] +
      12.3414f * o.data[1];
  return fract((fract(n) + o.data[0] * o.data[0] + o.data[1] * o.data[1]));
}
float f_n_noise_interpolated(Context &__ctx, f32x2 o) {
  f32x2 o_xp0yp0 = o + f32x2(0.f, 0.f);
  f32x2 o_xp0yp1 = o + f32x2(0.f, 1.f);
  f32x2 o_xp1yp0 = o + f32x2(1.f, 0.f);
  f32x2 o_xp1yp1 = o + f32x2(1.f, 1.f);
  float n_xp0yp0 = f_n_by_o(__ctx, floor(o_xp0yp0));
  float n_xp0yp1 = f_n_by_o(__ctx, floor(o_xp0yp1));
  float n_xp1yp0 = f_n_by_o(__ctx, floor(o_xp1yp0));
  float n_xp1yp1 = f_n_by_o(__ctx, floor(o_xp1yp1));
  f32x2 o_fract = fract(o);
  float n1 = (1.f - o_fract.data[0]) * n_xp0yp0 + o_fract.data[0] * n_xp1yp0;
  float n2 = (1.f - o_fract.data[0]) * n_xp0yp1 + o_fract.data[0] * n_xp1yp1;
  float n3 = (1.f - o_fract.data[1]) * n1 + o_fract.data[1] * n2;
  return n3;
}
float f_n_noise_interpolated_stacked(Context &__ctx, f32x2 o, float n_its,
                                     float n_it_factor_freq,
                                     float n_it_factor_amp) {
  float n_freq = 1.f;
  float n_amp = 1.f;
  float n_sum = 0.f;
  float n_factor_amp_sum = 0.f;
  for (float n_it = 0.f; n_it < n_its; n_it += 1.f) {
    n_freq *= n_it_factor_freq;
    n_amp *= n_it_factor_amp;
    n_factor_amp_sum += n_amp;
    n_sum += f_n_noise_interpolated(__ctx, o * n_freq) * n_amp;
  }
  return n_sum / n_factor_amp_sum;
}
float f_n_noise_interpolated_blurred(Context &__ctx, f32x2 o_fc) {
  f32x2 o_scl_krn = f32x2(3.f);
  float n_sum = 0.f;
  f32x2 o_scl_krn_half = floor(o_scl_krn / 2.f);
  for (float n_x = 0.f; n_x < o_scl_krn.data[0]; n_x += 1.f) {
    for (float n_y = 0.f; n_y < o_scl_krn.data[0]; n_y += 1.f) {
      f32x2 o_trn =
          f32x2(n_x - o_scl_krn_half.data[0], n_y - o_scl_krn_half.data[1]);
      n_sum += f_n_noise_interpolated(__ctx, o_fc + o_trn);
    }
  }
  return n_sum / (o_scl_krn.data[0] * o_scl_krn.data[1]);
}
float f_n_noise_interpolated_blurred_stacked(Context &__ctx, f32x2 o,
                                             float n_its,
                                             float n_it_factor_freq,
                                             float n_it_factor_amp) {
  float n_freq = 1.f;
  float n_amp = 1.f;
  float n_sum = 0.f;
  float n_factor_amp_sum = 0.f;
  for (float n_it = 0.f; n_it < n_its; n_it += 1.f) {
    n_freq *= n_it_factor_freq;
    n_amp *= n_it_factor_amp;
    n_factor_amp_sum += n_amp;
    n_sum += f_n_noise_interpolated_blurred(__ctx, o * n_freq) * n_amp;
  }
  return n_sum / n_factor_amp_sum;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 o_fc = (fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * .5f) /
               iResolution.data[1];
  fragColor = f32x4(1.0f);
  float n_5 = f_n_noise_interpolated(__ctx, o_fc * 5.f);
  float n_10 = f_n_noise_interpolated(__ctx, o_fc * 10.f);
  float n_20 = f_n_noise_interpolated(__ctx, o_fc * 20.f);
  float n_200 = f_n_noise_interpolated(__ctx, o_fc * 200.f);
  float n_sum = n_5 / 4.f + n_10 / 4.f + n_20 / 4.f;
  +n_200 / 4.f;
  float n_11 = f_n_noise_interpolated(__ctx, o_fc * 11.f) * .5f;
  float n_22 = f_n_noise_interpolated(__ctx, o_fc * 22.f) * .25f;
  float n_44 = f_n_noise_interpolated(__ctx, o_fc * 44.f) * .125f;
  float n_88 = f_n_noise_interpolated(__ctx, o_fc * 88.f) * 0.0625f;
  float n_s = n_11 + n_22 + n_44 + n_88;
  fragColor = f32x4(n_s);
  float n_noise_interpolated_stacked =
      f_n_noise_interpolated_stacked(__ctx, o_fc, 20.f, 6.f, 0.5f);
  float n_fac = 222.f;
  float n_noise_interpolated_blurred =
      f_n_noise_interpolated_blurred(__ctx, o_fc * n_fac);
  float n_noise_interpolated = f_n_noise_interpolated(__ctx, o_fc * n_fac);
  fragColor = f32x4(n_noise_interpolated_blurred);
  float n_noise_interpolated_stacked_10 =
      f_n_noise_interpolated_stacked(__ctx, o_fc, 10.f, 10.f, 0.5f);
  float n_noise_interpolated_blurred_stacked_10 =
      f_n_noise_interpolated_blurred_stacked(__ctx, o_fc, 10.f, 10.f, 0.5f);
  fragColor = f32x4(n_noise_interpolated_blurred_stacked_10);
  if (o_fc.data[0] > 0.f) {
    fragColor = f32x4(n_noise_interpolated_stacked_10);
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
