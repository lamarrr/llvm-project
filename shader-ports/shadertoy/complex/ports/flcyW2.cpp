// Port of https://www.shadertoy.com/view/flcyW2; author: Yusef28.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/flcyW2.json.
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
  Texture2D channel0;
  Texture2D channel1;
  TextureCube channel2;
  Texture2D channel3;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float sdCappedCylinder(Context &__ctx, f32x3 p, float h, float r) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 2>()), p.data[1])) - f32x2(h, r);
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float smax(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (a - b) / k, 0.0f, 1.0f);
  return mix(b, a, h) + k * h * (1.0f - h);
}
float knochen(Context &__ctx, f32x3 p, f32x3 xyz, f32x3 dim, float th,
              float phi, float ga, float spiegel, float r) {
  f32x3 sp = p - xyz;
  sp.data[0] = abs(sp.data[0]) - spiegel;
  float cs = cos(th), si = sin(th);
  sp.set_swizzle<1, 2>(sp.swizzle<1, 2>() * (f32x2x2(cs, si, -si, cs)));
  cs = cos(phi), si = sin(phi);
  sp.set_swizzle<0, 2>(sp.swizzle<0, 2>() * (f32x2x2(cs, si, -si, cs)));
  cs = cos(ga), si = sin(ga);
  sp.set_swizzle<0, 1>(sp.swizzle<0, 1>() * (f32x2x2(cs, si, -si, cs)));
  sp = abs(sp) - dim;
  return min(max(sp.data[0], max(sp.data[1], sp.data[2])), 0.0f) +
         length(max(sp, 0.0f)) - r;
}
float sdCircleCircle(Context &__ctx, f32x3 p, int n, float zeichen) {
  float a0 = float(n) / radians(360.f);
  float a = round(atan(p.data[2], p.data[0]) * a0) / a0;
  f32x3 sp = p - f32x3(cos(a), 0.0f, sin(a));
  float cs = cos(a), si = sin(a);
  sp.set_swizzle<0, 2>(sp.swizzle<0, 2>() * (f32x2x2(cs, si, -si, cs)));
  cs = cos(0.3f * zeichen), si = sin(0.3f * zeichen);
  sp.set_swizzle<0, 1>(sp.swizzle<0, 1>() * (f32x2x2(cs, si, -si, cs)));
  sp = abs(sp) - f32x3(0.1f + cos(sp.data[1] * 8.f) * 0.03f, 0.19f,
                       0.07f + cos(sp.data[1] * 6.f + 1.4f) * 0.05f * zeichen);
  return min(max(sp.data[0], max(sp.data[1], sp.data[2])), 0.0f) +
         length(max(sp, 0.0f)) - 0.03f;
}
const f32x3x3 m =
    f32x3x3(0.00f, 0.80f, 0.60f, -0.80f, 0.36f, -0.48f, -0.60f, -0.48f, 0.64f);
f32x4 objID, oSvObjID;
float svObjID;
float skull(Context &__ctx, f32x3 p) {
  f32x3 altp = p;
  p.data[0] += 1.f;
  p.data[0] = mod(p.data[0], 2.f) - 1.f;
  float PosX, PosY;
  float cs = cos(PosX), si = sin(PosX);
  if (altp.data[0] > 0.75f || altp.data[0] < -0.75f) {
    p.set_swizzle<0, 2>(f32x2x2(cs, si, -si, cs) * p.swizzle<0, 2>());
  } else {
    cs = cos(PosY), si = sin(PosY);
    p.set_swizzle<1, 2>(f32x2x2(cs, si, -si, cs) * p.swizzle<1, 2>());
  }
  float gu = -1.6f;
  cs = cos(gu), si = sin(gu);
  p.set_swizzle<1, 2>(f32x2x2(cs, si, -si, cs) * p.swizzle<1, 2>());
  p *= 0.6f;
  p.data[1] -= 0.2f;
  f32x3 sp = p - f32x3(0.f, 0.f, 0.03f);
  float k =
      length(sp * f32x3(1.f - sin(sp.data[2] * 6.f - 0.4f) * 0.05f,
                        0.95f + sin(sp.data[2] * 6.f + 1.6f) * 0.05f, 0.87f)) -
      0.5f;
  float alle = k;
  sp = p;
  k = length(sp - f32x3(0.f, -0.9f, -0.5f)) - 0.7f;
  alle = smax(__ctx, alle, -k, 0.09f);
  sp = p - f32x3(0.f, -0.05f, -0.28f);
  sp.data[0] = abs(sp.data[0]) - 0.65f;
  k = length(sp) - 0.3f;
  sp = p;
  k = knochen(__ctx, p, f32x3(0.f, -0.15f, -0.18f),
              f32x3(0.005f, 0.17f + sin(sp.data[2] * 4.f + 2.4f) * 0.1f,
                    0.2f + sin(sp.data[1] * 4.f + 2.f) * 0.18f),
              -0.2f, -0.4f, 0.1f, 0.42f + sin(sp.data[2] * 4.f + 2.4f) * 0.05f,
              0.03f);
  alle = smax(__ctx, alle, -k, 0.09f);
  sp = p;
  k = knochen(
      __ctx, p,
      f32x3(0.f,
            0.f + cos(abs(sp.data[0]) * 8.f + 5.f) * 0.03f * 1.f - 0.04f * 1.f,
            -0.45f - cos(abs(sp.data[0]) * 8.f + 5.8f) * 0.05f),
      f32x3(0.12f, 0.005f, 0.005f), 0.f, 0.f, -0.f, 0.14f, 0.04f);
  alle = smin(__ctx, alle, k, 0.07f);
  sp = p;
  k = knochen(__ctx, p, f32x3(0.f, 0.055f, -0.37f),
              f32x3(0.005f, 0.005f, 0.12f), -1.f, -0.25f, 0.f, 0.28f, 0.02f);
  alle = smin(__ctx, alle, k, 0.06f);
  sp = p;
  k = knochen(__ctx, p,
              f32x3(0.f, -0.14f, -0.46f + cos(sp.data[1] * 8.f + 1.5f) * 0.07f),
              f32x3(0.01f, 0.08f, 0.02f), 0.0f, 0.0f, 0.35f,
              0.26f + cos(sp.data[1] * 12.f + 1.2f) * 0.06f, 0.015f);
  alle = smin(__ctx, alle, k, 0.06f);
  sp = p;
  k = knochen(
      __ctx, p,
      f32x3(0.f, -0.28f, -0.42f + cos(abs(sp.data[0]) * 8.f + 2.f) * 0.02f),
      f32x3(0.13f, 0.055f + sin(abs(sp.data[0]) * 8.f + 2.f) * 0.02f, 0.05f),
      -0.5f, 0.2f, -0.1f, 0.12f, 0.05f);
  alle = smin(__ctx, alle, k, 0.09f);
  sp = p;
  k = knochen(__ctx, p, f32x3(0.f, -0.3f, -0.28f),
              f32x3(0.007f,
                    0.02f - cos(sp.data[2] * 8.f + 1.7f -
                                cos(sp.data[2] * 20.f + 1.7f)) *
                                0.01f,
                    0.16f),
              0.f, 0.1f, 0.f, 0.27f + cos(sp.data[2] * 8.f + 2.f) * 0.09f,
              0.03f + cos(sp.data[2] * 8.f + 4.7f) * 0.015f);
  alle = smin(__ctx, alle, k, 0.02f);
  float keiferHohe = 0.01f;
  sp = p;
  k = knochen(__ctx, p,
              f32x3(0.f, -0.51f + keiferHohe,
                    -0.25f - cos(abs(sp.data[1]) * 12.f + 3.f) * 0.04f),
              f32x3(0.005f - cos(sp.data[2] * 12.f + 4.5f) * 0.004f -
                        cos(sp.data[1] * 6.f + 1.2f) * 0.017f,
                    0.125f, 0.1f),
              0.2f + (-sp.data[1] - 0.5f) * 0.2f, -0.35f, -0.25f, 0.29f, 0.01f);
  float keifer = k;
  sp = p;
  k = knochen(__ctx, p, f32x3(0.f, -0.67f + keiferHohe, -0.35f),
              f32x3(0.005f, 0.02f, 0.16f), -0.3f, -0.5f, 0.f,
              0.18f + cos(sp.data[2] * 8.f + 3.14f) * 0.02f, 0.045f);
  keifer = smin(__ctx, keifer, k, 0.04f);
  sp = p;
  k = knochen(__ctx, p,
              f32x3(0.f, -0.58f + keiferHohe,
                    -0.28f - cos(abs(sp.data[1]) * 12.f + 3.f) * 0.06f),
              f32x3(0.001f, 0.15f, 0.055f), 0.2f + (-sp.data[1] - 0.5f) * 0.2f,
              -0.45f, -0.3f, 0.34f, 0.01f);
  keifer = smax(__ctx, keifer, -k, 0.04f);
  sp = p;
  sp.data[0] = abs(sp.data[0]) - 0.31f;
  k = length(sp - f32x3(0.f, -0.38f + keiferHohe, -0.23f)) - 0.06f;
  keifer = smax(__ctx, keifer, -k, 0.04f);
  sp = p;
  k = knochen(
      __ctx, p,
      f32x3(0.f, -0.73f + keiferHohe, -0.45f - cos(sp.data[0] * 8.f) * 0.1f),
      f32x3(0.09f, 0.02f, 0.015f), 0.f, 0.f, 0.f, 0.f, 0.03f);
  keifer = smin(__ctx, keifer, k, 0.09f);
  sp = p;
  sp.data[0] = abs(sp.data[0]) - 0.172f;
  k = length(sp - f32x3(0.f, -0.14f, -0.48f)) - 0.084f;
  alle = smax(__ctx, alle, -k, 0.075f);
  sp = p;
  k = knochen(__ctx, p, f32x3(0.f, -0.23f, -0.51f), f32x3(0.03f, 0.09f, 0.08f),
              -0.3f, 0.f, 0.3f, 0.f, 0.039f);
  alle = smin(__ctx, alle, k, 0.06f);
  sp = p;
  k = knochen(__ctx, p, f32x3(0.f, -0.25f, -0.54f), f32x3(0.022f, 0.08f, 0.07f),
              -0.35f, 0.f, 0.3f, 0.f, 0.03f);
  alle = smax(__ctx, alle, -k, 0.04f);
  sp = p;
  k = knochen(__ctx, p, f32x3(0.f, -0.25f, -0.5f), f32x3(0.001f, 0.12f, 0.07f),
              -0.35f, 0.f, 0.f, 0.f, 0.005f);
  alle = smin(__ctx, alle, k, 0.03f);
  float zahnNum = 12.f;
  float th = 0.15f;
  sp = p;
  cs = cos(th), si = sin(th);
  sp = sp - f32x3(0.f, -0.43f, -0.4f);
  sp.set_swizzle<1, 2>(sp.swizzle<1, 2>() * (f32x2x2(cs, si, -si, cs)));
  k = sdCappedCylinder(
      __ctx, sp,
      0.17f + cos(sp.data[1] * 12.f - 5.f) * 0.015f +
          abs(cos(atan(sp.data[2], sp.data[0]) * zahnNum - 0.8f)) * 0.005f,
      0.045f - abs(cos(atan(sp.data[2], sp.data[0]) * zahnNum - 0.8f)) * 0.02f -
          cos(sp.data[2] * 6.f - 1.75f) * 0.05f);
  alle = smin(__ctx, alle, k, 0.06f);
  sp = p - f32x3(0.f, -0.65f + keiferHohe, -0.42f);
  k = sdCappedCylinder(
      __ctx, sp,
      0.15f + cos(sp.data[1] * 12.f - 1.5f) * 0.03f +
          abs(cos(atan(sp.data[2], sp.data[0]) * zahnNum - 0.8f)) * 0.01f,
      0.05f - abs(cos(atan(sp.data[2], sp.data[0]) * zahnNum - 0.8f)) * 0.01f);
  keifer = smin(__ctx, keifer, k, 0.06f);
  sp = p - f32x3(0.0f, -0.51f, -0.41f);
  cs = cos(0.33f), si = sin(0.33f);
  sp.set_swizzle<0, 2>(sp.swizzle<0, 2>() * (f32x2x2(cs, si, -si, cs)));
  k = sdCircleCircle(__ctx, sp * 6.6f, 24, 1.f);
  float zahn = k;
  sp = p - f32x3(0.0f, -0.58f + keiferHohe, -0.39f);
  cs = cos(0.33f), si = sin(0.33f);
  sp.set_swizzle<0, 2>(sp.swizzle<0, 2>() * (f32x2x2(cs, si, -si, cs)));
  k = sdCircleCircle(__ctx, sp * 6.6f, 24, -1.f);
  zahn = min(zahn, k);
  objID = f32x4(zahn - 0.05f, alle, keifer, 0);
  alle = smin(__ctx, zahn, alle, 0.09f);
  alle = smin(__ctx, alle, keifer, 0.09f);
  sp = p - f32x3(0.f, -0.58f, -0.15f);
  k = sdCappedCylinder(__ctx, sp, 0.17f, 0.24f);
  alle = smax(__ctx, alle, -k, 0.06f);
  sp = p;
  sp.data[0] = abs(sp.data[0]) - 0.18f;
  k = length(sp - f32x3(0.f, -0.32f, -0.522f)) - 0.012f;
  alle = smax(__ctx, alle, -k, 0.01f);
  return alle;
}
float vig(Context &__ctx, f32x2 uv) {
  uv *= 1.0f - uv.swizzle<1, 0>();
  float vig = uv.data[0] * uv.data[1] * 200.0f;
  return pow(vig, 1.f);
}
float hash(Context &__ctx, float p) {
  return fract(sin(p * 324.341f) * 23402.43f);
}
float hash21(Context &__ctx, f32x2 st) {
  return fract(sin(dot(f32x2(12.23f, 74.343f), st)) * 43254.f);
}
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a - f32x4(0, 11, 33, 0)));
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
float Gyroid(Context &__ctx, f32x3 p, float scale, float thickness,
             float bias) {
  p *= scale;
  float d =
      abs(dot(sin(p), cos(p.swizzle<2, 0, 1>())) - bias) / scale - thickness;
  return d;
}
bool checkLogoBoundingBox(Context &__ctx, f32x3 p) {
  return (p.data[0] < -4.5f || p.data[0] > 4.5f || p.data[1] < -3.f ||
          p.data[1] > 2.f || p.data[2] < -4.f || p.data[2] > 4.f);
}
float map(Context &__ctx, f32x3 p) {
  float d = length(p) - 1.5f;
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * t;
    float d = map(__ctx, p);
    t += d * 0.7f;
    if (t > 100.f || abs(d) < 0.001f)
      break;
  }
  return t;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = map(__ctx, p) - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()),
                                  map(__ctx, p - e.swizzle<1, 0, 1>()),
                                  map(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 camRay(Context &__ctx, f32x2 uv, f32x3 o, f32x3 target, float zoom) {
  f32x3 fwd = normalize(target - o);
  f32x3 uu = f32x3(0.f, 1.f, 0.f);
  uu = dot(target, uu) < 0.01f ? f32x3(0.f, 0.f, 1.f) : uu;
  f32x3 right = normalize(cross(uu, fwd));
  f32x3 up = cross(fwd, right);
  f32x3 rd = right * uv.data[0] + up * uv.data[1] + fwd * zoom;
  return normalize(rd);
}
f32x2 map_from_sphere(Context &__ctx, f32x3 p) {
  p = normalize(p);
  return f32x2(atan(p.data[0], p.data[1]) / acos(-1.f) / 2.f + 0.5f,
               acos(p.data[2]) / acos(-1.f) / 2.f + 0.5f);
}
f32x3 checkers(Context &__ctx, f32x2 u) {
  f32x2 st = u;
  u *= 2.f;
  u = (fract((u * 2.f)) - 0.5f) * 2.f;
  u = u.data[0] * u.data[1] < 0.f ? abs(u.swizzle<1, 0>()) : abs(u);
  u = abs(u) - 0.7155f;
  u = abs(u) - 0.5f;
  f32x3 col = f32x3(0.5f, 0.5f, 0.5f) * 0.35f -
              mod(floor((st * 20.f).data[0]) +
                      mod(floor((st * 20.f).data[1]) + 1.f, 2.f),
                  2.f) *
                  0.15f -
              max(abs(fract((st * 20.f)) - 0.5f).data[0],
                  abs(fract((st * 20.f)) - 0.5f).data[1]) *
                  0.03f;
  col -= (smoothstep(0.47f, 0.48f,
                     max(abs(fract((st * 20.f)) - 0.5f).data[0],
                         abs(fract((st * 20.f)) - 0.5f).data[1])) *
              0.82f -
          0.01f);
  return col;
}
f32x3 celtic(Context &__ctx, f32x2 u) {
  f32x2 st = u;
  u *= 2.f;
  u = (fract((u * 2.f)) - 0.5f) * 2.f;
  u = u.data[0] * u.data[1] < 0.f ? abs(u.swizzle<1, 0>()) : abs(u);
  u = abs(u) - 0.7155f;
  u = abs(u) - 0.5f;
  f32x3 col = f32x3(0.5f, 0.5f, 0.5f) * 0.35f -
              mod(floor((st * 20.f).data[0]) +
                      mod(floor((st * 20.f).data[1]) + 1.f, 2.f),
                  2.f) *
                  0.15f -
              max(abs(fract((st * 20.f)) - 0.5f).data[0],
                  abs(fract((st * 20.f)) - 0.5f).data[1]) *
                  0.03f;
  col -= (smoothstep(0.47f, 0.48f,
                     max(abs(fract((st * 20.f)) - 0.5f).data[0],
                         abs(fract((st * 20.f)) - 0.5f).data[1])) *
              0.82f -
          0.01f);
  float radius = 0.959f;
  float thick = 0.08f;
  float othick = 0.01f;
  float bthick = 0.013f;
  float blur = 0.06f;
  f32x3 REDNEW = pow(f32x3(0.5f, 0.2f, 0.f) *
                         texture(__ctx.channel1, st * 4.f).swizzle<0, 1, 2>(),
                     f32x3(1.4f)) *
                 2.f;
  col = mix(col, f32x3(0.f, 0.f, 0.f),
            smoothstep(thick * 1.25f + blur, thick * 1.25f - blur,
                       abs(length(u - f32x2(0.5f, -0.5f)) - radius + thick)));
  col = mix(col, REDNEW,
            smoothstep(thick + 2.f / iResolution.data[1],
                       thick - 2.f / iResolution.data[1],
                       abs(length(u - f32x2(0.5f, -0.5f)) - radius + thick)));
  col =
      mix(col, f32x3(0.f, 0.f, 0.f),
          smoothstep(bthick + 2.f / iResolution.data[1] * 2.f,
                     bthick - 2.f / iResolution.data[1] * 2.f,
                     abs(abs(length(u - f32x2(0.5f, -0.5f)) - radius + thick) -
                         (thick - bthick))));
  col =
      mix(col,
          f32x3(0.9f, 0.5f, 0.3f) *
              (0.1f + texture(__ctx.channel0, st * 2.f).swizzle<0, 1, 2>()),
          smoothstep(othick + 2.f / iResolution.data[1] * 2.f,
                     othick - 2.f / iResolution.data[1] * 2.f,
                     abs(abs(length(u - f32x2(0.5f, -0.5f)) - radius + thick) -
                         (thick - othick))));
  col =
      mix(col, f32x3(0.f, 0.f, 0.f),
          smoothstep(bthick * 0.2f + 2.f / iResolution.data[1] * 2.f,
                     bthick * 0.2f - 2.f / iResolution.data[1] * 2.f,
                     abs(abs(length(u - f32x2(0.5f, -0.5f)) - radius + thick) -
                         (thick - bthick * 0.2f))));
  col = mix(col, f32x3(0.f, 0.f, 0.f),
            smoothstep(thick * 1.25f + blur, thick * 1.25f - blur,
                       abs(length(u - f32x2(0.5f, 0.5f)) - radius + thick)));
  col = mix(col, REDNEW,
            smoothstep(thick + 2.f / iResolution.data[1],
                       thick - 2.f / iResolution.data[1],
                       abs(length(u - f32x2(0.5f, 0.5f)) - radius + thick)));
  col = mix(col, f32x3(0.f, 0.f, 0.f),
            smoothstep(bthick + 2.f / iResolution.data[1] * 2.f,
                       bthick - 2.f / iResolution.data[1] * 2.f,
                       abs(abs(length(u - f32x2(0.5f, 0.5f)) - radius + thick) -
                           (thick - bthick))));
  col = mix(col,
            f32x3(0.9f, 0.5f, 0.3f) *
                (0.1f + texture(__ctx.channel0, st * 2.f).swizzle<0, 1, 2>()),
            smoothstep(othick + 2.f / iResolution.data[1] * 2.f,
                       othick - 2.f / iResolution.data[1] * 2.f,
                       abs(abs(length(u - f32x2(0.5f, 0.5f)) - radius + thick) -
                           (thick - othick))));
  col = mix(col, f32x3(0.f, 0.f, 0.f),
            smoothstep(bthick * 0.2f + 2.f / iResolution.data[1] * 2.f,
                       bthick * 0.2f - 2.f / iResolution.data[1] * 2.f,
                       abs(abs(length(u - f32x2(0.5f, 0.5f)) - radius + thick) -
                           (thick - bthick * 0.2f))));
  col = mix(col, f32x3(0.f, 0.f, 0.f),
            smoothstep(thick * 1.25f + blur, thick * 1.25f - blur,
                       abs(length(u - f32x2(-0.5f, 0.5f)) - radius + thick)));
  col = mix(col, REDNEW,
            smoothstep(thick + 2.f / iResolution.data[1],
                       thick - 2.f / iResolution.data[1],
                       abs(length(u - f32x2(-0.5f, 0.5f)) - radius + thick)));
  col =
      mix(col, f32x3(0.f, 0.f, 0.f),
          smoothstep(bthick + 2.f / iResolution.data[1] * 2.f,
                     bthick - 2.f / iResolution.data[1] * 2.f,
                     abs(abs(length(u - f32x2(-0.5f, 0.5f)) - radius + thick) -
                         (thick - bthick))));
  col =
      mix(col,
          f32x3(0.9f, 0.5f, 0.3f) *
              (0.1f + texture(__ctx.channel0, st * 2.f).swizzle<0, 1, 2>()),
          smoothstep(othick + 2.f / iResolution.data[1] * 2.f,
                     othick - 2.f / iResolution.data[1] * 2.f,
                     abs(abs(length(u - f32x2(-0.5f, 0.5f)) - radius + thick) -
                         (thick - othick))));
  col =
      mix(col, f32x3(0.f, 0.f, 0.f),
          smoothstep(bthick * 0.2f + 2.f / iResolution.data[1] * 2.f,
                     bthick * 0.2f - 2.f / iResolution.data[1] * 2.f,
                     abs(abs(length(u - f32x2(-0.5f, 0.5f)) - radius + thick) -
                         (thick - bthick * 0.2f))));
  return col;
}
f32x3 triplanar(Context &__ctx, f32x3 p, f32x3 n) {
  n = abs(n);
  return mix(mix(celtic(__ctx, p.swizzle<1, 2>()),
                 celtic(__ctx, p.swizzle<0, 1>()), n.data[2]),
             celtic(__ctx, p.swizzle<0, 2>()), n.data[1]);
}
f32x3 WorldToCube(Context &__ctx, f32x3 p) {
  f32x3 ap = abs(p);
  f32x3 sp = sign(p);
  float m = max(ap.data[0], max(ap.data[1], ap.data[2]));
  f32x3 st;
  if (m == ap.data[0])
    st = f32x3(p.swizzle<2, 1>(), 1.f * sp.data[0]);
  else if (m == ap.data[1])
    st = f32x3(p.swizzle<2, 0>(), 2.f * sp.data[1]);
  else
    st = f32x3(p.swizzle<0, 1>(), 3.f * sp.data[2]);
  st.set_swizzle<0, 1>(st.swizzle<0, 1>() / (m));
  st.set_swizzle<0, 1>(st.swizzle<0, 1>() *
                       (3.0f / (2.0f + abs(st.swizzle<0, 1>()))));
  return st;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 4, -1);
  f32x3 light = normalize(f32x3(-1.f, 2.f, 1.f));
  f32x3 light2 = normalize(f32x3(1.f, 2.f, -1.f));
  f32x3 rd = camRay(__ctx, uv, ro, f32x3(0, 0.f, 0), 1.f);
  f32x3 col =
      celtic(__ctx, uv.swizzle<0, 1>() + iTime * 0.05f) *
      vig(__ctx, (fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>())) *
      f32x3(0.7f, 0.5f, 0.3f) * 0.2f;
  col = mix(
      col, f32x3(0.f, 0.f, 0.f),
      smoothstep(0.f, 0.3f, (1.f - length(uv + f32x2(-0.09f, 0.1f)) - 0.5f)) *
          0.8f);
  float t = RayMarch(__ctx, ro, rd);
  if (t < 100.f) {
    f32x3 p = ro + rd * t;
    f32x3 n = GetNormal(__ctx, p);
    float ld = length(light - p);
    float ld2 = length(light2 - p);
    p.set_swizzle<1, 2>(
        p.swizzle<1, 2>() *
        (rot(__ctx, -m.data[1] * acos(-1.f) + 1.f + iTime * 0.2f)));
    p.set_swizzle<0, 2>(
        p.swizzle<0, 2>() *
        (rot(__ctx, -m.data[0] * acos(-1.f) * 2.f + iTime * 0.2f)));
    f32x3 cubeCoords = WorldToCube(__ctx, p);
    float attn = 1.f / (0.3f + ld * 0.2f + ld * ld);
    float diff = max(0.f, dot(n, light)) * 0.8f + 0.05f;
    float spec = pow(max(0.f, dot(reflect(n, -light), rd)), 100.f);
    float attn2 = 1.f / (0.3f + ld2 * 0.2f + ld2 * ld2);
    float diff2 = max(0.f, dot(n, light2)) * 0.8f + 0.05f;
    float spec2 = pow(max(0.f, dot(reflect(n, -light2), rd)), 100.f);
    float ref =
        texture(__ctx.channel2, -reflect(n, rd)).swizzle<0, 1, 2>().data[0];
    f32x3 tp_tex = triplanar(__ctx, p * 0.5f, n);
    f32x3 sc_tex = celtic(__ctx, map_from_sphere(__ctx, p));
    f32x3 cc_text =
        celtic(__ctx, (cubeCoords.swizzle<0, 1>() * 0.5f + cubeCoords.data[2]));
    f32x3 col1 = 1.4f * clamp(cc_text, 0.f, 1.f) * diff * 1.6f * attn +
                 ref * 0.0f * diff * f32x3(0.1f, 0.4f, 0.5f) +
                 f32x3(0.1f, 0.4f, 0.5f) * spec * .08f;
    f32x3 col2 = 1.4f * clamp(cc_text, 0.f, 1.f) * diff2 * 1.6f * attn2 *
                     f32x3(0.5f, 0.2f, 0.f) +
                 ref * 0.0f * diff2 + f32x3(0.5f, 0.2f, 0.f) * spec2 * .08f;
    col = (col1 + col2);
  }
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1,
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
  Context __ctx{{channel0, sampling0},
                {channel1, sampling1},
                {channel2, sampling2},
                {channel3, sampling3}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
