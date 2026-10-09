// Port of https://www.shadertoy.com/view/ct2GDt; author: Varyemez.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ct2GDt.json.
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
const int MAX_STEP = 80;
const float SURFACE_DIST = 0.02f;
const float PI = radians(180.f);
const f32x2 torusr = f32x2(0.85f, 0.15f) * 0.5f;
f32x2x2 rot2d(Context &__ctx, float r) {
  float s = sin(r), c = cos(r);
  return f32x2x2(c, -s, s, c);
}
float random(Context &__ctx, f32x2 s) {
  return fract(sin(dot(s, f32x2(362.485f, 123.445f)) * 764.346f) * 2647.9752f);
}
float plane(Context &__ctx, f32x3 p) { return abs(p.data[1]); }
float sphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float torus(Context &__ctx, f32x3 p, float a, float b) {
  return length(f32x2(length(p.swizzle<0, 2>()) - a, p.data[1])) - b;
}
float cube(Context &__ctx, f32x3 p, float r) {
  p = abs(p);
  return length(max(p - r, 0.f));
}
float map(Context &__ctx, f32x3 p) {
  float angle = atan(1.f, 1.f);
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot2d(__ctx, angle)));
  p = fract(p);
  p -= 0.5f;
  float t0 = torus(__ctx, p, torusr.data[0], torusr.data[1]);
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rot2d(__ctx, PI * 0.5f)));
  p.data[2] -= 0.5f;
  float t1 = torus(__ctx, p, torusr.data[0], torusr.data[1]);
  p.data[2] += 1.f;
  float t2 = torus(__ctx, p, torusr.data[0], torusr.data[1]);
  return min(min(t0, t1), t2);
}
f32x3 get_normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(0.05f, 0.f);
  float d = map(__ctx, p);
  return -normalize(f32x3(d - map(__ctx, p + e.swizzle<0, 1, 1>()),
                          d - map(__ctx, p + e.swizzle<1, 0, 1>()),
                          d - map(__ctx, p + e.swizzle<1, 1, 0>())));
}
float raymarch(Context &__ctx, f32x3 vo, f32x3 dir) {
  float d = 0.f;
  for (int i = 0; i < MAX_STEP; i++) {
    float cd = map(__ctx, vo + dir * d);
    if (cd < SURFACE_DIST)
      break;
    d += cd;
  }
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  f32x3 r0 = f32x3(0.f, 1.f, 0.f);
  f32x3 dir = normalize(f32x3(uv.swizzle<0, 1>(), 1.f));
  r0.data[2] += iTime;
  float d = raymarch(__ctx, r0, dir);
  f32x3 p = r0 + dir * d;
  fragColor = f32x4(0.3f, 0.f, 1.f, 1.f) / max(0.5f, d);
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
