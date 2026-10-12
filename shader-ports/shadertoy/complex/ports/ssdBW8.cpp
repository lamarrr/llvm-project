// Port of https://www.shadertoy.com/view/ssdBW8; author: Del.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssdBW8.json.
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
float sdCutHollowSphere(Context &__ctx, f32x3 p, float r, float h, float t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()), p.data[1]);
  float w = sqrt(r * r - h * h);
  return ((h * q.data[0] < w * q.data[1]) ? length(q - f32x2(w, h))
                                          : abs(length(q) - r)) -
         t;
}
float sdCutSphere(Context &__ctx, f32x3 p, float r, float h) {
  float w = sqrt(r * r - h * h);
  f32x2 q = f32x2(length(p.swizzle<0, 2>()), p.data[1]);
  float s =
      max((h - r) * q.data[0] * q.data[0] + w * w * (h + r - 2.0f * q.data[1]),
          h * q.data[0] - w * q.data[1]);
  return (s < 0.0f)        ? length(q) - r
         : (q.data[0] < w) ? h - q.data[1]
                           : length(q - f32x2(w, h));
}
float sdBoundingBox(Context &__ctx, f32x3 p, f32x3 b, float e) {
  p = abs(p) - b;
  f32x3 q = abs(p + e) - e;
  return min(min(length(max(f32x3(p.data[0], q.data[1], q.data[2]), 0.0f)) +
                     min(max(p.data[0], max(q.data[1], q.data[2])), 0.0f),
                 length(max(f32x3(q.data[0], p.data[1], q.data[2]), 0.0f)) +
                     min(max(q.data[0], max(p.data[1], q.data[2])), 0.0f)),
             length(max(f32x3(q.data[0], q.data[1], p.data[2]), 0.0f)) +
                 min(max(q.data[0], max(q.data[1], p.data[2])), 0.0f));
}
float cutspheretest(Context &__ctx, f32x3 pos, float rad, float scut,
                    bool hollow) {
  float s = scut < 0.0f ? -1.0f : 1.0f;
  scut = abs(scut);
  float plane = -((scut - 0.5f) * (rad * 2.0f));
  float d = 0.0f;
  float thick = 0.0f;
  if (hollow) {
    thick = .1f;
    d = sdCutHollowSphere(__ctx, pos * s, rad, plane, thick);
  } else {
    d = sdCutSphere(__ctx, pos * -s, rad, -plane);
  }
  float h = (plane + rad) * 0.5f;
  float yoff = rad - h;
  yoff *= s;
  f32x3 bbmid = f32x3(0.0f, yoff, 0.0f);
  f32x3 bbsize = f32x3(rad, h, rad) + thick;
  float d2 = sdBoundingBox(__ctx, pos + bbmid, bbsize, 0.025f);
  return min(d, d2);
}
float map(Context &__ctx, f32x3 pos) {
  float rad = 2.0f;
  float scut = sin(iTime * 0.9f);
  float d1 =
      cutspheretest(__ctx, pos + f32x3(2.5f, 0.0f, 0.0f), rad, scut, false);
  scut = sin(iTime * 0.6f);
  float d2 =
      cutspheretest(__ctx, pos + f32x3(-2.5f, 0.0f, 0.0f), rad, scut, true);
  return min(d1, d2);
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
  float an = 0.5f * iTime;
  float yy = 4.0f;
  if (iMouse.data[2] > 0.5f)
    yy = 0.0f;
  f32x3 ro = f32x3(9.f * cos(an), yy, 9.0f * sin(an));
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 tot = f32x3(0.0f);
  for (int m = 0; m < 2; m++)
    for (int n = 0; n < 2; n++) {
      f32x2 o = f32x2(float(m), float(n)) / float(2) - 0.5f;
      f32x2 p = (-iResolution.swizzle<0, 1>() + 2.0f * (fragCoord + o)) /
                iResolution.data[1];
      f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.8f * ww);
      const float tmax = 30.0f;
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
    }
  tot /= float(2 * 2);
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
