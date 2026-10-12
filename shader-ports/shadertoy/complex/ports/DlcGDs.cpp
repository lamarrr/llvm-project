// Port of https://www.shadertoy.com/view/DlcGDs; author: FTPlus.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlcGDs.json.
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
float sdfArc(Context &__ctx, f32x2 I, f32x2 A, f32x2 B, float s) {
  f32x2 v = .5f * (B - A);
  float p = abs(s), k = dot(v, v), l = sqrt(k) * sign(s), r = .5f * (k / p + p);
  f32x2 n = f32x2(-v.data[1], v.data[0]) / l, C = A + v + n * (r - p);
  float d = length(C - I);
  if (dot(n, C - I) * r < (r - p) * d)
    return sqrt(min(dot(A - I, A - I), dot(B - I, B - I)));
  return abs(d - r);
}
float ramp(Context &__ctx, float x) {
  x = abs(x);
  float y = 0.f;
  for (int i = 1; i < 7; ++i)
    y += (1.f / float(i)) * sin(x * float(i));
  return y * .5f + .5f;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 I) {
  float w = min(iResolution.data[0], iResolution.data[1]);
  I -= iResolution.swizzle<0, 1>() / 2.f;
  float t = iTime + 25.f;
  float tA = t / -5.f;
  float tB = t / -7.f;
  float ts = t / 10.f;
  f32x2 A = f32x2(cos(tA), sin(tA)) * .4f * w;
  f32x2 B = f32x2(cos(tB), sin(tB)) * .2f * w;
  float s = sin(ts) * .5f * w;
  if (iMouse.data[2] >= 0.f)
    s = iMouse.data[1] - .5f * w;
  float d = sdfArc(__ctx, I, A, B, s) - .04f * w;
  f32x3 color = d >= 0.f ? f32x3(.4f, .9f, .3f) : f32x3(.9f, .3f, .4f);
  f32x3 field =
      f32x3(.17f, .13f, .15f) +
      (f32x3(1.f) - f32x3(.17f, .13f, .15f)) * color * ramp(__ctx, d / 5.f);
  f32x3 image =
      mix(field, (f32x3(1.f) - f32x3(.17f, .13f, .15f)) * f32x3(.4f, .3f, .9f),
          smoothstep(2.f, 0.f, abs(d) - 1.f));
  O = f32x4(image, 1.f);
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
