// Port of https://www.shadertoy.com/view/clB3Dt; author: Karp.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clB3Dt.json.
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
f32x2 RotateUV(Context &__ctx, f32x2 &uv, float angle) {
  angle = angle * 3.14159265359f / 180.0f;
  return uv * f32x2x2(cos(angle), sin(angle), -sin(angle), cos(angle));
}
f32x3 Color255(Context &__ctx, int R, int G, int B) {
  return f32x3(float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f);
}
float Clamp01(Context &__ctx, float value) { return clamp(value, 0.0f, 1.0f); }
float sdEquilateralTriangle(Context &__ctx, f32x2 p, float r) {
  const float k = sqrt(3.0f);
  p.data[0] = abs(p.data[0]) - r;
  p.data[1] = p.data[1] + r / k;
  if (p.data[0] + k * p.data[1] > 0.0f)
    p = f32x2(p.data[0] - k * p.data[1], -k * p.data[0] - p.data[1]) / 2.0f;
  p.data[0] -= clamp(p.data[0], -2.0f * r, 0.0f);
  return -length(p) * sign(p.data[1]);
}
float sdBox(Context &__ctx, f32x2 pos, f32x2 box) {
  f32x2 dist = abs(pos) - box;
  return length(max(dist, 0.0f)) + min(max(dist.data[0], dist.data[1]), 0.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>();
  uv = 2.0f * uv.swizzle<0, 1>() / iResolution.data[1];
  float t = mod(iTime * 0.5f, 5.0f);
  t = abs(sin(iTime * 0.25f) * 4.f) - .5f;
  float angle = sin(-iTime) * 6.66f * mix(0.0f, 1.0f, Clamp01(__ctx, t - 2.0f));
  uv = RotateUV(__ctx, uv, angle);
  uv.data[0] += sin(-iTime) * 0.1f * mix(0.0f, 1.0f, Clamp01(__ctx, t - 2.0f));
  uv.data[1] += cos(-iTime) * 0.1f * mix(0.0f, 1.0f, Clamp01(__ctx, t - 2.0f));
  f32x2 uvL = RotateUV(__ctx, uv, -120.0f);
  f32x2 uvR = RotateUV(__ctx, uv, 120.0f);
  float dist = 0.15f;
  float size = 0.6f;
  float borderSize = 0.1f;
  float rounding = 0.1f;
  float blurLvl = 100.0f;
  float triA = sdEquilateralTriangle(__ctx, uv + f32x2(0.0f, dist),
                                     mix(-dist, size, Clamp01(__ctx, t)));
  float triB = sdEquilateralTriangle(__ctx, uvL + f32x2(0.0f, dist),
                                     mix(-dist, size, Clamp01(__ctx, t)));
  float triC = sdEquilateralTriangle(__ctx, uvR + f32x2(0.0f, dist),
                                     mix(-dist, size, Clamp01(__ctx, t)));
  float rectA =
      sdBox(__ctx, uv + f32x2(0.0f, -size - dist + borderSize - 0.02f),
            f32x2(mix(-1.0f, borderSize, Clamp01(__ctx, t - .33f))));
  float rectB =
      sdBox(__ctx, uvL + f32x2(0.0f, -size - dist + borderSize - 0.02f),
            f32x2(mix(-1.0f, borderSize, Clamp01(__ctx, t - .44f))));
  float rectC =
      sdBox(__ctx, uvR + f32x2(0.0f, -size - dist + borderSize - 0.02f),
            f32x2(mix(-1.0f, borderSize, Clamp01(__ctx, t - .55f))));
  float diagMask =
      sdBox(__ctx, uvR + f32x2(0.0f, 0.18f),
            f32x2(1.0f, mix(-1.0f, 0.025f, Clamp01(__ctx, (t * .33f) + .25f))));
  float lineMask =
      sdBox(__ctx, uv + f32x2(0.3f, 0.18f),
            f32x2(0.55f, mix(-1.0f, 0.025f, Clamp01(__ctx, (t * .33f) + .1f))));
  f32x3 bgCol = f32x3(.0902f);
  f32x3 logoCol = Color255(__ctx, 19, 175, 240);
  f32x3 pixel = bgCol;
  pixel = mix(logoCol, pixel, Clamp01(__ctx, (triA - rounding) * blurLvl));
  pixel = mix(logoCol, pixel, Clamp01(__ctx, (triB - rounding) * blurLvl));
  pixel = mix(logoCol, pixel, Clamp01(__ctx, (triC - rounding) * blurLvl));
  pixel = mix(logoCol, pixel, Clamp01(__ctx, (rectA - rounding) * blurLvl));
  pixel = mix(logoCol, pixel, Clamp01(__ctx, (rectB - rounding) * blurLvl));
  pixel = mix(logoCol, pixel, Clamp01(__ctx, (rectC - rounding) * blurLvl));
  pixel = mix(bgCol, pixel, Clamp01(__ctx, (diagMask - 0.1f) * blurLvl));
  pixel = mix(bgCol, pixel, Clamp01(__ctx, (lineMask - 0.1f) * blurLvl));
  f32x4 result = f32x4(pixel, 1.0f);
  fragColor = result;
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
