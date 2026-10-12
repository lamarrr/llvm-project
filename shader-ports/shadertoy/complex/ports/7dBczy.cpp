// Port of https://www.shadertoy.com/view/7dBczy; author: Del.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7dBczy.json.
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
f32x2 gm;
f32x2 smoothRot(Context &__ctx, f32x2 p, float s, float m, float c, float d) {
  s *= 0.5f;
  float k = length(p);
  float x = asin(sin(atan(p.data[0], p.data[1]) * s) * (1.0f - m)) * k;
  float ds = k * s;
  float y = mix(ds, 2.0f * ds - sqrt(x * x + ds * ds), c);
  return f32x2(x / s, y / s - d);
}
float wedge(Context &__ctx, f32x3 position, float halfWidth, float halfHeight,
            float halfDepth) {
  f32x2 end = f32x2(halfWidth, -halfHeight);
  f32x2 d1 = f32x2(-halfWidth - position.data[0],
                   max(abs(position.data[1]) - halfHeight, 0.0f));
  f32x2 d2 = position.swizzle<0, 1>() -
             end * clamp(dot(position.swizzle<0, 1>(), end) / dot(end, end),
                         -1.0f, 1.0f);
  f32x2 d3 = f32x2(max(abs(position.data[0]) - halfWidth, 0.0f),
                   -halfHeight - position.data[1]);
  float d4 =
      sqrt(min(min(dot(d1, d1), dot(d2, d2)), dot(d3, d3))) *
      sign(max(max(max(d1.data[0], d2.data[0]), d2.data[1]), d3.data[1]));
  float d5 = abs(position.data[2]) - halfDepth;
  return length(max(f32x2(d4, d5), 0.0f)) + min(max(d4, d5), 0.0f);
}
f32x2x2 rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, s, -s, c);
}
float map(Context &__ctx, f32x3 p) {
  float sides = 10.0f;
  if (iMouse.data[2] > 0.5f) {
    if (gm.data[1] > 0.5f)
      p.data[1] = abs(p.data[1]);
    sides = 6.0f;
  }
  float h = 1.5f + sin(iTime) * 0.25f;
  f32x3 pos = p;
  pos.set_swizzle<0, 2>(pos.swizzle<0, 2>() * (rot(__ctx, iTime * 0.5f)));
  pos.set_swizzle<2, 0>(
      smoothRot(__ctx, pos.swizzle<0, 2>(), sides, 0.0f, 2.0f, 1.0f));
  float d1 = wedge(__ctx, pos, 1.0f, h, 4.0f);
  pos = p + f32x3(5.0f, 0.0f, 0.0f);
  pos.set_swizzle<0, 2>(pos.swizzle<0, 2>() * (rot(__ctx, iTime * 0.5f)));
  pos.set_swizzle<2, 0>(
      smoothRot(__ctx, pos.swizzle<0, 2>(), sides, 0.0f, 1.0f, 1.0f));
  float d2 = wedge(__ctx, pos, 1.0f, h, 4.0f);
  pos = p - f32x3(5.0f, 0.0f, 0.0f);
  pos.set_swizzle<0, 2>(pos.swizzle<0, 2>() * (rot(__ctx, iTime * 0.5f)));
  pos.set_swizzle<2, 0>(
      smoothRot(__ctx, pos.swizzle<0, 2>(), sides, 0.05f, 1.0f, 1.0f));
  float d3 = wedge(__ctx, pos, 1.0f, h, 4.0f);
  d1 = min(d1, d2);
  d1 = min(d1, d3);
  return d1 * 0.9f;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.001f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>() * eps) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>() * eps) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>() * eps) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>() * eps));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  gm = (-iResolution.swizzle<0, 1>() + 2.0f * iMouse.swizzle<0, 1>()) /
       iResolution.data[1];
  float an = 3.14f * 0.5f;
  f32x3 ro =
      f32x3(8.0f * cos(an), 0.0f + (sin(iTime * 0.25f) * 3.0f), 8.0f * sin(an));
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  f32x2 p =
      (-iResolution.swizzle<0, 1>() + 2.0f * fragCoord) / iResolution.data[1];
  f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.8f * ww);
  const float tmax = 50.0f;
  float t = 0.0f;
  for (int i = 0; i < 128; i++) {
    f32x3 pos = ro + t * rd;
    float h = map(__ctx, pos);
    if (h < 0.0001f || t > tmax)
      break;
    t += h;
  }
  float v = 1.0f - abs(p.data[1]);
  f32x3 col = f32x3(v * 0.1f);
  if (t < tmax) {
    f32x3 pos = ro + t * rd;
    f32x3 nor = calcNormal(__ctx, pos);
    float dif = clamp(dot(nor, f32x3(0.7f, 0.6f, 0.4f)), 0.0f, 1.0f);
    float amb = 0.5f + 0.5f * dot(nor, f32x3(0.0f, 0.8f, 0.6f));
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
