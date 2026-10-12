// Port of https://www.shadertoy.com/view/msKyDR; author: uctumi.
// Dataset license metadata: apache-2.0; original retained in
// upstream/msKyDR.json.
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
float rand(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
float rand2(Context &__ctx, f32x2 co) {
  return fract(cos(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
float valueNoiseSimple(Context &__ctx, f32x2 vl) {
  float minStep = 1.0f;
  f32x2 grid = floor(vl);
  f32x2 gridPnt1 = grid;
  f32x2 gridPnt2 = f32x2(grid.data[0], grid.data[1] + minStep);
  f32x2 gridPnt3 = f32x2(grid.data[0] + minStep, grid.data[1]);
  f32x2 gridPnt4 = f32x2(gridPnt3.data[0], gridPnt2.data[1]);
  float s = rand2(__ctx, grid);
  float t = rand2(__ctx, gridPnt3);
  float u = rand2(__ctx, gridPnt2);
  float v = rand2(__ctx, gridPnt4);
  float x1 = smoothstep(0.f, 1.f, fract(vl.data[0]));
  float interpX1 = mix(s, t, x1);
  float interpX2 = mix(u, v, x1);
  float y = smoothstep(0.f, 1.f, fract(vl.data[1]));
  float interpY = mix(interpX1, interpX2, y);
  return interpY;
}
float getLowFreqs(Context &__ctx) {
  const int NUM_FREQS = 32;
  const float lowStart = 0.65f;
  const float lowEnd = 0.75f;
  float result = 0.0f;
  for (int i = 0; i < NUM_FREQS; i++) {
    result += texture(__ctx.channel0,
                      f32x2(lowStart + (lowEnd - lowStart) * float(i) /
                                           float(NUM_FREQS - 1),
                            0.25f))
                  .data[0];
  }
  return smoothstep(0.0f, 1.0f, (result / float(NUM_FREQS)) * 2.f);
}
float fractalNoise(Context &__ctx, f32x2 vl) {
  float persistance = 2.0f;
  float amplitude = 0.5f;
  float rez = 0.0f;
  f32x2 p = vl;
  for (float i = 0.0f; i < 8.0f; i++) {
    rez += amplitude * valueNoiseSimple(__ctx, p);
    amplitude /= persistance;
    p *= persistance;
  }
  return rez;
}
float complexFBM(Context &__ctx, f32x2 p) {
  float sound = getLowFreqs(__ctx);
  float slow = iTime / 2.5f;
  float fast = iTime / .5f;
  f32x2 offset1 = f32x2(slow, 0.f);
  f32x2 offset2 = f32x2(sin(fast) * 0.1f, 0.f);
  return (1.f + sound) *
         fractalNoise(
             __ctx,
             p + offset1 +
                 fractalNoise(
                     __ctx, p + fractalNoise(
                                    __ctx, p + 2.f * fractalNoise(
                                                         __ctx, p - offset2))));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 blueColor = f32x3(0.07f, 0.2f, .5f);
  f32x3 orangeColor2 = f32x3(0, 0, 0);
  f32x3 rez = mix(orangeColor2, blueColor, complexFBM(__ctx, uv));
  fragColor = f32x4(rez, 1.0f);
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
