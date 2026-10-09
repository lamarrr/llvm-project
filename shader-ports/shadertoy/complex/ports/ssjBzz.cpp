// Port of https://www.shadertoy.com/view/ssjBzz; author: SnoopethDuckDuck.
// Dataset license metadata: mit; original retained in upstream/ssjBzz.json.
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
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float mlength(Context &__ctx, f32x2 uv) {
  return max(abs(uv.data[0]), abs(uv.data[1]));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
float smin(Context &__ctx, float a, float b) {
  float k = 0.12f;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
f32x2 hash(Context &__ctx, f32x2 p) {
  p = f32x2(dot(p, f32x2(127.1f, 311.7f)), dot(p, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(p) * 43758.5453123f);
}
float noise(Context &__ctx, f32x2 p) {
  const float K1 = 0.366025404f;
  const float K2 = 0.211324865f;
  f32x2 i = floor(p + (p.data[0] + p.data[1]) * K1);
  f32x2 a = p - i + (i.data[0] + i.data[1]) * K2;
  float m = step(a.data[1], a.data[0]);
  f32x2 o = f32x2(m, 1.0f - m);
  f32x2 b = a - o + K2;
  f32x2 c = a - 1.0f + 2.0f * K2;
  f32x3 h = max(0.5f - f32x3(dot(a, a), dot(b, b), dot(c, c)), 0.0f);
  f32x3 n = h * h * h * h *
            f32x3(dot(a, hash(__ctx, i + 0.0f)), dot(b, hash(__ctx, i + o)),
                  dot(c, hash(__ctx, i + 1.0f)));
  return dot(n, f32x3(70.0f));
}
f32x2x2 Rot(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float f = 0.0f;
  uv *= 4.0f;
  float a = 3.14159f + 0.5f * tanh(4.f * cos(iTime)) / tanh(4.f);
  float c = cos(a), s = sin(a);
  f32x2x2 rot = f32x2x2(c, s, -s, c);
  float m = 0.5f;
  float n = 6.f;
  float am = 10.f;
  float fr = fract(0.8f * iTime / am);
  float poop = 1.5f + floor(am * fr);
  poop = mix(poop, poop + 1.f, fr);
  float test =
      poop * step(0.5f, fract(0.25f + 0.5f * uv.data[1] +
                              0.05f * cos(iTime + 0.2f * uv.data[0]))) -
      1.f;
  for (float i = 0.f; i < n; i++) {
    a = (n - i) / n * 0.006f * iTime;
    uv += 10.f * a * f32x2(cos(a), sin(a));
    f += m * noise(__ctx, uv);
    uv += (5.f + 0.45f * cos(uv.data[0] * 2.f + test + 0.2f * iTime)) * cos(f);
  }
  f = 0.5f + 0.5f * f;
  f = smoothstep(0.f, 1.f, f);
  float k = 6.f;
  f = smoothstep(-k, k, f);
  f32x3 e = f32x3(1);
  f32x3 col =
      pal(__ctx, 0.755f + test * f, e, e, e, 0.36f * f32x3(0, 1, 2) / 3.f);
  float mx =
      0.5f + 0.5f * tanh(4.f * cos(0.12f * uv.data[0] + iTime)) / tanh(4.f);
  col *= mix(1.f, test, mx);
  col = sqrt(col) + 0.05f;
  fragColor = f32x4(col, 1.0f);
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
