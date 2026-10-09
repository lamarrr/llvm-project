// Port of https://www.shadertoy.com/view/NsGfDt; author: intrakits.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NsGfDt.json.
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
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float sdCylinder(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ab, ap) / dot(ab, ab);
  f32x3 c = a + t * ab;
  float x = length(p - c) - r;
  float y = (abs(t - .5f) - .5f) * length(ab);
  float e = length(max(f32x2(x, y), 0.f));
  float i = min(max(x, y), 0.f);
  return e + i;
}
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ab, ap) / dot(ab, ab);
  t = clamp(t, 0.f, 1.f);
  f32x3 c = a + t * ab;
  float d = length(p - c) - r;
  return d;
}
float sdTorus(Context &__ctx, f32x3 p, f32x2 r) {
  float x = length(p.swizzle<0, 2>()) - r.data[0];
  return length(f32x2(x, p.data[1])) - r.data[1];
}
float dBox(Context &__ctx, f32x3 p, f32x3 s) {
  return length(max(abs(p) - s, 0.f));
}
float GetDist(Context &__ctx, f32x3 p) {
  f32x4 sphere = f32x4(0, 1, 6, 1);
  float ds = length(p - sphere.swizzle<0, 1, 2>()) - sphere.data[3];
  float dp = p.data[1];
  f32x3 p2 = p;
  p2 -= f32x3(2, 0, 6);
  p2.set_swizzle<0, 1>(p2.swizzle<0, 1>() * (Rot(__ctx, iTime)));
  float cd = sdCapsule(__ctx, p2, f32x3(0, 0.5f, 0), f32x3(0, 1.5f, 0), 0.5f);
  f32x3 p1 = p;
  p1 -= f32x3(-3, 0.5f, 6);
  p1.set_swizzle<0, 1>(p1.swizzle<0, 1>() * (Rot(__ctx, iTime)));
  float td = sdTorus(__ctx, p1, f32x2(.5f, .3f));
  float bd = dBox(__ctx, p - f32x3(-2, 0.5f, 8), f32x3(0.5f));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (Rot(__ctx, iTime)));
  float cyld = sdCylinder(__ctx, p, f32x3(0, .3f, 5), f32x3(3, 0.3f, 6), .3f);
  float d = min(ds, dp);
  d = min(cd, d);
  d = min(d, td);
  d = min(bd, d);
  d = min(cyld, d);
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + dO * rd;
    float ds = GetDist(__ctx, p);
    dO += ds;
    if (ds < 0.01f || dO > 100.f) {
      break;
    }
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(0.01f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
float GetLight(Context &__ctx, f32x3 p) {
  f32x3 lightPos = f32x3(0, 4, 0);
  f32x3 l = normalize(lightPos - p);
  f32x3 n = GetNormal(__ctx, p);
  float diff = clamp(dot(n, l), 0.f, 1.f);
  diff = smoothstep(0.4f, .41f, diff);
  return diff;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0);
  f32x3 ro = f32x3(0, 4, 0);
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1] - 0.5f, 1));
  float d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  float diff = GetLight(__ctx, p);
  col = f32x3(diff);
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
