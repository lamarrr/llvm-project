// Port of https://www.shadertoy.com/view/slXcR4; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slXcR4.json.
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
const f32x4 hsv2rgb_K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 p =
      abs(fract(c.swizzle<0, 0, 0>() + hsv2rgb_K.swizzle<0, 1, 2>()) * 6.0f -
          hsv2rgb_K.swizzle<3, 3, 3>());
  return c.data[2] * mix(hsv2rgb_K.swizzle<0, 0, 0>(),
                         clamp(p - hsv2rgb_K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
float sRGB(Context &__ctx, float t) {
  return mix(1.055f * pow(t, 1.f / 2.4f) - 0.055f, 12.92f * t,
             step(t, 0.0031308f));
}
f32x3 sRGB(Context &__ctx, f32x3 c) {
  return f32x3(sRGB(__ctx, c.data[0]), sRGB(__ctx, c.data[1]),
               sRGB(__ctx, c.data[2]));
}
float pmin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
f32x2 mod2(Context &__ctx, f32x2 &p, f32x2 size) {
  f32x2 c = floor((p + size * 0.5f) / size);
  p = mod(p + size * 0.5f, size) - size * 0.5f;
  return c;
}
float tanh_approx(Context &__ctx, float x) {
  float x2 = x * x;
  return clamp(x * (27.0f + x2) / (27.0f + 9.0f * x2), -1.0f, 1.0f);
}
f32x2 toPolar(Context &__ctx, f32x2 p) {
  return f32x2(length(p), atan(p.data[1], p.data[0]) + 3.141592654f);
}
f32x2 spiralEffect(Context &__ctx, f32x2 p, float a, float n) {
  f32x2 op = p;
  float b = a / (2.0f * 3.141592654f);
  f32x2 pp = toPolar(__ctx, op);
  float aa = pp.data[1];
  pp -= f32x2(pp.data[1] * n * b, (pp.data[0] / b + 3.141592654f) / n);
  f32x2 nn = mod2(__ctx, pp, f32x2(a, (2.0f * 3.141592654f) / n));
  float xf = tanh_approx(__ctx, 20.0f * length(p) / abs(n));
  return f32x2(abs(pp.data[0]) * xf, mod(nn.data[1], n));
}
float julia_map(Context &__ctx, f32x2 &p, f32x2 c) {
  float s = 1.0f;
  p.data[0] -= 0.5f;
  c = p;
  const int JULIA_ITERATIONS = 24;
  for (int i = 0; i < JULIA_ITERATIONS; ++i) {
    f32x2 p2 = p * p;
    p = f32x2(p2.data[0] - p2.data[1], 2.0f * p.data[0] * p.data[1]);
    p += c;
    s *= 1.9f;
    s *= sqrt(p2.data[0] + p2.data[1]);
  }
  return 1.0f / s;
}
f32x3 df(Context &__ctx, f32x2 p) {
  f32x2 c0 = f32x2(-0.7440f, .0148f);
  f32x2 c1 = 0.85f * f32x2(-0.45f, 0.75f);
  f32x2 c = mix(c0, c1, smoothstep(-1.0f, 1.0f, sin(0.5f * iTime)));
  float js = julia_map(__ctx, p, c);
  float cd = -(length(p) - 2.0f) * js;
  const float z = 0.25f;
  p /= z;
  float a = 0.5f;
  float sp0 = 11.0f;
  float sp1 = -3.0f;
  f32x2 se0 =
      spiralEffect(__ctx,
                   p * f32x2x2(cos(-0.123f * iTime), sin(-0.123f * iTime),
                               -sin(-0.123f * iTime), cos(-0.123f * iTime)),
                   a, sp0);
  f32x2 se1 = spiralEffect(__ctx,
                           p * f32x2x2(cos(.1f * iTime), sin(.1f * iTime),
                                       -sin(.1f * iTime), cos(.1f * iTime)),
                           a, sp1);
  f32x2 se = f32x2(pmin(__ctx, se0.data[0], se1.data[0], 0.125f),
                   se0.data[1] + se1.data[1]);
  float h = se.data[1] * 0.05f + iTime * 0.2f;
  float d = -((se.data[0]) - 0.05f);
  d *= z * js;
  return f32x3(d, cd, h);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float aa = 2.0f / iResolution.data[1];
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 d3 = df(__ctx, p);
  float d = d3.data[0];
  float cd = d3.data[1];
  float h = d3.data[2];
  float s = smoothstep(aa, -aa, d);
  f32x3 col = f32x3(0.0f);
  col = hsv2rgb(__ctx, f32x3(fract(h), 0.95f, s));
  col = mix(col, f32x3(0.0f), smoothstep(aa, -aa, cd));
  col = sRGB(__ctx, col);
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
