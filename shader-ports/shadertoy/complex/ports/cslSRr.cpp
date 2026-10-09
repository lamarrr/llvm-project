// Port of https://www.shadertoy.com/view/cslSRr; author: kishimisu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cslSRr.json.
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
float lightRep = 12.f;
float attenuation = 20.f;
float hash11(Context &__ctx, float p) {
  p = fract(p * .1031f);
  p *= p + 33.33f;
  return fract(2.f * p * p);
}
f32x3 getLight(Context &__ctx, float d, f32x3 color) {
  return max(f32x3(0.f),
             color / (1.f + pow(abs(d * attenuation), 1.3f)) - .001f * 0.f);
}
float getLevel(Context &__ctx, float x) {
  return texelFetch(__ctx.channel0, i32x2(int(x * 512.f), 0)).data[0];
}
float getPitch(Context &__ctx, float freq, int octave) {
  return getLevel(__ctx, pow(2.0f, float(octave)) * freq / 12000.0f);
}
float logX(Context &__ctx, float x, float a, float c) {
  return 1.0f / (exp(-a * (x - c)) + 1.0f);
}
float logisticAmp(Context &__ctx, float amp) {
  float c = 1.0f - (0.25f);
  float a = 20.0f * (1.0f - iMouse.data[1] / iResolution.data[1]);
  a = 20.f;
  return (logX(__ctx, amp, a, c) - logX(__ctx, 0.0f, a, c)) /
         (logX(__ctx, 1.0f, a, c) - logX(__ctx, 0.0f, a, c));
}
float getAudioIntensityAt(Context &__ctx, float x) {
  x = abs(fract(x));
  float freq = pow(2.f, x * 3.f) * 261.f;
  return iChannelTime[0] <= 0.f ? hash11(__ctx, x)
                                : logisticAmp(__ctx, getPitch(__ctx, freq, 1));
}
float map(Context &__ctx, f32x3 p, f32x3 &col) {
  p.data[1] = abs(p.data[1]) - 13.f - getAudioIntensityAt(__ctx, 0.f) * 1.2f;
  f32x2 id = floor((p.swizzle<0, 2>() + 2.f / 2.f) / 2.f);
  p.data[1] +=
      sin(length(sin(id / 5.23f - iTime) * cos(id / 10.45f + iTime))) * 8.f;
  f32x3 fp = (mod(p + lightRep / 2.f, lightRep) - lightRep / 2.f);
  fp.data[1] = p.data[1];
  const float r = 0.f;
  for (float j = -r; j <= r; j++)
    for (float i = -r; i <= r; i++) {
      f32x3 off = f32x3(i, 0.f, j) * lightRep;
      f32x3 nid = floor((p - off + lightRep / 2.f) / lightRep);
      float d = length(fp + off) - 1.f;
      f32x3 c = fract(sin((nid)*f32x3x3(127.1f, 311.7f, 74.7f, 269.5f, 183.3f,
                                        246.1f, 113.5f, 271.9f, 124.6f)) *
                      43758.5453123f);
      f32x3 light =
          f32x3(getAudioIntensityAt(__ctx, c.data[0] * .33f),
                getAudioIntensityAt(__ctx, c.data[1] * .33f + .33f),
                4.f * getAudioIntensityAt(__ctx, c.data[2] * .33f + .67f));
      light *= getAudioIntensityAt(__ctx, c.data[0] + c.data[2] + c.data[1]) +
               (c.data[0] + c.data[2] + c.data[1]);
      col += getLight(__ctx, d, light);
    }
  p.set_swizzle<0, 2>((mod(p.swizzle<0, 2>() + 2.f / 2.f, 2.f) - 2.f / 2.f));
  return length(p) - 1.f;
}
void initRayOriginAndDirection(Context &__ctx, f32x2 uv, f32x3 &ro, f32x3 &rd) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() * 2.f - 1.f;
  ro = f32x3(iTime * 8.f - 6.f, 0.f, 0.f);
  float t = -iTime * .15f * 0.f;
  f32x3 f = normalize(f32x3(cos(t), 0, sin(t)));
  f32x3 r = normalize(cross(f32x3(0, 1, 0), f));
  rd = normalize(f + uv.data[0] * r + uv.data[1] * cross(f, r));
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  f32x2 uv = (2.f * F - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 p, ro, rd, col;
  initRayOriginAndDirection(__ctx, uv, ro, rd);
  float t = 0.f;
  for (float i = 0.f; i < 30.f; i++) {
    p = ro + t * rd;
    t += map(__ctx, p, col);
    if (t > 100.f)
      break;
  }
  col = pow(col, f32x3(.45f));
  O = f32x4(col, 1.0f);
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
