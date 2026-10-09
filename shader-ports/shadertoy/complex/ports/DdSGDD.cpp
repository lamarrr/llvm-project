// Port of https://www.shadertoy.com/view/DdSGDD; author: Del.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdSGDD.json.
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
float noise(Context &__ctx, f32x3 x) {
  f32x3 i = floor(x);
  f32x3 f = fract(x);
  f = f * f * (3.0f - 2.0f * f);
  f32x2 uv =
      (i.swizzle<0, 1>() + f32x2(37.0f, 17.0f) * i.data[2]) + f.swizzle<0, 1>();
  f32x2 rg =
      textureLod(__ctx.channel0, (uv + 1.5f) / 256.0f, 0.0f).swizzle<1, 0>();
  return mix(rg.data[0], rg.data[1], f.data[2]);
}
float mNoise(Context &__ctx, f32x3 pos) {
  f32x3 q = pos;
  const f32x3x3 m = f32x3x3(0.20f, 0.80f, 0.60f, -0.80f, 0.36f, -0.48f, -0.60f,
                            -0.48f, 0.64f);
  float amplitude = .65f;
  float f = amplitude * noise(__ctx, q);
  q = m * q * 2.11f;
  float scale = 2.02f;
  for (int i = 0; i < 3; ++i) {
    f += amplitude * noise(__ctx, q);
    q = m * q * scale;
    amplitude *= 0.5f;
  }
  return f;
}
f32x2 polarMap(Context &__ctx, f32x2 uv, float shift, float inner) {
  float px = 1.0f - fract(atan(uv.data[1], uv.data[0]) / 6.28f + 0.25f) + shift;
  float py = (sqrt(uv.data[0] * uv.data[0] + uv.data[1] * uv.data[1]) *
                  (1.0f + inner * 2.0f) -
              inner) *
             2.0f;
  return f32x2(px, py);
}
f32x3 firePalette(Context &__ctx, float i) {
  float T = 1400.f + 1400.f * i;
  f32x3 L = f32x3(7.4f, 5.6f, 4.4f);
  L = pow(L, f32x3(5.0f)) * (exp(1.43876719683e5f / (T * L)) - 1.0f);
  return 1.0f - exp(-5e8f / L);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = (fragCoord.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  f32x2 uv = p;
  uv.data[0] = abs(uv.data[0]);
  float inner = 0.0f;
  float shift = 0.0f;
  float hrepeat = 16.0f;
  uv = polarMap(__ctx, uv * 1.4f, shift, inner);
  uv.data[0] *= hrepeat;
  if (iMouse.data[2] > 0.5f)
    uv = f32x2(p.data[0] * 4.0f, p.data[1] + 0.9f);
  float t = iTime * 0.2f;
  float fval = uv.data[1];
  uv = f32x2(uv.data[0], uv.data[1] - t) * 2.55f;
  fval = fval + 0.67f + (mNoise(__ctx, f32x3(uv, t * 1.1f)) * 0.33f);
  f32x3 color = firePalette(__ctx, 2.4f - fval);
  fragColor = f32x4(color.swizzle<0, 1, 2>(), 1.0f);
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
