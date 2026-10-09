// Port of https://www.shadertoy.com/view/lfs3R8; author: kilika.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/lfs3R8.json.
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
  Texture2D channel1;
  Texture2D channel2;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float grayscale(Context &__ctx, float x) {
  x = clamp(x, 0.0f, 1.0f);
  float contrastFactor = 3.5f;
  x = 0.5f - contrastFactor * (x - 1.5f) * (x - 0.5f);
  return clamp(x, 0.0f, 1.0f);
}
f32x4 colormap(Context &__ctx, float x) {
  float gray = grayscale(__ctx, x);
  return f32x4(gray, gray, gray, 1.0f);
}
float rand(Context &__ctx, f32x2 n) {
  return fract(sin(dot(n, f32x2(12.9898f, 4.1414f))) * 43758.5453f);
}
float noise(Context &__ctx, f32x2 p) {
  f32x2 ip = floor(p);
  f32x2 u = fract(p);
  u = u * u * (3.0f - 2.0f * u);
  float res =
      mix(mix(rand(__ctx, ip), rand(__ctx, ip + f32x2(1.0f, 0.0f)), u.data[0]),
          mix(rand(__ctx, ip + f32x2(0.0f, 1.0f)),
              rand(__ctx, ip + f32x2(1.0f, 1.0f)), u.data[0]),
          u.data[1]);
  return res * res;
}
const f32x2x2 mtx = f32x2x2(0.80f, 0.60f, -0.60f, 0.80f);
float fbm(Context &__ctx, f32x2 p) {
  float f = 0.0f;
  f += 0.500000f * noise(__ctx, p + iTime);
  p = mtx * p * 2.02f;
  f += 0.031250f * noise(__ctx, p);
  p = mtx * p * 2.01f;
  f += 0.250000f * noise(__ctx, p);
  p = mtx * p * 2.03f;
  f += 0.125000f * noise(__ctx, p);
  p = mtx * p * 2.01f;
  f += 0.062500f * noise(__ctx, p);
  p = mtx * p * 2.04f;
  f += 0.015625f * noise(__ctx, p + sin(iTime));
  return f / 0.96875f;
}
float pattern(Context &__ctx, f32x2 p) {
  return fbm(__ctx, p + fbm(__ctx, p + fbm(__ctx, p)));
}
float glow(Context &__ctx, f32x3 color, float baseRadius, float time,
           float alpha) {
  float brightness = dot(color, f32x3(0.299f, 0.587f, 0.114f));
  float radius = baseRadius + sin(time) * 0.5f;
  float adjustedGlow = brightness * max(radius, 0.0f) * alpha;
  return adjustedGlow;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x4 color1 = texture(__ctx.channel0, uv);
  f32x4 color2 = texture(__ctx.channel1, uv);
  f32x4 color3 = texture(__ctx.channel2, uv);
  float shade = pattern(__ctx, uv);
  fragColor = f32x4(colormap(__ctx, shade).swizzle<0, 1, 2>(), shade);
  f32x4 mixedColor = mix(color1, color2, fragColor);
  f32x4 color = f32x4(colormap(__ctx, shade).swizzle<0, 1, 2>(), shade);
  float glowStrength =
      glow(__ctx, mixedColor.swizzle<0, 1, 2>(), 0.5f, iTime, shade);
  float glowStrength2 =
      glow(__ctx, mixedColor.swizzle<0, 1, 2>(), 1.5f, iTime * 2.0f, shade);
  f32x3 glowColor =
      mixedColor.swizzle<0, 1, 2>() + glowStrength * glowStrength2;
  fragColor = f32x4(glowColor, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1,
               __binding__(0, 2) texture_2d<f32x4> channel2,
               __binding__(1, 2) sampler sampling2) {
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
  Context __ctx{
      {channel0, sampling0}, {channel1, sampling1}, {channel2, sampling2}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
