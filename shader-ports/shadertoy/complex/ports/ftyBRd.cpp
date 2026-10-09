// Port of https://www.shadertoy.com/view/ftyBRd; author: iq.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ftyBRd.json.
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
float sdHelix(Context &__ctx, f32x3 p, float fr, float r1, float r2) {
  f32x2 nline = f32x2(fr, 6.283185f * r1);
  f32x2 pline = f32x2(nline.data[1], -nline.data[0]);
  float repeat = nline.data[0] * nline.data[1];
  f32x2 pc = f32x2(p.data[0], r1 * atan(p.data[1], p.data[2]));
  f32x2 pp = f32x2(dot(pc, pline), dot(pc, nline));
  pp.data[0] = round(pp.data[0] / repeat) * repeat;
  f32x2 qc = (nline * pp.data[1] + pline * pp.data[0]) / dot(nline, nline);
  qc.data[1] /= r1;
  f32x3 q = f32x3(qc.data[0], sin(qc.data[1]) * r1, cos(qc.data[1]) * r1);
  return length(p - q) - r2;
}
float map(Context &__ctx, f32x3 pos) {
  float fr = 1.0f - 0.5f * cos(6.283185f * iTime / 6.0f);
  float r1 = 0.4f;
  float r2 = 0.1f;
  return sdHelix(__ctx, pos, fr, r1, r2) - 0.0001f;
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
  float an = 1.0f + 0.0f * iTime;
  f32x3 ro = f32x3(1.5f * cos(an), 0.4f, 1.5f * sin(an));
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  f32x2 p =
      (-iResolution.swizzle<0, 1>() + 2.0f * fragCoord) / iResolution.data[1];
  f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.5f * ww);
  const float tmax = 13.0f;
  float t = 0.0f;
  for (int i = 0; i < 256; i++) {
    f32x3 pos = ro + t * rd;
    float h = map(__ctx, pos);
    if (abs(h) < 0.0001f || t > tmax)
      break;
    t += h * 0.75f;
  }
  f32x3 col = f32x3(0.0f);
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
