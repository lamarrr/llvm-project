// Port of https://www.shadertoy.com/view/DtySWD; author: NikolaErceg.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtySWD.json.
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
float logisticAmp(Context &__ctx, float amp) {
  float c = mix(.8f, 1.f, smoothstep(0.f, 10.f, iTime)), a = 20.f;
  return ((1.f / (exp(-a * (amp - c)) + 1.f)) -
          (1.f / (exp(-a * (0.0f - c)) + 1.f))) /
         ((1.f / (exp(-a * (1.0f - c)) + 1.f)) -
          (1.f / (exp(-a * (0.0f - c)) + 1.f)));
}
float getVol(Context &__ctx, float samples) {
  float avg = 0.f;
  for (float i = 0.f; i < samples; i++)
    avg += (texelFetch(__ctx.channel0, i32x2(int(i / samples * 512.f), 0))
                .data[0]);
  return avg / samples;
}
float PHI = 1.61803398874989484820459f;
float gold_noise(Context &__ctx, f32x2 xy, float seed) {
  return fract(tan(distance(xy * PHI, xy) * seed) * xy.data[0]);
}
f32x3 palette(Context &__ctx, float dist) {
  f32x3 a = f32x3(0.5f, 0.0f, 0.5f);
  f32x3 b = f32x3(0.25f, 0.25f, 0.5f);
  f32x3 c = f32x3(1.0f, 1.0f, 1.0f);
  f32x3 d = f32x3(0.263f, 0.416f, 0.557f);
  return a + b * cos(6.28318f * (c * dist * d));
}
float sdHexagon(Context &__ctx, f32x2 p, float r) {
  const f32x3 k = f32x3(-0.866025404f, 0.5f, 0.577350269f);
  p = abs(p);
  p -= 2.0f * min(dot(k.swizzle<0, 1>(), p), 0.0f) * k.swizzle<0, 1>();
  p -= f32x2(clamp(p.data[0], -k.data[2] * r, k.data[2] * r), r);
  return length(p) * sign(p.data[1]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float speed = 0.1f;
  float ringmultiplier = 2.0f;
  float ringwidth = 64.0f;
  float smoothing = 0.4f;
  float iter = 6.0f;
  float zoom = 2.0f;
  float seed = 6502.f;
  float vol = getVol(__ctx, 8.0f);
  float myTime = (iTime * 0.5f) + 0.0f + vol;
  zoom += vol * 0.1f;
  f32x2 uv =
      ((fragCoord * 2.0f - iResolution.swizzle<0, 1>()) / iResolution.data[1]);
  f32x2 uv0 = uv;
  float noise =
      gold_noise(__ctx, fragCoord, seed) - ((sin(myTime * 0.5f) * 0.5f) + 0.5f);
  uv *= (zoom * sin(myTime * 0.05f));
  f32x3 outCol = f32x3(0.0f);
  for (float i = 0.0f; i < iter; i++) {
    uv = fract(uv * (1.25f + (sin((myTime * 0.1f) * i)))) - 0.5f;
    float dist = sdHexagon(__ctx, uv, 0.2f * (vol + i));
    f32x3 col = palette(__ctx, length(uv0) + myTime + (vol * 4.f));
    dist = sin((dist - (myTime * speed)) * ringmultiplier * 2.0f) / ringwidth;
    dist = abs(dist);
    dist = pow(0.00015f / dist, 1.05f);
    col = col * (smoothstep(0.0f, 0.5f - smoothing, dist) * noise) * 2.0f;
    outCol += col + dist;
  }
  fragColor = f32x4(outCol, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
