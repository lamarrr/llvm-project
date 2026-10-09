// Port of https://www.shadertoy.com/view/DsX3D7; author: Aaras.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DsX3D7.json.
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
  float s = sin(a);
  float c = cos(a);
  return f32x2x2(c, -s, s, c);
}
f32x3 R(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = p + f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i - p);
  return d;
}
float Sphere(Context &__ctx, f32x3 p, f32x3 o, float radius) {
  return length(p - o) - radius;
}
float Plane(Context &__ctx, f32x3 p, float h) { return p.data[1] - h; }
float Box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float Cylinder(Context &__ctx, f32x3 p, f32x3 c) {
  return length(p.swizzle<0, 2>() - c.swizzle<0, 1>()) - c.data[2];
}
float Capsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 pa = p - a, ba = b - a;
  float h = dot(pa, ba) / dot(ba, ba);
  return length(pa - ba * h) - r;
}
f32x3 RandomVector(Context &__ctx, float i) {
  return normalize(f32x3(sin(329.34f + 131.8f * sin(722.1f * i + 8.23f)),
                         sin(494.34f + 2871.1f * sin(431.1f * i + 415.39f)),
                         sin(757.34f + 9539.1f * sin(161.1f * i + 201.39f))));
}
float SmoothMin(Context &__ctx, float a, float b) {
  float k = 3.0f;
  float h = pow(max((k - abs(a - b)) / k, 0.0f), 3.0f) * k / 6.0f;
  return min(a, b) - h;
}
float GetDist(Context &__ctx, f32x3 p) {
  float displacement = sin(5.0f * p.data[0]) * sin(5.0f * p.data[1]) *
                       sin(5.0f * p.data[2]) * 0.25f;
  p += displacement;
  float dSphere = Sphere(__ctx, p, f32x3(0.0f, 0.0f, 0.0f),
                         2.0f + (sin(iTime * 2.0f) * 0.5f));
  float d = dSphere;
  for (float i = 0.0f; i < 3.0f; i += 0.1f) {
    f32x3 pos = RandomVector(__ctx, i) + f32x3(sin(iTime + i * 4.0f));
    float off = sin(iTime * 5.0f + floor(i * 4.0f)) * 0.5f + 1.5f;
    pos *= off * 2.0f;
    float dSmallSphere = Sphere(__ctx, p, pos, 0.1f);
    d = SmoothMin(__ctx, d, dSmallSphere);
  }
  return d;
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.0f;
  float d;
  float minD = 10000.0f;
  f32x3 p;
  for (int i = 0; i < 100; i++) {
    p = ro + rd * dO;
    d = GetDist(__ctx, p);
    if (d < minD)
      minD = d;
    dO += d;
    if (abs(dO) < .1f || dO > 100.0f)
      break;
  }
  return f32x2(dO, minD);
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(.01f, .0f);
  float d = GetDist(__ctx, p);
  f32x3 n = f32x3(d - GetDist(__ctx, p - f32x3(e.swizzle<0, 1, 1>())),
                  d - GetDist(__ctx, p - f32x3(e.swizzle<1, 0, 1>())),
                  d - GetDist(__ctx, p - f32x3(e.swizzle<1, 1, 0>())));
  return normalize(n);
}
float GetShadow(Context &__ctx, f32x3 p, f32x3 s) {
  float d = RayMarch(__ctx, p, s).data[0];
  float ds = length(f32x3(-5.0f, 10.0f, -5.0f) - p);
  return step(.1f, d - ds);
}
float GetLight(Context &__ctx, f32x3 p) {
  if (length(p) > 100.0f / 2.0f)
    return 0.0f;
  f32x3 s = normalize(f32x3(-5.0f, 10.0f, -5.0f) - p);
  f32x3 n = GetNormal(__ctx, p);
  float l = dot(n, s);
  l = clamp(l, 0.0f, 1.0f);
  p += n * .1f * 3.0f;
  float sh = 1.0f;
  GetShadow(__ctx, p, s) + .1f;
  return l * sh;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 c = f32x3(0.0f);
  f32x3 ro = f32x3(0, 4, -5);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() * (Rot(__ctx, sin(iTime) * .1f)));
  ro.set_swizzle<0, 2>(
      ro.swizzle<0, 2>() *
      (Rot(__ctx,
           sin(iTime / 2.0f) * 3.1415926535897932384626433832795f / 2.0f)));
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 0, 0), .7f);
  f32x2 d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d.data[0];
  c = f32x3(abs(GetLight(__ctx, p) + 0.5f)) / 1.5f;
  c *= uv.swizzle<0, 1, 0>() + f32x3(0.5f);
  fragColor = f32x4(c, 1.0f);
  if (d.data[0] > 100.0f)
    fragColor = f32x4(0.1f);
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
