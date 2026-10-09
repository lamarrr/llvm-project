// Port of https://www.shadertoy.com/view/ctfSRl; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctfSRl.json.
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

float f_n_rnd(Context &__ctx, float n) {
  n = fract(n * .1031f);
  n *= n + 33.33f;
  n *= n + n;
  return fract(n);
}
f32x2 f_o_rnd(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 o_fc = (fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * .5f) /
               iResolution.data[1];
  f32x2 o_fccp = o_fc;
  f32x2 o_mc = (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * .5f) /
               iResolution.data[1];
  if (iMouse.data[2] == 0.f) {
    float n_amp = sin(iTime * 0.2f);
    o_mc = f32x2(sin(iTime), cos(iTime)) * n_amp;
  }
  f32x2 o_mccp = o_mc;
  o_fc *= 18.f;
  o_mc *= 18.f;
  float n_tau = 6.2831f;
  float n_amp = 2.f;
  float n_dmin = 1.f;
  f32x2 o_fcmcdlt = o_fc - o_mc;
  float n_ang = fract(atan(o_fcmcdlt.data[1], o_fcmcdlt.data[0]) / n_tau);
  float n_intensity = clamp(length(o_fccp - o_mccp), 0.f, 1.f);
  f32x2 o_fcf = fract(o_fc) - .5f;
  f32x2 o_sclk = f32x2(9.f);
  float n_min = floor(o_sclk.data[0] / 2.f);
  for (float n_x = -n_min; n_x <= n_min; n_x += 1.f) {
    for (float n_y = -n_min; n_y <= n_min; n_y += 1.f) {
      f32x2 o_fck = f32x2(n_x, n_y);
      f32x2 o_rnd = f_o_rnd(__ctx, floor(o_fc) + o_fck);
      f32x2 o_amp_rnd = n_amp * o_rnd;
      n_intensity = n_ang * (1.f - length(o_fcmcdlt));
      f32x2 o_dlt = (floor(o_fc) + o_fck) - o_mc;
      float n_ang = atan(o_dlt.data[0], o_dlt.data[1]);
      f32x2 o_off = f32x2(sin(n_ang) * n_amp, cos(n_ang) * n_amp);
      o_fck += o_off;
      float n = length(o_fcf - o_fck);
      if (n < n_dmin) {
        n_dmin = n;
      }
    }
  }
  fragColor = f32x4(sqrt(n_dmin));
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
