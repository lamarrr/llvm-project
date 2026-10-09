// Port of https://www.shadertoy.com/view/msBfDh; author: LydianLights.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msBfDh.json.
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
f32x3 mod289(Context &__ctx, f32x3 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x4 mod289(Context &__ctx, f32x4 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
f32x4 taylorInvSqrt(Context &__ctx, f32x4 r) {
  return 1.79284291400159f - 0.85373472095314f * r;
}
float snoise(Context &__ctx, f32x3 v) {
  const f32x2 C = f32x2(1.0f / 6.0f, 1.0f / 3.0f);
  const f32x4 D = f32x4(0.0f, 0.5f, 1.0f, 2.0f);
  f32x3 i = floor(v + dot(v, C.swizzle<1, 1, 1>()));
  f32x3 x0 = v - i + dot(i, C.swizzle<0, 0, 0>());
  f32x3 g = step(x0.swizzle<1, 2, 0>(), x0.swizzle<0, 1, 2>());
  f32x3 l = 1.0f - g;
  f32x3 i1 = min(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 i2 = max(g.swizzle<0, 1, 2>(), l.swizzle<2, 0, 1>());
  f32x3 x1 = x0 - i1 + C.swizzle<0, 0, 0>();
  f32x3 x2 = x0 - i2 + C.swizzle<1, 1, 1>();
  f32x3 x3 = x0 - D.swizzle<1, 1, 1>();
  i = mod289(__ctx, i);
  f32x4 p = permute(
      __ctx,
      permute(__ctx,
              permute(__ctx,
                      i.data[2] + f32x4(0.0f, i1.data[2], i2.data[2], 1.0f)) +
                  i.data[1] + f32x4(0.0f, i1.data[1], i2.data[1], 1.0f)) +
          i.data[0] + f32x4(0.0f, i1.data[0], i2.data[0], 1.0f));
  float n_ = 0.142857142857f;
  f32x3 ns = n_ * D.swizzle<3, 1, 2>() - D.swizzle<0, 2, 0>();
  f32x4 j = p - 49.0f * floor(p * ns.data[2] * ns.data[2]);
  f32x4 x_ = floor(j * ns.data[2]);
  f32x4 y_ = floor(j - 7.0f * x_);
  f32x4 x = x_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 y = y_ * ns.data[0] + ns.swizzle<1, 1, 1, 1>();
  f32x4 h = 1.0f - abs(x) - abs(y);
  f32x4 b0 = f32x4(x.swizzle<0, 1>(), y.swizzle<0, 1>());
  f32x4 b1 = f32x4(x.swizzle<2, 3>(), y.swizzle<2, 3>());
  f32x4 s0 = floor(b0) * 2.0f + 1.0f;
  f32x4 s1 = floor(b1) * 2.0f + 1.0f;
  f32x4 sh = -step(h, f32x4(0.0f));
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
  f32x4 m = max(
      0.5f - f32x4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0f);
  m = m * m;
  return 105.0f *
         dot(m * m, f32x4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}
const f32x2 OFFSET = f32x2(69.0f, 420.0f);
const float FREQ = 2.0f;
const float AMPLITUDE = 1.0f;
const int OCTAVES = 4;
const float LACUNARITY = 2.0f;
const float GAIN = 0.5f;
float fbm(Context &__ctx, f32x2 uv) {
  float t = iTime / 2.0f;
  float value = 0.0f;
  float amplitude = AMPLITUDE;
  float freq = FREQ;
  for (int i = 0; i < OCTAVES; i++) {
    f32x2 p = freq * (uv + OFFSET);
    f32x3 q = f32x3(p.swizzle<0, 1>(), t);
    float n = snoise(__ctx, q);
    value += amplitude * n;
    freq *= LACUNARITY;
    amplitude *= GAIN;
  }
  return 0.5f * value + 0.5f;
}
float getNoise(Context &__ctx, f32x2 uv) {
  return fbm(__ctx, uv + fbm(__ctx, uv + fbm(__ctx, uv)));
}
f32x3 gamma(Context &__ctx, f32x3 color) {
  return pow(max(color, 0.0f), f32x3(1.0f / 2.2f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float n = getNoise(__ctx, uv);
  n = pow(n, 2.5f);
  f32x3 color = f32x3(n);
  color = gamma(__ctx, color);
  fragColor = f32x4(color, 1.0f);
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
