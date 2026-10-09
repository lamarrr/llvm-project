// Port of https://www.shadertoy.com/view/msBXDV; author: SONENEIS.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msBXDV.json.
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
struct Context {
  TextureCube channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), -sin(a), sin(a), cos(a));
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f - 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(a, b, h) - k * h * (1.0f - h);
}
float smax(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(a, b, h) + k * h * (1.0f - h);
}
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sdPlane(Context &__ctx, f32x3 p, f32x3 n, float d) {
  return dot(p, n) + d;
}
float sdCylinderInf(Context &__ctx, f32x3 p, float r) {
  return length(p.swizzle<0, 1>()) - r;
}
float scene(Context &__ctx, f32x3 p) {
  float d = 99999.99999f;
  d = min(d, mix(sdBox(__ctx, p, f32x3(0.4f)) - 0.1f, sdSphere(__ctx, p, 0.5f),
                 -1.0f));
  d = smax(__ctx, d, -sdSphere(__ctx, p, 0.43f), 0.5f);
  d = min(d, sdSphere(__ctx, p, 0.5f));
  d = smin(__ctx, d, sdPlane(__ctx, p, f32x3(0, 1, 0), 0.5f), 0.25f);
  d = smin(__ctx, d, sdSphere(__ctx, p - f32x3(1, -0.25f, -1), 0.3f), 0.05f);
  return d;
}
float raymarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float d = 0.001f;
  for (int i = 0; i < 128; i++) {
    f32x3 p = ro + rd * d;
    float h = scene(__ctx, p);
    d += h * 0.45f;
    if (h < 0.001f || d > 20.0f)
      break;
  }
  if (d > 20.0f)
    return 20.0f;
  return d;
}
float calcSoftShadow(Context &__ctx, f32x3 ro, f32x3 rd, float k) {
  float mint = 0.001f;
  float tmax = 256.0f;
  float res = 1.0f;
  float t = mint;
  float ph = 1e10f;
  for (int i = 0; i < 512; i++) {
    float h = scene(__ctx, ro + rd * t);
    float y = (i == 0) ? 0.0f : h * h / (2.0f * ph);
    float d = sqrt(h * h - y * y);
    res = min(res, k * d / max(0.0f, t - y));
    ph = h;
    t += h * 0.45f;
    if (res < 0.0001f || t > tmax)
      break;
  }
  res = clamp(res, 0.0f, 1.0f);
  return res * res * (3.0f - 2.0f * res);
}
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = 0; i < 5; i++) {
    float h = 0.1f * float(i) / 10.0f;
    float d = scene(__ctx, pos + h * nor);
    occ += (h - d) * sca;
    sca *= 0.95f;
    if (occ > 0.35f)
      break;
  }
  return clamp(1.0f - 7.0f * occ, 0.0f, 1.0f);
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(1, -1) * 0.001f;
  return normalize(
      f32x3(e.swizzle<1, 1, 0>() * scene(__ctx, p + e.swizzle<1, 1, 0>()) +
            e.swizzle<0, 1, 1>() * scene(__ctx, p + e.swizzle<0, 1, 1>()) +
            e.swizzle<1, 0, 1>() * scene(__ctx, p + e.swizzle<1, 0, 1>()) +
            e.swizzle<0, 0, 0>() * scene(__ctx, p + e.swizzle<0, 0, 0>())));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0.0f);
  f32x3 ro = f32x3(0.0f, 0.0f, 3.0f);
  f32x3 rd = normalize(f32x3(uv, -2.0f));
  ro.data[1] += 1.0f;
  rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() * (rot(__ctx, -0.25f)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (rot(__ctx, iTime * 0.5f)));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (rot(__ctx, iTime * 0.5f)));
  f32x3 bg = f32x3(0.7f, 0.8f, 1.0f);
  float d = raymarch(__ctx, ro, rd);
  if (d > 0.0f && d < 20.0f) {
    f32x3 pos = ro + rd * d;
    f32x3 nor = calcNormal(__ctx, pos);
    f32x3 mate = f32x3(0.0f);
    f32x3 ref = reflect(rd, nor);
    float occ = calcAO(__ctx, pos, nor);
    mate = clamp(texture(__ctx.channel0, ref).swizzle<0, 1, 2>(), 0.0f, 1.0f);
    mate = pow(mate, f32x3(2.2f));
    mate *= pow(bg, f32x3(2.2f));
    {
      f32x3 lig = normalize(f32x3(0.5f));
      float dif = clamp(dot(nor, lig), 0.0f, 1.0f);
      float spe = 2.0f * pow(clamp(dot(ref, lig), 0.0f, 1.0f), 10.0f);
      dif *= calcSoftShadow(__ctx, pos, lig, 8.0f);
      col += mate * dif;
      spe *= calcSoftShadow(__ctx, pos, ref, 4.0f);
      col += mate * 5.0f * spe * dif;
      col += clamp(0.001f * pow(spe, 10.0f), 0.0f, 1.0f) * dif;
    }
    {
      float dif = clamp(nor.data[1] * 0.5f + 0.5f, 0.0f, 1.0f);
      float spe = clamp(ref.data[1] * 0.5f + 0.5f, 0.0f, 1.0f);
      col += 0.05f * mate * f32x3(0.2f, 0.4f, 1.0f) * dif;
      spe *= calcSoftShadow(__ctx, pos, ref, 1.0f);
      col += 0.05f * f32x3(0.2f, 0.4f, 1.0f) * spe;
    }
    col = clamp(col, 0.0f, 1.0f) * occ * occ * occ;
  }
  col = clamp(pow(col, f32x3(1.0f / 2.2f)), 0.0f, 1.0f);
  col = mix(
      col,
      clamp(texture(__ctx.channel0, rd).swizzle<0, 1, 2>() * bg, 0.0f, 1.0f),
      clamp(0.000001f * d * d * d * d * d, 0.0f, 1.0f));
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_cube<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
