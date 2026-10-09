// Port of https://www.shadertoy.com/view/dsy3Dw; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dsy3Dw.json.
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
const float Repetitions = 16.0f;
float mod1(Context &__ctx, float &p, float size) {
  float halfsize = size * 0.5f;
  float c = floor((p + halfsize) / size);
  p = mod(p + halfsize, size) - halfsize;
  return c;
}
float dfcos(Context &__ctx, float x) {
  return sqrt(x * x + 1.0f) * 0.8f - 1.8f;
}
float dfcos(Context &__ctx, f32x2 p, float freq) {
  float x = p.data[0];
  float y = p.data[1];
  x *= freq;
  float x1 = abs(mod(x + 3.141592654f, (2.0f * 3.141592654f)) - 3.141592654f);
  float x2 = abs(mod(x, (2.0f * 3.141592654f)) - 3.141592654f);
  float a = 0.18f * freq;
  x1 /= max(y * a + 1.0f - a, 1.0f);
  x2 /= max(-y * a + 1.0f - a, 1.0f);
  return (mix(-dfcos(__ctx, x2) - 1.0f, dfcos(__ctx, x1) + 1.0f,
              clamp(y * 0.5f + 0.5f, 0.0f, 1.0f))) /
             max(freq * 0.8f, 1.0f) +
         max(abs(y) - 1.0f, 0.0f) * sign(y);
}
float pmin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
f32x2 toPolar(Context &__ctx, f32x2 p) {
  return f32x2(length(p), atan(p.data[1], p.data[0]));
}
f32x2 toRect(Context &__ctx, f32x2 p) {
  return p.data[0] * f32x2(cos(p.data[1]), sin(p.data[1]));
}
float modPolar(Context &__ctx, f32x2 &p, f32x2 pp, float add) {
  const float angle = (2.0f * 3.141592654f) / Repetitions;
  pp.data[1] += add;
  if (pp.data[1] > 3.141592654f) {
    pp.data[1] = -2.0f * 3.141592654f + pp.data[1];
  }
  float a = pp.data[1] + angle / 2.0f;
  float r = pp.data[0];
  float c = floor(a / angle);
  a = mod(a, angle) - (angle / 2.0f);
  p = f32x2(cos(a), sin(a)) * r;
  if (abs(c) >= (Repetitions / 2.0f))
    c = abs(c);
  return c;
}
float hash(Context &__ctx, float co) {
  return fract(sin(co * 12.9898f) * 13758.5453f);
}
f32x3 sRGB(Context &__ctx, f32x3 t) {
  return mix(1.055f * pow(t, f32x3(1.f / 2.4f)) - 0.055f, 12.92f * t,
             step(t, f32x3(0.0031308f)));
}
f32x3 aces_approx(Context &__ctx, f32x3 v) {
  v = max(v, 0.0f);
  v *= 0.6f;
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 pp) {
  const float lw = 0.01f;
  const float begin = 0.0f;
  const float end = 9.0f;
  const float reps = end - begin;
  p = p.swizzle<1, 0>();
  float aa = 4.0f / iResolution.data[1];
  f32x2 ppp = toPolar(__ctx, p);
  f32x3 col = f32x3(0.25f, 0.5f, 1.0f) * 0.1f;
  float d6 = 1E3f;
  float lp = length(p);
  float llp = dot(p, p);
  for (float i = begin; i < end; ++i) {
    f32x2 mp = p;
    float mn = modPolar(__ctx, mp, ppp,
                        i * ((2.0f * 3.141592654f) / (Repetitions * reps)));
    float h0 = hash(__ctx, 3667.0f * mn + 123.0f * i + 123.4f);
    float h1 = fract(3677.0f * h0);
    float h2 = fract(8677.0f * h0);
    f32x2 mp3 = mp;
    mp3.data[0] *= 0.9f + 0.2f * llp;
    mp3.data[0] += -0.04f * iTime * mix(1.0f, 4.0f, h0);
    float zz = 0.015f * mix(3.0f, 1.0f, h2 * h2);
    mp3 /= zz;
    float gd = (abs(dfcos(__ctx, mp3, 1.0f)) - mix(0.2f, 0.0f, lp));
    d6 = min(d6, abs(mp3.data[1]) * zz);
    f32x3 gcol = (1.0f + cos(f32x3(0.0f, 1.0f, 2.0f) +
                             mix(0.25f, 1.0f, h1) * (iTime + 23.0f))) *
                 0.05f;
    col += gcol / max(gd, mix(25E-3f, 2E-2f, lp));
  }
  float d0 = lp - 1.0f;
  float d1 = lp - 0.25f;
  float d2 = abs(d0 + lw) - lw;
  float d3 = abs(d1 + lw) - lw;
  float d4 = min(d2, d3);
  float d5 = max(d0 + lw, -(d1 + lw));
  d6 = max(d6, -(d0 + lw));
  d6 -= 0.0033f;
  col -= f32x3(0.0f, 1.0f, 2.0f).swizzle<2, 1, 0>() * 0.33f * llp;
  col =
      mix(col, 0.075f * f32x3(0.25f, 0.125f, 1.0f), smoothstep(0.0f, -aa, -d5));
  col = mix(col,
            (1.0f + cos(f32x3(0.0f, 1.0f, 2.0f) + 2.0f * length(p) +
                        abs(ppp.data[1]) - 0.5f * iTime)),
            smoothstep(0.0f, -aa, d6));
  col = aces_approx(__ctx, col);
  col = mix(col, f32x3(1.0f), smoothstep(0.0f, -aa, d4));
  col *= smoothstep(1.5f, 0.5f, length(pp));
  col = sRGB(__ctx, col);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  f32x2 pp = p;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = effect(__ctx, p, pp);
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
