// Port of https://www.shadertoy.com/view/7djBzm; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7djBzm.json.
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
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float mlength(Context &__ctx, f32x2 p) {
  return max(abs(p.data[0]), abs(p.data[1]));
}
float mlength(Context &__ctx, f32x3 p) {
  return max(abs(p.data[0]), max(abs(p.data[1]), abs(p.data[2])));
}
f32x3 distort(Context &__ctx, f32x3 p) {
  float o = 2.f * 3.14159f / 3.f;
  float t = 1.5f * length(p) - 0.2f * iTime;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (Rot(__ctx, t - o)));
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (Rot(__ctx, t)));
  p.set_swizzle<2, 0>(p.swizzle<2, 0>() * (Rot(__ctx, t + o)));
  return p;
}
float thc(Context &__ctx, float a, float b) {
  return tanh(a * cos(b)) / tanh(a);
}
float GetDist(Context &__ctx, f32x3 p) {
  f32x3 op = p;
  float cd = length(p);
  float mx = 0.5f + 0.5f * cos(iTime);
  float m1 = mix(abs(p.data[1]), length(p.swizzle<0, 2>()), mx);
  float m2 = mix(length(p.swizzle<0, 2>()), p.data[1], mx);
  float k = 0.f;
  f32x3 t1 = p;
  t1.set_swizzle<0, 1>(t1.swizzle<0, 1>() * (Rot(__ctx, iTime)));
  float a = atan(t1.data[0], t1.data[1]);
  float d0 = length(t1.swizzle<0, 2>()) - 2.f;
  f32x2 u = f32x2(d0, t1.data[1]);
  float d1 = 0.5f * length(u) + 0.0001f;
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (Rot(__ctx, -0.3f * iTime)));
  p = distort(__ctx, p);
  p = fract(8.f * p) - 0.5f;
  float e = 0.05f;
  float sd = e * mlength(__ctx, p) - 0.5f * e * length(p) + 0.0001f;
  float d = d1;
  d = sd;
  float m = 1.5f + 0.25f * cos(length(op.swizzle<0, 2>()) * 5.f - iTime);
  d = max(d, abs(abs(m - cd) - 0.3f) - 0.12f);
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float dO = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p);
    if (abs(dS) < 0.0001f || dO > 50.f)
      break;
    dO += dS * z;
  }
  return min(dO, 50.f);
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float t = 0.125f * iTime, o = 2.f * 3.14159f / 3.f;
  f32x3 ro = 3.6f * f32x3(cos(t - o), cos(t), cos(t + o));
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0), 0.95f);
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd, 1.f);
  if (d < 50.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    float v = smoothstep(0.f, 1.f, exp(-0.5f * length(p)));
    v *= v;
    f32x3 e = f32x3(1);
    col =
        v * pal(__ctx, 0.5f * v + 0.36f, e, e, e, 0.8f * f32x3(0, 1, 2) / 3.f);
    col += 0.1f;
  }
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
