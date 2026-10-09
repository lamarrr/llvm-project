// Port of https://www.shadertoy.com/view/clGXRW; author: KTSAMA.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clGXRW.json.
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
  TextureCube channel2;
  Texture2D channel3;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x3 mod2D289(Context &__ctx, f32x3 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x4 mod2D289(Context &__ctx, f32x4 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x3 permute(Context &__ctx, f32x3 x) {
  return mod2D289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod2D289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
float noise3D(Context &__ctx, f32x3 p) {
  f32x3 a = floor(p);
  f32x3 d = p - a;
  d = d * d * (3.0f - 2.0f * d);
  f32x4 b = f32x4(a.data[0], a.data[0], a.data[1], a.data[1]) +
            f32x4(0.0f, 1.0f, 0.0f, 1.0f);
  f32x4 k1 = permute(__ctx, b.swizzle<0, 1, 0, 1>());
  f32x4 k2 = permute(__ctx, k1.swizzle<0, 1, 0, 1>() + b.swizzle<2, 2, 3, 3>());
  f32x4 c = k2 + a.swizzle<2, 2, 2, 2>();
  f32x4 k3 = permute(__ctx, c);
  f32x4 k4 = permute(__ctx, c + 1.0f);
  f32x4 o1 = fract(k3 * (1.0f / 41.0f));
  f32x4 o2 = fract(k4 * (1.0f / 41.0f));
  f32x4 o3 = o2 * d.data[2] + o1 * (1.0f - d.data[2]);
  f32x2 o4 =
      o3.swizzle<1, 3>() * d.data[0] + o3.swizzle<0, 2>() * (1.0f - d.data[0]);
  return o4.data[1] * d.data[1] + o4.data[0] * (1.0f - d.data[1]);
}
float sdSphere(Context &__ctx, f32x3 p, float s) { return length(p) - s; }
float opU(Context &__ctx, float d1, float d2) { return min(d1, d2); }
f32x4 opU4(Context &__ctx, f32x4 d1, f32x4 d2) {
  return d1.data[3] < d2.data[3] ? d1 : d2;
}
f32x4 opUS4(Context &__ctx, f32x4 d1, f32x4 d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2.data[3] - d1.data[3]) / k, 0.0f, 1.0f);
  f32x3 color = mix(d2.swizzle<0, 1, 2>(), d1.swizzle<0, 1, 2>(), h);
  float dist = mix(d2.data[3], d1.data[3], h) - k * h * (1.0f - h);
  return f32x4(color, dist);
}
f32x4 opUS4(Context &__ctx, f32x4 d1_col, f32x4 d2_col, float d1, float d2,
            float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  f32x3 color = mix(d2_col.swizzle<0, 1, 2>(), d1_col.swizzle<0, 1, 2>(), h);
  float dist = mix(d2, d1, h) - k * h * (1.0f - h);
  return f32x4(color, dist);
}
float opUS(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  float dist = mix(d2, d1, h) - k * h * (1.0f - h);
  return dist;
}
f32x2 opUS(Context &__ctx, float d1, float d2, float d1_num, float d2_num,
           float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  f32x2 dist = f32x2(0.0f, 0.0f);
  dist.data[0] = mix(d2, d1, h) - k * h * (1.0f - h);
  dist.data[1] = mix(d1_num, d2_num, h);
  return dist;
}
float opSS(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f - 0.5f * (d2 + d1) / k, 0.0f, 1.0f);
  return mix(d2, -d1, h) + k * h * (1.0f - h);
}
float opIS(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f - 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) + k * h * (1.0f - h);
}
f32x2 map(Context &__ctx, f32x3 pos) {
  f32x2 res1 = f32x2(pos.data[1], 0.0f);
  f32x2 res2 = f32x2(pos.data[1], 1.0f);
  res1.data[0] = sdSphere(
      __ctx, pos - f32x3(cos(iTime + 3.1415926f / 2.0f), 0.0f, 0.0f), 0.5f);
  res2.data[0] = sdSphere(__ctx, pos - f32x3(sin(iTime), 0.0f, 0.0f), 0.5f);
  f32x2 result1 = opUS(__ctx, res1.data[0], res2.data[0], res1.data[1],
                       res2.data[1], 0.45f);
  return result1;
}
f32x2 mapInside(Context &__ctx, f32x3 pos) {
  float noiseScale = 10.0f;
  f32x2 res3 = f32x2(pos.data[1], 0.0f);
  f32x2 res3_1 = f32x2(pos.data[1], 0.0f);
  f32x2 res3_2 = f32x2(pos.data[1], 0.0f);
  f32x2 res3_3 = f32x2(pos.data[1], 0.0f);
  f32x2 res3_4 = f32x2(pos.data[1], 0.0f);
  f32x2 res4 = f32x2(pos.data[1], 1.0f);
  f32x2 res4_1 = f32x2(pos.data[1], 1.0f);
  f32x2 res4_2 = f32x2(pos.data[1], 1.0f);
  f32x2 res4_3 = f32x2(pos.data[1], 1.0f);
  f32x2 res4_4 = f32x2(pos.data[1], 1.0f);
  res3.data[0] = sdSphere(
      __ctx, pos - f32x3(cos(iTime + 3.1415926f / 2.0f), 0.0f, 0.0f), 0.2f);
  res3_1.data[0] =
      sdSphere(__ctx,
               pos - f32x3(cos(iTime + 3.1415926f / 2.0f), 0.0f, 0.0f) -
                   f32x3(0.06f, 0.15f, 0.1f),
               0.01f);
  res3_2.data[0] =
      sdSphere(__ctx,
               pos - f32x3(cos(iTime + 3.1415926f / 2.0f), 0.0f, 0.0f) -
                   f32x3(0.1f, -0.15f, 0.06f),
               0.01f);
  res3_3.data[0] =
      sdSphere(__ctx,
               pos - f32x3(cos(iTime + 3.1415926f / 2.0f), 0.0f, 0.0f) -
                   f32x3(0.1f, -0.1f, -0.15f),
               0.02f);
  res3_4.data[0] =
      sdSphere(__ctx,
               pos - f32x3(cos(iTime + 3.1415926f / 2.0f), 0.0f, 0.0f) -
                   f32x3(-0.15f, 0.1f, -0.1f),
               0.02f);
  res3.data[0] = opUS(__ctx, res3.data[0], res3_1.data[0], 0.1f);
  res3.data[0] = opUS(__ctx, res3.data[0], res3_2.data[0], 0.1f);
  res3.data[0] = opUS(__ctx, res3.data[0], res3_3.data[0], 0.1f);
  res3.data[0] =
      opUS(__ctx, res3.data[0], res3_4.data[0], 0.1f) +
      (noise3D(__ctx,
               (pos - f32x3(cos(iTime + 3.1415926f / 2.0f), 0.0f, 0.0f)) *
                   noiseScale) *
           2.0f -
       1.0f) *
          0.008f;
  res4.data[0] = sdSphere(__ctx, pos - f32x3(sin(iTime), 0.0f, 0.0f), 0.2f);
  res4_1.data[0] = sdSphere(
      __ctx, pos - f32x3(sin(iTime), 0.0f, 0.0f) - f32x3(0.1f, -0.1f, 0.1f),
      0.01f);
  res4_2.data[0] = sdSphere(
      __ctx, pos - f32x3(sin(iTime), 0.0f, 0.0f) - f32x3(-0.1f, -0.1f, 0.1f),
      0.01f);
  res4_3.data[0] = sdSphere(
      __ctx, pos - f32x3(sin(iTime), 0.0f, 0.0f) - f32x3(-0.1f, -0.1f, -0.15f),
      0.02f);
  res4_4.data[0] = sdSphere(
      __ctx, pos - f32x3(sin(iTime), 0.0f, 0.0f) - f32x3(0.12f, 0.12f, 0.12f),
      0.03f);
  res4.data[0] = opUS(__ctx, res4.data[0], res4_1.data[0], 0.1f);
  res4.data[0] = opUS(__ctx, res4.data[0], res4_2.data[0], 0.1f);
  res4.data[0] = opUS(__ctx, res4.data[0], res4_3.data[0], 0.1f);
  res4.data[0] =
      opUS(__ctx, res4.data[0], res4_4.data[0], 0.1f) +
      (noise3D(__ctx, (pos - f32x3(sin(iTime), 0.0f, 0.0f)) * noiseScale) *
           2.0f -
       1.0f) *
          0.008f;
  f32x2 result2 =
      opUS(__ctx, res3.data[0], res4.data[0], res3.data[1], res4.data[1], 0.3f);
  return result2;
}
f32x2 iBox(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 rad) {
  f32x3 invRd = 1.0f / rd;
  f32x3 dist = invRd * ro;
  f32x3 boxRadius = abs(invRd) * rad;
  f32x3 t1 = -dist - boxRadius;
  f32x3 t2 = -dist + boxRadius;
  return f32x2(max(max(t1.data[0], t1.data[1]), t1.data[2]),
               min(min(t2.data[0], t2.data[1]), t2.data[2]));
}
f32x2 raycast(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x2 res = f32x2(-1.0f, -1.0f);
  float tmin = 1.0f;
  float tmax = 20.0f;
  f32x2 tb =
      iBox(__ctx, ro - f32x3(0.0f, 0.0f, 0.0f), rd, f32x3(2.0f, 2.0f, 2.0f));
  if (tb.data[0] < tb.data[1] && tb.data[1] > 0.0f && tb.data[0] < tmax) {
    tmin = max(tb.data[0], tmin);
    tmax = min(tb.data[1], tmax);
    float t = tmin;
    for (int i = 0; i < 70 && t < tmax; i++) {
      f32x2 h = map(__ctx, ro + rd * t);
      if (abs(h.data[0]) < (0.0001f * t)) {
        res = f32x2(t, h.data[1]);
        break;
      }
      t += h.data[0];
    }
  }
  return res;
}
f32x2 raycastInside(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x2 res = f32x2(-1.0f, -1.0f);
  float tmin = 1.0f;
  float tmax = 20.0f;
  f32x2 tb =
      iBox(__ctx, ro - f32x3(0.0f, 0.0f, 0.0f), rd, f32x3(2.0f, 2.0f, 2.0f));
  if (tb.data[0] < tb.data[1] && tb.data[1] > 0.0f && tb.data[0] < tmax) {
    tmin = max(tb.data[0], tmin);
    tmax = min(tb.data[1], tmax);
    float t = tmin;
    for (int i = 0; i < 70 && t < tmax; i++) {
      f32x2 h = mapInside(__ctx, ro + rd * t);
      if (abs(h.data[0]) < (0.0001f * t)) {
        res = f32x2(t, h.data[1]);
        break;
      }
      t += h.data[0];
    }
  }
  return res;
}
float calcSoftshadow(Context &__ctx, f32x3 ro, f32x3 rd, float mint,
                     float tmax) {
  float tp = (0.8f - ro.data[1]) / rd.data[1];
  if (tp > 0.0f)
    tmax = min(tmax, tp);
  float res = 1.0f;
  float t = mint;
  for (int i = (min(iFrame, 0)); i < 24; i++) {
    float h = map(__ctx, ro + rd * t).data[0];
    float s = clamp(8.0f * h / t, 0.0f, 1.0f);
    res = min(res, s);
    t += clamp(h, 0.01f, 0.2f);
    if (res < 0.004f || t > tmax)
      break;
  }
  res = clamp(res, 0.0f, 1.0f);
  return res * res * (3.0f - 2.0f * res);
}
float calcSoftshadow_Inside(Context &__ctx, f32x3 ro, f32x3 rd, float mint,
                            float tmax) {
  float tp = (0.8f - ro.data[1]) / rd.data[1];
  if (tp > 0.0f)
    tmax = min(tmax, tp);
  float res = 1.0f;
  float t = mint;
  for (int i = (min(iFrame, 0)); i < 24; i++) {
    float h = mapInside(__ctx, ro + rd * t).data[0];
    float s = clamp(8.0f * h / t, 0.0f, 1.0f);
    res = min(res, s);
    t += clamp(h, 0.01f, 0.2f);
    if (res < 0.004f || t > tmax)
      break;
  }
  res = clamp(res, 0.0f, 1.0f);
  return res * res * (3.0f - 2.0f * res);
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x3 n = f32x3(0.0f);
  for (int i = (min(iFrame, 0)); i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * map(__ctx, pos + 0.0005f * e).data[0];
  }
  return normalize(n);
}
f32x3 calcNormal_Inside(Context &__ctx, f32x3 pos) {
  f32x3 n = f32x3(0.0f);
  for (int i = (min(iFrame, 0)); i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * mapInside(__ctx, pos + 0.0005f * e).data[0];
  }
  return normalize(n);
}
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = (min(iFrame, 0)); i < 5; i++) {
    float h = 0.01f + 0.12f * float(i) / 4.0f;
    float d = map(__ctx, pos + h * nor).data[0];
    occ += (h - d) * sca;
    sca *= 0.95f;
    if (occ > 0.35f)
      break;
  }
  return clamp(1.0f - 3.0f * occ, 0.0f, 1.0f) * (0.5f + 0.5f * nor.data[1]);
}
float calcAO_Inside(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = (min(iFrame, 0)); i < 5; i++) {
    float h = 0.01f + 0.12f * float(i) / 4.0f;
    float d = mapInside(__ctx, pos + h * nor).data[0];
    occ += (h - d) * sca;
    sca *= 0.95f;
    if (occ > 0.35f)
      break;
  }
  return clamp(1.0f - 3.0f * occ, 0.0f, 1.0f) * (0.5f + 0.5f * nor.data[1]);
}
float checkersGradBox(Context &__ctx, f32x2 p, f32x2 dpdx, f32x2 dpdy) {
  f32x2 w = abs(dpdx) + abs(dpdy) + 0.001f;
  f32x2 i = 2.0f *
            (abs(fract((p - 0.5f * w) * 0.5f) - 0.5f) -
             abs(fract((p + 0.5f * w) * 0.5f) - 0.5f)) /
            w;
  return 0.5f - 0.5f * i.data[0] * i.data[1];
}
const int c_samplesX = 15;
const int c_samplesY = 15;
const float c_textureSize = 512.0f;
const int c_halfSamplesX = c_samplesX / 2;
const int c_halfSamplesY = c_samplesY / 2;
const float c_pixelSize = (1.0f / c_textureSize);
float Gaussian(Context &__ctx, float sigma, float x) {
  return exp(-(x * x) / (2.0f * sigma * sigma));
}
f32x3 BlurredPixel(Context &__ctx, f32x2 uv) {
  float total = 0.0f;
  f32x3 ret = f32x3(0);
  for (int iy = 0; iy < c_samplesY; ++iy) {
    float fy = Gaussian(__ctx, 10.0f, float(iy) - float(c_halfSamplesY));
    float offsety = float(iy - c_halfSamplesY) * c_pixelSize;
    for (int ix = 0; ix < c_samplesX; ++ix) {
      float fx = Gaussian(__ctx, 5.0f, float(ix) - float(c_halfSamplesX));
      float offsetx = float(ix - c_halfSamplesX) * c_pixelSize;
      total += fx * fy;
      ret += texture(__ctx.channel3, uv + f32x2(offsetx, offsety))
                 .swizzle<0, 1, 2>() *
             fx * fy;
    }
  }
  return ret / total;
}
f32x4 render(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 rdx, f32x3 rdy, f32x2 uv,
             float inside) {
  f32x4 col = f32x4(0.0f);
  f32x4 backGround = texture(__ctx.channel3, uv) * 0.8f;
  col.set_swizzle<0, 1, 2>(f32x3(0.7f, 0.7f, 0.9f) -
                           max(rd.data[1], 0.0f) * 0.3f);
  col.data[3] = 0.0f;
  f32x2 resOut = raycast(__ctx, ro, rd);
  float t = resOut.data[0];
  float m = resOut.data[1];
  if (m >= -0.5f) {
    f32x3 pos = ro + t * rd;
    f32x3 nor = (m < 0.0f) ? f32x3(0.0f, 1.0f, 0.0f) : calcNormal(__ctx, pos);
    f32x3 ref = reflect(rd, nor);
    col = mix(f32x4(0.8f, 0.2f, 0.2f, 0.8f), f32x4(0.2f, 0.2f, 0.8f, 0.8f), m);
    float ks = 5.0f;
    float occ = calcAO(__ctx, pos, nor);
    backGround =
        f32x4(BlurredPixel(__ctx, uv + (nor.swizzle<0, 1>()) * 20.0f /
                                           iResolution.swizzle<0, 1>()),
              1.0f) *
        (1.0f - inside * 0.98f);
    col.set_swizzle<0, 1, 2>(col.swizzle<0, 1, 2>() *
                             (backGround.swizzle<0, 1, 2>()));
    f32x3 lin = f32x3(0.0f);
    {
      f32x3 lig = normalize(f32x3(0.1f, 0.8f, 0.1f));
      f32x3 hal = normalize(lig - rd);
      float dif = clamp(dot(nor, lig), 0.0f, 1.0f);
      dif *= calcSoftshadow(__ctx, pos, lig, 0.05f, 2.5f);
      float spe = pow(clamp(dot(nor, hal), 0.0f, 1.0f), 500.0f);
      spe *= dif;
      spe *= 0.04f + 0.96f * pow(clamp(1.0f - dot(hal, lig), 0.0f, 1.0f), 5.0f);
      lin += col.swizzle<0, 1, 2>() * 2.20f * dif * f32x3(1.30f, 1.00f, 0.70f);
      lin += 5.00f * spe * f32x3(1.30f, 1.00f, 0.70f) * ks;
    }
    {
      float dif = 0.5f * sqrt(clamp(0.5f + 0.5f * nor.data[1], 0.0f, 1.0f));
      dif *= occ;
      float spe = smoothstep(-0.2f, 0.2f, ref.data[1]);
      spe *= dif;
      spe *= 0.04f + 0.96f * pow(clamp(1.0f + dot(nor, rd), 0.0f, 1.0f), 5.0f);
      spe *= calcSoftshadow(__ctx, pos, ref, 0.05f, 2.5f);
      lin += col.swizzle<0, 1, 2>() * 0.60f * dif * f32x3(0.40f, 0.60f, 1.15f);
      lin += 2.00f * spe * f32x3(0.40f, 0.60f, 1.30f) * ks;
    }
    {
      float dif =
          clamp(dot(nor, normalize(f32x3(0.5f, 0.0f, 0.6f))), 0.0f, 1.0f) *
          clamp(1.0f - pos.data[1], 0.0f, 1.0f);
      dif *= occ;
      lin += col.swizzle<0, 1, 2>() * 0.55f * dif * f32x3(0.25f, 0.25f, 0.25f);
    }
    {
      float dif = pow(clamp(1.0f + dot(nor, rd), 0.0f, 1.0f), 2.0f);
      dif *= occ;
      lin += col.swizzle<0, 1, 2>() * 0.25f * dif * f32x3(1.00f, 1.00f, 1.00f);
    }
    {
      f32x3 evn_col = texture(__ctx.channel2, ref).swizzle<0, 1, 2>() * 2.0f;
      float f = pow((1.0f - max(0.0f, dot(-rd, nor))) * 2.0f, 5.0f);
      lin += col.swizzle<0, 1, 2>() * evn_col * 0.1f *
             pow((max(0.0f, dot(-rd, nor))) * 2.0f, 5.0f);
      lin += col.swizzle<0, 1, 2>() * f * evn_col * 0.2f;
      lin *= clamp(pow((max(0.0f, dot(-rd, nor))) * 1.0f, 2.0f) + 0.3f, 0.0f,
                   1.0f);
    }
    col.set_swizzle<0, 1, 2>(0.7f * lin);
    col.set_swizzle<0, 1, 2>(mix(col.swizzle<0, 1, 2>(),
                                 f32x3(0.7f, 0.7f, 0.9f),
                                 1.0f - exp(-0.0001f * t * t * t)));
  }
  return f32x4(
      clamp(f32x4(backGround.swizzle<0, 1, 2>() * (1.0f - col.data[3]) +
                      col.swizzle<0, 1, 2>() * col.data[3],
                  1.0f),
            0.0f, 1.0f));
}
f32x4 renderInside(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 rdx, f32x3 rdy,
                   f32x2 uv) {
  f32x4 col = f32x4(0.0f);
  f32x4 backGround = f32x4(0.0f);
  col.set_swizzle<0, 1, 2>(f32x3(0.7f, 0.7f, 0.9f) -
                           max(rd.data[1], 0.0f) * 0.3f);
  col.data[3] = 0.0f;
  f32x2 resSide = raycastInside(__ctx, ro, rd);
  float t = resSide.data[0];
  float m = resSide.data[1];
  if (m >= -0.5f) {
    f32x3 pos = ro + t * rd;
    f32x3 nor =
        (m < 0.0f) ? f32x3(0.0f, 1.0f, 0.0f) : calcNormal_Inside(__ctx, pos);
    f32x3 ref = reflect(rd, nor);
    col = mix(f32x4(0.11f, 0.15f, 0.25f, 1.0f),
              f32x4(0.11f, 0.15f, 0.25f, 1.0f), m);
    float ks = 0.5f;
    float occ = calcAO_Inside(__ctx, pos, nor);
    f32x3 lin = f32x3(0.0f);
    {
      f32x3 lig = normalize(f32x3(0.1f, 0.8f, 0.1f));
      f32x3 hal = normalize(lig - rd);
      float dif = clamp(pow(dot(nor, lig), 2.0f), 0.0f, 1.0f);
      dif *= calcSoftshadow_Inside(__ctx, pos, lig, 0.05f, 2.5f);
      float spe = pow(clamp(dot(nor, hal), 0.0f, 1.0f), 100.0f);
      spe *= dif;
      spe *= 0.04f + 0.96f * pow(clamp(1.0f - dot(hal, lig), 0.0f, 1.0f), 5.0f);
      lin += col.swizzle<0, 1, 2>() * 2.20f * dif * f32x3(1.30f, 1.00f, 0.70f);
      lin += 5.00f * spe * f32x3(1.30f, 1.00f, 0.70f) * ks;
    }
    {
      float dif = 0.5f * sqrt(clamp(0.5f + 0.5f * nor.data[1], 0.0f, 1.0f));
      dif *= occ;
      float spe = smoothstep(-0.2f, 0.2f, ref.data[1]);
      spe *= dif;
      spe *= 0.04f + 0.96f * pow(clamp(1.0f + dot(nor, rd), 0.0f, 1.0f), 5.0f);
      spe *= calcSoftshadow_Inside(__ctx, pos, ref, 0.05f, 2.5f);
      lin += col.swizzle<0, 1, 2>() * 0.60f * dif * f32x3(0.40f, 0.60f, 1.15f);
      lin += 2.00f * spe * f32x3(0.40f, 0.60f, 1.30f) * ks;
    }
    {
      float dif =
          clamp(dot(nor, normalize(f32x3(0.5f, 0.0f, 0.6f))), 0.0f, 1.0f) *
          clamp(1.0f - pos.data[1], 0.0f, 1.0f);
      dif *= occ;
      lin += col.swizzle<0, 1, 2>() * 0.55f * dif * f32x3(0.25f, 0.25f, 0.25f);
    }
    {
      float dif = pow(clamp(1.0f + dot(nor, rd), 0.0f, 1.0f), 2.0f);
      dif *= occ;
      lin += col.swizzle<0, 1, 2>() * 0.25f * dif * f32x3(1.00f, 1.00f, 1.00f);
    }
    {
      f32x3 evn_col = texture(__ctx.channel2, ref).swizzle<0, 1, 2>() * 0.5f;
      float f = pow((1.0f - max(0.0f, dot(-rd, nor))) * 1.5f, 5.0f);
      lin += col.swizzle<0, 1, 2>() * f * evn_col * 0.2f;
    }
    col.set_swizzle<0, 1, 2>(0.7f * lin);
    col.set_swizzle<0, 1, 2>(mix(col.swizzle<0, 1, 2>(),
                                 f32x3(0.7f, 0.7f, 0.9f),
                                 1.0f - exp(-0.0001f * t * t * t)));
  }
  return f32x4(
      clamp(f32x4(backGround.swizzle<0, 1, 2>() * (1.0f - col.data[3]) +
                      col.swizzle<0, 1, 2>() * col.data[3],
                  col.data[3]),
            0.0f, 1.0f));
}
f32x3x3 setCamera(Context &__ctx, f32x3 ro, f32x3 ta, float cr) {
  f32x3 cw = normalize(ta - ro);
  f32x3 cp = f32x3(sin(cr), cos(cr), 0.0f);
  f32x3 cu = normalize(cross(cw, cp));
  f32x3 cv = (cross(cu, cw));
  return f32x3x3(cu, cv, cw);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 mo = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float time = 32.0f + iTime * 1.5f;
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ro = ta + f32x3(4.5f * cos(0.1f * time + 7.0f * mo.data[0]), 2.0f,
                        4.5f * sin(0.1f * time + 7.0f * mo.data[0]));
  f32x3x3 ca = setCamera(__ctx, ro, ta, 0.0f);
  f32x3 tot = f32x3(0.0f);
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  const float fl = 3.5f;
  f32x3 rd = ca * normalize(f32x3(p, fl));
  f32x2 px =
      (2.0f * (fragCoord + f32x2(1.0f, 0.0f)) - iResolution.swizzle<0, 1>()) /
      iResolution.data[1];
  f32x2 py =
      (2.0f * (fragCoord + f32x2(0.0f, 1.0f)) - iResolution.swizzle<0, 1>()) /
      iResolution.data[1];
  f32x3 rdx = ca * normalize(f32x3(px, fl));
  f32x3 rdy = ca * normalize(f32x3(py, fl));
  f32x4 colInside = renderInside(__ctx, ro, rd, rdx, rdy, uv);
  f32x4 col = render(__ctx, ro, rd, rdx, rdy, uv, colInside.data[3]);
  col += colInside;
  col.set_swizzle<0, 1, 2>(pow(col.swizzle<0, 1, 2>(), f32x3(0.4545f)));
  tot += col.swizzle<0, 1, 2>();
  fragColor = f32x4(tot, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 2) texture_cube<f32x4> channel2,
               __binding__(1, 2) sampler sampling2,
               __binding__(0, 3) texture_2d<f32x4> channel3,
               __binding__(1, 3) sampler sampling3) {
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
  Context __ctx{{channel2, sampling2}, {channel3, sampling3}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
