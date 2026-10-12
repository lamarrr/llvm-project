// Port of https://www.shadertoy.com/view/ddfSRM; author: rge15.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddfSRM.json.
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
float N21(Context &__ctx, f32x2 p) {
  p = 50.f * fract(p * 3.14159265359f);
  p = 50.f * fract((p + f32x2(0.24342f, 709.23f)) / 3.14159265359f);
  return -1.f + 2.f * fract((p.data[0] * p.data[1]));
}
f32x2 N22(Context &__ctx, f32x2 p) {
  p = 50.f * fract((p + f32x2(0.24342f, 709.23f)) / 3.14159265359f);
  return 1.f -
         2.f * fract(p * f32x2(p.data[0] + p.data[1], p.data[1] * p.data[0]) +
                     f32x2(0.29137f, 0.9348f));
}
f32x2 modulo(Context &__ctx, f32x2 divident, f32x2 divisor) {
  return mod(divident, divisor) * (divisor / abs(divisor));
}
float valueNoise(Context &__ctx, f32x2 pos) {
  f32x2 i = floor(pos);
  f32x2 f = fract(pos);
  f32x2 inter = f * f * (3.f - 2.f * f);
  float vA = N21(__ctx, i + f32x2(0.f, 0.f));
  float vB = N21(__ctx, i + f32x2(1.f, 0.f));
  float vC = N21(__ctx, i + f32x2(0.f, 1.f));
  float vD = N21(__ctx, i + f32x2(1.f, 1.f));
  return vA + (vB - vA) * inter.data[0] + (vC - vA) * inter.data[1] +
         (vD - vC - vB + vA) * inter.data[0] * inter.data[1];
}
float tiledValueNoise(Context &__ctx, f32x2 pos, f32x2 frec) {
  f32x2 f = fract(pos);
  f32x2 imin = floor(pos);
  f32x2 imax = ceil(pos);
  imin = modulo(__ctx, imin, frec);
  imax = modulo(__ctx, imax, frec);
  f32x2 inter = f * f * (3.f - 2.f * f);
  float vA = N21(__ctx, imin);
  float vB = N21(__ctx, f32x2(imax.data[0], imin.data[1]));
  float vC = N21(__ctx, f32x2(imin.data[0], imax.data[1]));
  float vD = N21(__ctx, imax);
  return vA + (vB - vA) * inter.data[0] + (vC - vA) * inter.data[1] +
         (vD - vC - vB + vA) * inter.data[0] * inter.data[1];
}
float gradientNoise(Context &__ctx, f32x2 pos) {
  f32x2 f = fract(pos);
  f32x2 i = floor(pos);
  f32x2 inter = f * f * (3.f - 2.f * f);
  f32x2 vecA = N22(__ctx, i);
  f32x2 vecB = N22(__ctx, i + f32x2(1.f, 0.f));
  f32x2 vecC = N22(__ctx, i + f32x2(0.f, 1.f));
  f32x2 vecD = N22(__ctx, i + f32x2(1.f));
  float valA = dot(f, vecA);
  float valB = dot(f - f32x2(1.f, 0.f), vecB);
  float valC = dot(f - f32x2(0.f, 1.f), vecC);
  float valD = dot(f - f32x2(1.f, 1.f), vecD);
  return valA + (valB - valA) * inter.data[0] + (valC - valA) * inter.data[1] +
         (valD - valB - valC + valA) * inter.data[0] * inter.data[1];
}
float tiledGradientNoise(Context &__ctx, f32x2 pos, f32x2 frec) {
  f32x2 f = fract(pos);
  f32x2 imin = floor(pos);
  f32x2 imax = ceil(pos);
  imin = modulo(__ctx, imin, frec);
  imax = modulo(__ctx, imax, frec);
  f32x2 inter = f * f * (3.f - 2.f * f);
  f32x2 vecA = N22(__ctx, imin);
  f32x2 vecB = N22(__ctx, f32x2(imax.data[0], imin.data[1]));
  f32x2 vecC = N22(__ctx, f32x2(imin.data[0], imax.data[1]));
  f32x2 vecD = N22(__ctx, imax);
  float valA = dot(f, vecA);
  float valB = dot(f - f32x2(1.f, 0.f), vecB);
  float valC = dot(f - f32x2(0.f, 1.f), vecC);
  float valD = dot(f - f32x2(1.f, 1.f), vecD);
  return valA + (valB - valA) * inter.data[0] + (valC - valA) * inter.data[1] +
         (valD - valB - valC + valA) * inter.data[0] * inter.data[1];
}
float valueFBM(Context &__ctx, f32x2 p, float h, int octaves) {
  float value = 0.f;
  for (int i = 0; i < octaves; i++) {
    float f = pow(2.f, float(i));
    float a = pow(f, -h);
    value += a * valueNoise(__ctx, f * p);
  }
  return value;
}
float tiledValueFBM(Context &__ctx, f32x2 p, f32x2 frec, float h, int octaves) {
  float value = 0.f;
  for (int i = 0; i < octaves; i++) {
    float f = pow(2.f, float(i));
    float a = pow(f, -h);
    value += a * tiledValueNoise(__ctx, f * p, frec);
  }
  return value;
}
float gradientFBM(Context &__ctx, f32x2 p, float h, int octaves) {
  float value = 0.f;
  for (int i = 0; i < octaves; i++) {
    float f = pow(2.f, float(i));
    float a = pow(f, -h);
    value += a * gradientNoise(__ctx, f * p);
  }
  return value;
}
float tiledGradientFBM(Context &__ctx, f32x2 p, f32x2 frec, float h,
                       int octaves) {
  float value = 0.f;
  for (int i = 0; i < octaves; i++) {
    float f = pow(2.f, float(i));
    float a = pow(f, -h);
    value += a * tiledGradientNoise(__ctx, f * p, frec);
  }
  return value;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 p_uv = f32x2(atan(uv.data[0], uv.data[1]), length(uv));
  f32x2 np_uv =
      f32x2(p_uv.data[0] / (2.f * 3.14159265359f) + 0.5f, p_uv.data[1]);
  float noise = 0.f;
  noise = 0.7f * tiledGradientFBM(
                     __ctx, np_uv * f32x2(10.f, 2.f) + f32x2(iTime * 0.5f),
                     f32x2(10.f), 0.49f, 2);
  noise *= np_uv.data[1];
  f32x3 col = f32x3(noise);
  fragColor = f32x4(col, 1.0f);
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
