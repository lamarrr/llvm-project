// Port of https://www.shadertoy.com/view/DdBBW3; author: alphardex.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdBBW3.json.
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

const float PI = 3.14159265359f;
f32x3 mod289(Context &__ctx, f32x3 x) {
  return x - floor(x * (1.f / 289.f)) * 289.f;
}
f32x4 mod289(Context &__ctx, f32x4 x) {
  return x - floor(x * (1.f / 289.f)) * 289.f;
}
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod289(__ctx, ((x * 34.f) + 1.f) * x);
}
f32x4 taylorInvSqrt(Context &__ctx, f32x4 r) {
  return 1.79284291400159f - .85373472095314f * r;
}
float snoise(Context &__ctx, f32x3 v) {
  const f32x2 C = f32x2(1.f / 6.f, 1.f / 3.f);
  const f32x4 D = f32x4(0.f, .5f, 1.f, 2.f);
  f32x3 i = floor(v + dot(v, C.swizzle<1, 1, 1>()));
  f32x3 x0 = v - i + dot(i, C.swizzle<0, 0, 0>());
  f32x3 g = step(x0.swizzle<1, 2, 0>(), x0.swizzle<0, 1, 2>());
  f32x3 l = 1.f - g;
  f32x3 i1 = min(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 i2 = max(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 x1 = x0 - i1 + C.swizzle<0, 0, 0>();
  f32x3 x2 = x0 - i2 + C.swizzle<1, 1, 1>();
  f32x3 x3 = x0 - D.swizzle<1, 1, 1>();
  i = mod289(__ctx, i);
  f32x4 p = permute(
      __ctx,
      permute(__ctx, permute(__ctx, i.data[2] + f32x4(0.f, i1.data[2],
                                                      i2.data[2], 1.f)) +
                         i.data[1] + f32x4(0.f, i1.data[1], i2.data[1], 1.f)) +
          i.data[0] + f32x4(0.f, i1.data[0], i2.data[0], 1.f));
  float n_ = .142857142857f;
  f32x3 ns = n_ * D.swizzle<3, 1, 2>() - D.swizzle<0, 2, 0>();
  f32x4 j = p - 49.f * floor(p * ns.data[2] * ns.data[2]);
  f32x4 x_ = floor(j * ns.data[2]);
  f32x4 y_ = floor(j - 7.f * x_);
  f32x4 x = x_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 y = y_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 h = 1.f - abs(x) - abs(y);
  f32x4 b0 = f32x4(x.swizzle<0, 1>(), y.swizzle<0, 1>());
  f32x4 b1 = f32x4(x.swizzle<2, 3>(), y.swizzle<2, 3>());
  f32x4 s0 = floor(b0) * 2.f + 1.f;
  f32x4 s1 = floor(b1) * 2.f + 1.f;
  f32x4 sh = -step(h, f32x4(0.f));
  f32x4 a0 = b0.swizzle<0, 2, 1, 3>() +
             s0.swizzle<0, 2, 1, 3>() * sh.swizzle<0, 0, 1, 1>();
  f32x4 a1 = b1.swizzle<0, 2, 1, 3>() +
             s1.swizzle<0, 2, 1, 3>() * sh.swizzle<2, 2, 3, 3>();
  f32x3 p0 = f32x3(a0.swizzle<0, 1>(), h.data[0]);
  f32x3 p1 = f32x3(a0.swizzle<2, 3>(), h.data[1]);
  f32x3 p2 = f32x3(a1.swizzle<0, 1>(), h.data[2]);
  f32x3 p3 = f32x3(a1.swizzle<2, 3>(), h.data[3]);
  f32x4 norm = taylorInvSqrt(
      __ctx, f32x4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
  p0 *= norm.data[0];
  p1 *= norm.data[1];
  p2 *= norm.data[2];
  p3 *= norm.data[3];
  f32x4 m =
      max(.6f - f32x4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.f);
  m = m * m;
  return 42.f *
         dot(m * m, f32x4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}
float fbm(Context &__ctx, f32x3 p, float progress) {
  float value = 0.f;
  float amplitude = 1.f;
  float frequency = 1.f;
  float lacunarity = 2.f;
  float persistance = .5f;
  float scale = mix(0.f, 600.f, progress);
  int octaves = 10;
  for (int i = 0; i < octaves; i++) {
    float noiseVal = snoise(__ctx, p * frequency * scale);
    value += amplitude * noiseVal;
    frequency *= lacunarity;
    amplitude *= persistance;
  }
  return value;
}
f32x4 getFromColor(Context &__ctx, f32x2 uv) {
  return texture(__ctx.channel0, uv);
}
float remap(Context &__ctx, float a, float b, float c, float d, float t) {
  return clamp((t - a) / (b - a), 0.f, 1.f) * (d - c) + c;
}
f32x4 transition(Context &__ctx, f32x2 uv) {
  float progress = abs(sin(iTime * .5f));
  float ratio = iResolution.data[0] / iResolution.data[1];
  f32x2 p = uv;
  p -= .5f;
  p.data[0] *= ratio;
  float noise = fbm(__ctx, f32x3(p, 0.f), progress);
  noise = remap(__ctx, -1.f, 1.f, 0.f, 1.f, noise);
  float pr = smoothstep(progress - .01f, progress, noise);
  pr *= step(.1f, 1.f - progress);
  return mix(getFromColor(__ctx, uv), f32x4(0.f), pr);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x4 col = transition(__ctx, uv);
  fragColor = col;
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
