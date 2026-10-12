// Port of https://www.shadertoy.com/view/7lyyWD; author: zelect.
// Dataset license metadata: mit; original retained in upstream/7lyyWD.json.
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
float hash(Context &__ctx, float n) { return fract(sin(n) * 43758.5453f); }
float noise(Context &__ctx, f32x3 x) {
  f32x3 p = floor(x);
  f32x3 f = fract(x);
  f = f * f * (3.0f - 2.0f * f);
  float n = p.data[0] + p.data[1] * 57.0f + 113.0f * p.data[2];
  return mix(
      mix(mix(hash(__ctx, n + 0.0f), hash(__ctx, n + 1.0f), f.data[0]),
          mix(hash(__ctx, n + 57.0f), hash(__ctx, n + 58.0f), f.data[0]),
          f.data[1]),
      mix(mix(hash(__ctx, n + 113.0f), hash(__ctx, n + 114.0f), f.data[0]),
          mix(hash(__ctx, n + 170.0f), hash(__ctx, n + 171.0f), f.data[0]),
          f.data[1]),
      f.data[2]);
}
const float PI = 3.14159265359f;
const float hPI = 1.5707963268f;
float sdCylinder(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  const f32x2 c = f32x2(1.0f, 0.0f);
  const f32x2 smoothing = f32x2(-0.1f, 0.1f);
  f32x3 pa = p - a, ba = b - a;
  float tt = dot(pa, ba) / dot(ba, ba);
  float t = clamp(tt, 0.0f, 1.0f);
  f32x3 cap = pa - ba * t;
  f32x2 q = f32x2(length(cap.swizzle<0, 2>()), -cap.data[1]);
  float h =
      (sin(abs(-clamp((abs(tt - 0.5f) * 16.0f) - 8.0f, -hPI, hPI))) - 1.0f) *
      sign(-tt + 0.5f) * -((a.data[1] - b.data[1]) * 0.03f);
  float w = sqrt(r * r - h * h);
  float s =
      max((h - r) * q.data[0] * q.data[0] + w * w * (h + r - 2.0f * q.data[1]),
          h * q.data[0] - w * q.data[1]);
  float final = (s < 0.0f)        ? length(q) - r
                : (q.data[0] < w) ? h - q.data[1]
                                  : length(q - f32x2(w, h));
  return final;
}
float map(Context &__ctx, f32x3 pos) {
  return sdCylinder(
      __ctx, pos, f32x3(cos(iTime + 2.22f) * -0.3f, cos(iTime) * -0.3f, -0.1f),
      f32x3(0.3f, sin(iTime) * 0.5f, 0.3f), 0.15f);
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.0005f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>() * eps) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>() * eps) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>() * eps) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>() * eps));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float an = 0.5f * (iTime - 10.0f);
  f32x3 ro = f32x3(1.0f * cos(an), 0.4f, 1.0f * sin(an));
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  f32x2 p =
      (-iResolution.swizzle<0, 1>() + 2.0f * fragCoord) / iResolution.data[1];
  f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.5f * ww);
  const float tmax = 3.0f;
  float t = 0.0f;
  for (int i = 0; i < 256; i++) {
    f32x3 pos = ro + t * rd;
    float h = map(__ctx, pos);
    if (h < 0.0001f || t > tmax)
      break;
    t += h;
  }
  f32x3 col = f32x3(0.0f);
  if (t < tmax) {
    f32x3 pos = ro + t * rd;
    f32x3 nor = calcNormal(__ctx, pos);
    float dif = clamp(dot(nor, f32x3(0.57703f)), 0.0f, 1.0f);
    float amb = 0.5f + 0.5f * dot(nor, f32x3(0.0f, 1.0f, 0.0f));
    col = f32x3(0.2f, 0.3f, 0.4f) * amb + f32x3(0.8f, 0.7f, 0.5f) * dif;
  }
  col = sqrt(col);
  tot += col;
  fragColor = f32x4(tot, 1.0f);
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
