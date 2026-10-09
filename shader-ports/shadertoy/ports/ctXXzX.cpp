// Port of https://www.shadertoy.com/view/ctXXzX; author: Ridah.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctXXzX.json.
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

uniform f32x2 u_mouse;
uniform float u_time;
f32x3 mod289(Context &__ctx, f32x3 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x2 mod289(Context &__ctx, f32x2 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x3 permute(Context &__ctx, f32x3 x) {
  return mod289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
float snoise(Context &__ctx, f32x2 v) {
  const f32x4 C = f32x4(0.211324865405187f, 0.366025403784439f,
                        -0.577350269189626f, 0.024390243902439f);
  f32x2 i = floor(v + dot(v, C.swizzle<1, 1>()));
  f32x2 x0 = v - i + dot(i, C.swizzle<0, 0>());
  f32x2 i1;
  i1 = (x0.data[0] > x0.data[1]) ? f32x2(1.0f, 0.0f) : f32x2(0.0f, 1.0f);
  f32x4 x12 = x0.swizzle<0, 1, 0, 1>() + C.swizzle<0, 0, 2, 2>();
  x12.set_swizzle<0, 1>(x12.swizzle<0, 1>() - (i1));
  i = mod289(__ctx, i);
  f32x3 p =
      permute(__ctx, permute(__ctx, i.data[1] + f32x3(0.0f, i1.data[1], 1.0f)) +
                         i.data[0] + f32x3(0.0f, i1.data[0], 1.0f));
  f32x3 m = max(0.5f - f32x3(dot(x0, x0),
                             dot(x12.swizzle<0, 1>(), x12.swizzle<0, 1>()),
                             dot(x12.swizzle<2, 3>(), x12.swizzle<2, 3>())),
                0.0f);
  m = m * m;
  m = m * m;
  f32x3 x = 2.0f * fract(p * C.swizzle<3, 3, 3>()) - 1.0f;
  f32x3 h = abs(x) - 0.5f;
  f32x3 ox = floor(x + 0.5f);
  f32x3 a0 = x - ox;
  m *= 1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h);
  f32x3 g;
  g.data[0] = a0.data[0] * x0.data[0] + h.data[0] * x0.data[1];
  g.set_swizzle<1, 2>(a0.swizzle<1, 2>() * x12.swizzle<0, 2>() +
                      h.swizzle<1, 2>() * x12.swizzle<1, 3>());
  return 130.0f * dot(m, g);
}
float lerp(Context &__ctx, float a, float b, float t) {
  return (1.f - t) * a + t * b;
}
float hash(Context &__ctx, float x, float y) {
  return fract(abs(sin(sin(123.321f + x) * (y + 321.123f)) * 456.654f));
}
float perlin(Context &__ctx, float x, float y) {
  float col = 0.0f;
  for (int i = 0; i < 8; i++) {
    float fx = floor(x);
    float fy = floor(y);
    float cx = ceil(x);
    float cy = ceil(y);
    float a = hash(__ctx, fx, fy);
    float b = hash(__ctx, fx, cy);
    float c = hash(__ctx, cx, fy);
    float d = hash(__ctx, cx, cy);
    col += lerp(__ctx, lerp(__ctx, a, b, fract(y)), lerp(__ctx, c, d, fract(y)),
                fract(x));
    col /= 2.0f;
    x /= 2.0f;
    y /= 2.0f;
  }
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 pos = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float scale = 2.f;
  f32x3 color;
  float noise;
  pos *= scale;
  float d = snoise(__ctx, pos.swizzle<0, 1>()) * scale;
  float u = (pos.data[0] + d);
  float v = (pos.data[1] + d);
  d = snoise(__ctx, f32x2(u, v)) * scale;
  noise = snoise(__ctx, pos.swizzle<0, 1>() * sin(pos.data[1]) +
                            f32x2(d * sin(iTime) + iTime * 1.f));
  color = f32x3(.6f * (f32x3(2.f) * noise -
                       f32x3(noise * .1f, noise * .2f - sin(u / 30.f) * .1f,
                             noise * .3f + sin(v / 40.f) * .2f)));
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
