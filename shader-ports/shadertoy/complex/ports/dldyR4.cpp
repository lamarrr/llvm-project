// Port of https://www.shadertoy.com/view/dldyR4; author: trooper905.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dldyR4.json.
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
f32x3 palette(Context &__ctx, float t) {
  f32x3 a = f32x3(0.158f, 1.102f, 0.468f);
  f32x3 b = f32x3(0.502f, 0.502f, 1.000f);
  f32x3 c = f32x3(0.428f, 1.000f, 1.008f);
  f32x3 d = f32x3(0.000f, 0.538f, 0.312f);
  return a + b * cos(6.28318f * (c * t + d));
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float sdEquilateralTriangle(Context &__ctx, f32x2 p, float r) {
  const float k = sqrt(3.0f);
  p.data[0] = abs(p.data[0]) - r;
  p.data[1] = p.data[1] + r / k;
  if (p.data[0] + k * p.data[1] > 0.0f)
    p = f32x2(p.data[0] - k * p.data[1], -k * p.data[0] - p.data[1]) / 2.0f;
  p.data[0] -= clamp(p.data[0], -2.0f * r, 0.0f);
  return -length(p) * sign(p.data[1]);
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
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.0f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 uv0 = uv;
  f32x2 uv2 = uv;
  f32x2 uv3 = uv;
  f32x2 uv4 = uv;
  f32x3 finalColor = f32x3(0.0f);
  f32x3 mask = f32x3(0.0f);
  uv = fract(uv * 1.f) - 0.5f;
  f32x3 col;
  float size = .5f;
  float sizeB = .8f;
  float f = 0.f;
  float d = 0.f;
  float sq = abs(sin(sizeB + iTime * .3f) / 2.5f);
  float bounds = sdBox(__ctx, uv0, f32x2(sizeB, sizeB));
  bounds = smoothstep(0.05f, 0.f, bounds);
  for (float i = 0.0f; i < 4.0f; i++) {
    uv = fract(uv * 1.4f) - 0.5f;
    uv2 = fract(uv2 * 2.f) - 0.5f;
    uv2.data[0] = fract(uv2.data[0] - (iTime * .1f)) - 0.5f;
    uv3 = fract(uv3 * 1.f - (iTime * .1f)) - 0.5f;
    uv4 = fract(uv4 * .5f) - 0.5f;
    float d = sdBox(__ctx, uv, f32x2(.1f, .1f));
    float f = sdBox(__ctx, uv3, f32x2(size, size));
    float e = sdBox(__ctx, uv0, f32x2(size - .2f, size - .2f));
    float y = sdBox(__ctx, uv3, f32x2(sq, sq));
    float z = sdBox(__ctx, uv4, f32x2(sq, sq));
    z = abs(z);
    z = smoothstep(0.f, .1f, z);
    z = (.02f / z);
    y = abs(y);
    y = smoothstep(0.f, .1f, y);
    y = .005f / y;
    d += length(d) * exp(length(uv0));
    e += length(f) * exp(length(uv0));
    e += d * exp(length(uv0));
    f = length(f);
    float tri = sdEquilateralTriangle(__ctx, uv0, .7f);
    float di = sdRhombus(__ctx, uv0, f32x2(sq * 2.f, sq * 2.f));
    di = abs(di);
    di = smoothstep(0.f, .1f, di);
    di = .01f / di;
    d = sin(d * 6.f + iTime) / 3.f;
    d = abs(d);
    d = pow(0.06f / d, 1.0f);
    float g = (pow(0.03f / f, 2.f));
    f = asin(pow(0.01f / f, 1.f));
    e = (pow(0.03f / e, 1.f));
    f32x3 col = palette(__ctx, length(uv0) + iTime * .5f);
    float final = (.01f / (d / z) + di);
    mask += final;
    finalColor += (col * final) * bounds;
  }
  fragColor = f32x4(finalColor, 1.0f);
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
