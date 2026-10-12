// Port of https://www.shadertoy.com/view/dlfSDM; author: Karp.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlfSDM.json.
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
f32x2 AspectRatioUV(Context &__ctx, f32x2 fragCoord, f32x2 resolution) {
  f32x2 uv =
      f32x2(fragCoord.swizzle<0, 1>() - 0.5f * resolution.swizzle<0, 1>());
  uv = 2.0f * uv.swizzle<0, 1>() / resolution.data[1];
  return uv;
}
f32x2 RotateUV(Context &__ctx, f32x2 &uv, float angle) {
  angle = angle * 3.14159265359f / 180.0f;
  return uv * f32x2x2(cos(angle), sin(angle), -sin(angle), cos(angle));
}
f32x3 Color255(Context &__ctx, int R, int G, int B) {
  return f32x3(float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f);
}
float Clamp01(Context &__ctx, float value) { return clamp(value, 0.0f, 1.0f); }
float sdHexagon(Context &__ctx, f32x2 p, float r) {
  const f32x3 k = f32x3(-0.866025404f, 0.5f, 0.577350269f);
  p = abs(p);
  p -= 2.0f * min(dot(k.swizzle<0, 1>(), p), 0.0f) * k.swizzle<0, 1>();
  p -= f32x2(clamp(p.data[0], -k.data[2] * r, k.data[2] * r), r);
  return length(p) * sign(p.data[1]);
}
float ndot(Context &__ctx, f32x2 a, f32x2 b) {
  return a.data[0] * b.data[0] - a.data[1] * b.data[1];
}
float sdRhombus(Context &__ctx, f32x2 p, f32x2 b) {
  p = abs(p);
  float h = clamp(ndot(__ctx, b - 2.0f * p, b) / dot(b, b), -1.0f, 1.0f);
  float d = length(p - 0.5f * b * f32x2(1.0f - h, 1.0f + h));
  return d * sign(p.data[0] * b.data[1] + p.data[1] * b.data[0] -
                  b.data[0] * b.data[1]);
}
float sdBox(Context &__ctx, f32x2 pos, f32x2 box) {
  f32x2 dist = abs(pos) - box;
  return length(max(dist, 0.0f)) + min(max(dist.data[0], dist.data[1]), 0.0f);
}
f32x3 Cube(Context &__ctx, f32x2 uv, float size, f32x3[3] colors,
           float blurLvl) {
  f32x3 result = f32x3(0.0f);
  float top =
      sdRhombus(__ctx, RotateUV(__ctx, uv, 000.0f) + f32x2(0.0f, -.6f / 1.732f),
                f32x2(.6f, .6f / 1.732f));
  float left =
      sdRhombus(__ctx, RotateUV(__ctx, uv, 120.0f) + f32x2(0.0f, -.6f / 1.732f),
                f32x2(.6f, .6f / 1.732f));
  float right = sdRhombus(
      __ctx, RotateUV(__ctx, uv, -120.0f) + f32x2(0.0f, -.6f / 1.732f),
      f32x2(.6f, .6f / 1.732f));
  result = mix(colors[0], result, Clamp01(__ctx, (top)*blurLvl));
  result = mix(colors[1], result, Clamp01(__ctx, (left)*blurLvl));
  result = mix(colors[2], result, Clamp01(__ctx, (right)*blurLvl));
  return result;
}
float CubeMask(Context &__ctx, f32x2 uv, float size, float blurLvl) {
  return Clamp01(__ctx,
                 (sdHexagon(__ctx, RotateUV(__ctx, uv, 30.0f), size * 0.5f)) *
                     blurLvl);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float time = mod(iTime, 3.f * 3.14159265359f);
  float size = 1.f + Clamp01(__ctx, (time * .5f) - 2.f) * sin(time) * .2f;
  float rounding = 0.1f;
  float blurLvl = 250.0f;
  f32x3 background = Color255(__ctx, 250, 250, 250);
  f32x3 shadow = Color255(__ctx, 0, 0, 0);
  f32x3 highlight = Color255(__ctx, 140, 140, 140);
  f32x3 midtone = Color255(__ctx, 76, 76, 76);
  f32x3[] colors = f32x3[](highlight, midtone, shadow);
  f32x2 uv = AspectRatioUV(__ctx, fragCoord, iResolution.swizzle<0, 1>());
  f32x3 pixel = background;
  float voidmask, square, line;
  pixel = mix(Clamp01(__ctx, time - 0.33f) *
                  Cube(__ctx, uv * (1.f / size), 0.f, colors, blurLvl),
              pixel, CubeMask(__ctx, uv, Clamp01(__ctx, time) * size, blurLvl));
  for (float i = 0.f; i < 3.f; i++) {
    square = sdRhombus(__ctx,
                       RotateUV(__ctx, uv, i * 120.f) +
                           Clamp01(__ctx, time - 1.f - (i * .33f)) *
                               f32x2(0.0f, -0.25f * size),
                       Clamp01(__ctx, time - 1.f - (i * .33f)) *
                           f32x2(.22f * size, .22f * size / 1.75f));
    voidmask += mix(1.f, 0.f, Clamp01(__ctx, (square - .01f) * blurLvl));
    line = sdBox(__ctx,
                 RotateUV(__ctx, uv, i * 120.f) +
                     Clamp01(__ctx, time - 2.f - (i * .33f)) *
                         f32x2(0.0f, -1.f * size),
                 Clamp01(__ctx, time - 2.f - (i * .33f)) *
                     f32x2(0.025f * size, 0.8f * size));
    voidmask += mix(1.f, 0.f, Clamp01(__ctx, (line - .01f) * blurLvl));
  }
  pixel = mix(pixel, background,
              Clamp01(__ctx, time - 1.66f) * Clamp01(__ctx, voidmask));
  fragColor = f32x4(pixel, 1.0f);
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
