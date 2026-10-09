// Port of https://www.shadertoy.com/view/cl3cD4; author: chenxianming.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cl3cD4.json.
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
float O(Context &__ctx, f32x3 p) { return length(p) - .25f; }
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h) - r;
}
float ct(Context &__ctx, f32x3 p, f32x3 p2) {
  return max(sdCapsule(__ctx, p, normalize(p2), f32x3(0), .01f),
             max(max(abs(p.data[0]) - .5f, abs(p.data[1]) - .5f),
                 abs(p.data[2]) - .5f));
}
float f(Context &__ctx, f32x3 p, f32x3 p2) {
  return min(O(__ctx, p2), ct(__ctx, p, p2));
}
void updateCR(Context &__ctx, f32x3 &a, f32x3 &b, f32x3 &c, float i) {
  a.set_swizzle<0, 1>(a.swizzle<0, 1>() *
                      (f32x2x2(cos(fract(iTime * .25f) * 3.1415926f * 4.f),
                               sin(fract(iTime * .25f) * 3.1415926f * 4.f),
                               -sin(fract(iTime * .25f) * 3.1415926f * 4.f),
                               cos(fract(iTime * .25f) * 3.1415926f * 4.f))));
  a.data[0] += (.3f * 2.f) * i - (.3f * (3.f - 1.f));
  b.data[0] += (.3f * 2.f) * i - (.3f * (3.f - 1.f));
  c.set_swizzle<0, 2>(c.swizzle<0, 2>() *
                      (f32x2x2(cos(fract(iTime * .25f) * 3.1415926f * 4.f),
                               sin(fract(iTime * .25f) * 3.1415926f * 4.f),
                               -sin(fract(iTime * .25f) * 3.1415926f * 4.f),
                               cos(fract(iTime * .25f) * 3.1415926f * 4.f))));
  c.data[0] += (.3f * 2.f) * i - (.3f * (3.f - 1.f));
}
void updateXY(Context &__ctx, f32x3 &a, f32x3 &b, float j) {
  a.set_swizzle<0, 1>(a.swizzle<0, 1>() *
                      (f32x2x2(cos(fract(iTime * .25f) * 3.1415926f * 4.f),
                               sin(fract(iTime * .25f) * 3.1415926f * 4.f),
                               -sin(fract(iTime * .25f) * 3.1415926f * 4.f),
                               cos(fract(iTime * .25f) * 3.1415926f * 4.f))));
  b.data[1] += (.3f * 2.f) * j - (.3f * (3.f - 1.f));
  a.data[1] += (.3f * 2.f) * j - (.3f * (3.f - 1.f));
}
void updateR(Context &__ctx, f32x3 &a, f32x3 &b, float h) {
  a.data[2] += (.3f * 2.f) * h - (.3f * (3.f - 1.f));
  b.set_swizzle<0, 2>(b.swizzle<0, 2>() *
                      (f32x2x2(cos(fract(iTime * .25f) * 3.1415926f * 4.f),
                               sin(fract(iTime * .25f) * 3.1415926f * 4.f),
                               -sin(fract(iTime * .25f) * 3.1415926f * 4.f),
                               cos(fract(iTime * .25f) * 3.1415926f * 4.f))));
  b.data[2] += (.3f * 2.f) * h - (.3f * (3.f - 1.f));
}
float df(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 2>(
      p.swizzle<0, 2>() *
      (f32x2x2(cos(fract(iTime * .25f * .5f) * 3.1415926f * 4.f),
               sin(fract(iTime * .25f * .5f) * 3.1415926f * 4.f),
               -sin(fract(iTime * .25f * .5f) * 3.1415926f * 4.f),
               cos(fract(iTime * .25f * .5f) * 3.1415926f * 4.f))));
  p.set_swizzle<2, 1>(
      p.swizzle<2, 1>() *
      (f32x2x2(cos(fract(iTime * .25f * .5f) * 3.1415926f * 4.f),
               sin(fract(iTime * .25f * .5f) * 3.1415926f * 4.f),
               -sin(fract(iTime * .25f * .5f) * 3.1415926f * 4.f),
               cos(fract(iTime * .25f * .5f) * 3.1415926f * 4.f))));
  float d = .1f, i, id;
  for (; i < 3.f; i++) {
    f32x3 p1 = p, p1a = p, p1b = p;
    updateCR(__ctx, p1a, p1, p1b, i);
    for (float j = 0.f; j < 3.f; j++) {
      f32x3 p2 = p, p2a = p;
      updateXY(__ctx, p2a, p2, j);
      for (float h = 0.f; h < 3.f; h++) {
        id++;
        f32x3 p3 = p, p3a = p;
        updateR(__ctx, p3, p3a, h);
        f32x3 m3 = f32x3(p1.data[0], p2.data[1], p3.data[2]);
        if (h == 0.f && (fract(iTime * .5f) < .75f && fract(iTime * .5f) > .5f))
          m3.data[0] = p1a.data[0], m3.data[1] = p2a.data[1];
        if (j == 1.f && (fract(iTime * .5f) > .75f && fract(iTime * .5f) < 1.f))
          m3.data[0] = p1b.data[0], m3.data[2] = p3a.data[2];
        if (id != 14.f)
          d = min(d, f(__ctx, p, m3));
      }
    }
  }
  return d;
}
void mainImage(Context &__ctx, f32x4 &U, f32x2 V) {
  f32x2 u = (V * 2.f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 c, p, o = f32x3(0.f, 0.f, -2.5f), r = f32x3(u, 1.f);
  float t, d, i, dp, e = sqrt(1e-4f);
  for (; i < 40.f; i++)
    p = o + r * t, d = df(__ctx, p), dp = (1.f - abs(d) / e),
    c += (dp > 0.f) ? .1f * (i / 40.f) * dp / t : 0.f,
    t += max(abs(d), e * .5f);
  U = f32x4(c, 1.0f);
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
