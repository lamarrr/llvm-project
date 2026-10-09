// Port of https://www.shadertoy.com/view/DsSGDw; author: danzabarr.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DsSGDw.json.
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
float sphere(Context &__ctx, f32x3 p, f32x3 c, float r) {
  return length(p - c) - r;
}
float plane(Context &__ctx, f32x3 p, f32x3 n, float d) { return dot(p, n) - d; }
float quad(Context &__ctx, f32x3 p, f32x4x4 transform) { return 0.f; }
float metaball(Context &__ctx, f32x3 p, f32x3 c, float r) {
  return pow(r, 2.0f) /
         (pow(p.data[0] - c.data[0], 2.0f) + pow(p.data[1] - c.data[1], 2.0f) +
          pow(p.data[2] - c.data[2], 2.0f));
}
float aabb(Context &__ctx, f32x3 p, f32x3 pos, f32x3 size) {
  f32x3 q = abs(p - pos) - size;
  return length(max(q, 0.f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.f);
}
float GetDist(Context &__ctx, f32x3 p) {
  float d = 200.f;
  d = min(d, aabb(__ctx, p, f32x3(2.f, 2.f, 10.f), f32x3(3.f, 1.f, 5.f)));
  d = min(d, plane(__ctx, p, f32x3(0.f, 1.f, 0.f), 0.f));
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float da = 0.0f;
  for (int i = 0; i < 200; i++) {
    f32x3 ri = ro + rd * da;
    float d = GetDist(__ctx, ri);
    da += d;
    if (da > 200.f || d < .00001f)
      break;
  }
  return da;
}
float sdSphere(Context &__ctx, f32x3 p, float r) {
  f32x3 offset = f32x3(0, 0, -2);
  return length(p - offset) - r;
}
f32x3 GetNormal(Context &__ctx, f32x3 point) {
  float EPSILON = .00001f;
  float a = GetDist(
      __ctx, f32x3(point.data[0] + EPSILON, point.data[1], point.data[2]));
  float b = GetDist(
      __ctx, f32x3(point.data[0] - EPSILON, point.data[1], point.data[2]));
  float c = GetDist(
      __ctx, f32x3(point.data[0], point.data[1] + EPSILON, point.data[2]));
  float d = GetDist(
      __ctx, f32x3(point.data[0], point.data[1] - EPSILON, point.data[2]));
  float e = GetDist(
      __ctx, f32x3(point.data[0], point.data[1], point.data[2] + EPSILON));
  float f = GetDist(
      __ctx, f32x3(point.data[0], point.data[1], point.data[2] - EPSILON));
  return normalize(f32x3(a - b, c - d, e - f));
}
f32x3 GetNormalold(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(.00001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
float GetLight(Context &__ctx, f32x3 p) {
  f32x3 lightPos = f32x3(3, 15, 2);
  lightPos.set_swizzle<0, 2>(lightPos.swizzle<0, 2>() +
                             (f32x2(sin(iTime), cos(iTime)) * 2.f));
  f32x3 l = normalize(lightPos - p);
  f32x3 n = GetNormal(__ctx, p);
  float dif = clamp(dot(n, l), 0.f, 1.f);
  float d = RayMarch(__ctx, p + n * .00001f * 2.f, l);
  if (d < length(lightPos - p))
    dif *= .1f;
  return dif;
}
f32x3x3 rotateX(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x3x3(f32x3(1, 0, 0), f32x3(0, c, -s), f32x3(0, s, c));
}
f32x3x3 rotateY(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x3x3(f32x3(c, 0, s), f32x3(0, 1, 0), f32x3(-s, 0, c));
}
f32x3x3 rotateZ(Context &__ctx, float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return f32x3x3(f32x3(c, -s, 0), f32x3(s, c, 0), f32x3(0, 0, 1));
}
f32x2 r;
bool stored = false;
float PI = 3.1415926535f;
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0);
  f32x3 ro = f32x3(0, 5, 0);
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1], 1));
  float tilt = PI / 12.f;
  float pan = 0.f;
  rd *= rotateX(__ctx, tilt) * rotateY(__ctx, pan);
  float d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  float dif = GetLight(__ctx, p);
  col = f32x3(dif);
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
