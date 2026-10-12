// Port of https://www.shadertoy.com/view/mlGczK; author: timmaffett.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlGczK.json.
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
float hash11(Context &__ctx, float p) {
  f32x3 p3 = fract(f32x3(p) * 443.8975f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 19.19f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float hash12(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * 443.8975f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 19.19f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float hash13(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * 443.8975f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 19.19f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
f32x4 hsb2rgb(Context &__ctx, f32x4 c) {
  f32x3 rgb = clamp(
      abs(mod(c.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
      0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return f32x4(c.data[2] * mix(f32x3(1.0f), rgb, c.data[1]), 1.f);
}
void mainImage(Context &__ctx, f32x4 &o, f32x2 u) {
  f32x3 R = iResolution, q, k;
  u = (u - R.swizzle<0, 1>() / 2.f) / R.data[1];
  o *= 0.f;
  float d, ss, s, f, i, j, t = iTime * .2f, e = 1e-2f;
  f32x2 mouse = (iMouse.swizzle<0, 1>() == f32x2(0.f))
                    ? f32x2(0.5f)
                    : iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float maxIter = 78.0f * mouse.data[0];
  while (i++ < maxIter) {
    q = f32x3(d * u, d),
    q.set_swizzle<0, 2>(q.swizzle<0, 2>() *
                        (f32x2x2(cos(cos(t) + f32x4(0, 11, 33, 0)))));
    q.set_swizzle<1, 2>(q.swizzle<1, 2>() *
                        (f32x2x2(cos(t + f32x4(0, 11, 33, 0)))));
    q.data[2] -= 2.f * t;
    k = floor(q) + .5f;
    q = fract(q) - .5f;
    q.set_swizzle<1, 2>(q.swizzle<1, 2>() *
                        (f32x2x2(cos(-t + f32x4(0, 11, 33, 0)))));
    q.set_swizzle<0, 2>(q.swizzle<0, 2>() *
                        (f32x2x2(cos(-cos(t) + f32x4(0, 11, 33, 0)))));
    q.set_swizzle<0, 1>(q.swizzle<0, 1>() *
                        (f32x2x2(cos(cos(k.data[0] + k.data[1] + k.data[2]) +
                                     f32x4(0, 11, 33, 0)))));
    s = length(max(abs(q.swizzle<2, 0>()), abs(q.data[1])) - .2f) - .01f - .05f;
    s = abs(s);
    f = 1.f - s / e,
    o += (f > .01f)
             ? hsb2rgb(
                   __ctx,
                   f32x4(3.141592653f /
                             hash13(__ctx,
                                    (cos((k) * 6.3f + f32x3(0, 23, 21)) * .5f +
                                     .5f)),
                         0.9f, 0.9f, 1)) *
                   f / d * i / 210.f
             : f32x4(0);
    d += max(s, e / 5.f);
  }
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
