// Port of https://www.shadertoy.com/view/Dlj3zd; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dlj3zd.json.
#include "../support.h"
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
float dot2(Context &__ctx, f32x2 v) { return dot(v, v); }
float smin(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sdCappedCylinderX(Context &__ctx, f32x3 p, float h, float r) {
  f32x2 d = abs(f32x2(length(p.swizzle<1, 2>()), p.data[0])) - f32x2(h, r);
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
float sdCappedCylinderY(Context &__ctx, f32x3 p, float h, float r) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 2>()), p.data[1])) - f32x2(h, r);
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
float sdCappedCylinderZ(Context &__ctx, f32x3 p, float h, float r) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 1>()), p.data[2])) - f32x2(h, r);
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
float sdHorizontalCapsule(Context &__ctx, f32x3 p, float w, float r) {
  p.data[0] -= clamp(p.data[0], -w * 0.5f, w * 0.5f);
  return length(p) - r;
}
float sdCappedCone(Context &__ctx, f32x3 p, float h, float r1, float r2) {
  f32x2 q = f32x2(length(p.swizzle<1, 2>()), p.data[0]);
  f32x2 k1 = f32x2(r2, h);
  f32x2 k2 = f32x2(r2 - r1, 2.0f * h);
  f32x2 ca = f32x2(q.data[0] - min(q.data[0], (q.data[1] < 0.0f) ? r1 : r2),
                   abs(q.data[1]) - h);
  f32x2 cb = q - k1 + k2 * clamp(dot(k1 - q, k2) / dot2(__ctx, k2), 0.0f, 1.0f);
  float s = (cb.data[0] < 0.0f && ca.data[1] < 0.0f) ? -1.0f : 1.0f;
  return s * sqrt(min(dot2(__ctx, ca), dot2(__ctx, cb)));
}
float largeLensParts(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  float d = sdBox(__ctx, p, f32x3(0.13f, 0.13f, 0.13f));
  float a = radians(-70.f);
  p.data[1] = abs(p.data[1]) - 0.1f;
  d = max(-dot(p, f32x3(cos(a), sin(a), 0.0f)), d);
  p = prevP;
  p.data[2] = abs(p.data[2]) - 0.1f;
  d = max(-dot(p, f32x3(cos(a), 0.0f, sin(a))), d);
  p = prevP;
  d = max(p.data[0] - 0.05f, d);
  return d;
}
float largeLens(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  float d = largeLensParts(__ctx, p);
  p *= 1.1f;
  float d2 = largeLensParts(__ctx, p - f32x3(0.01f, 0.0f, 0.0f));
  d = max(-d2, d);
  p = prevP;
  d2 = sdCappedCylinderX(__ctx, p - f32x3(-0.1f, 0.0f, 0.0f), 0.035f, 0.01f) -
       0.02f;
  d = min(d, d2);
  return d;
}
float mainLens(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  float d = sdCappedCylinderX(__ctx, p, 0.08f, 0.02f) - 0.02f;
  float d2 =
      sdCappedCylinderX(__ctx, p - f32x3(0.06f, 0.0f, 0.05f), 0.04f, 0.055f);
  d = min(d, d2);
  d2 = sdCappedCylinderX(__ctx, p - f32x3(0.02f, 0.04f, -0.03f), 0.03f, 0.05f);
  float mask =
      sdCappedCylinderX(__ctx, p - f32x3(0.07f, 0.04f, -0.03f), 0.025f, 0.01f);
  d2 = max(-mask, d2);
  d = min(d, d2);
  d2 =
      sdCappedCylinderX(__ctx, p - f32x3(0.03f, -0.04f, -0.03f), 0.032f, 0.05f);
  mask =
      sdCappedCylinderX(__ctx, p - f32x3(0.08f, -0.04f, -0.03f), 0.027f, 0.01f);
  d2 = max(-mask, d2);
  d = min(d, d2);
  d2 = sdCappedCylinderX(__ctx, p - f32x3(0.105f, 0.0f, 0.05f), 0.06f, 0.01f);
  d = min(d, d2);
  d2 = largeLens(__ctx, p - f32x3(0.25f, 0.0f, 0.05f));
  d = min(d, d2);
  return d;
}
float viewFinder(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  float d = sdBox(__ctx, p, f32x3(0.035f));
  float d2 =
      sdCappedCylinderX(__ctx, p - f32x3(-0.03f, 0.0f, 0.0f), 0.025f, 0.1f);
  d = min(d, d2);
  d2 = sdCappedCone(__ctx, p - f32x3(-0.15f, 0.0f, 0.0f), 0.03f, 0.05f, 0.03f);
  float mask =
      sdCappedCone(__ctx, p - f32x3(-0.17f, 0.0f, 0.0f), 0.03f, 0.04f, 0.02f);
  d2 = max(-mask, d2);
  d = min(d, d2);
  return d;
}
float reelParts(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 1>(
      p.swizzle<0, 1>() *
      (f32x2x2(cos(radians(iTime * 30.f)), -sin(radians(iTime * 30.f)),
               sin(radians(iTime * 30.f)), cos(radians(iTime * 30.f)))));
  f32x3 prevP = p;
  p.data[2] = abs(p.data[2]) - 0.04f;
  float d = sdCappedCylinderZ(__ctx, p, 0.17f, 0.01f);
  p.set_swizzle<0, 1>(
      length(p.swizzle<0, 1>()) *
      cos(mod(atan(p.swizzle<0, 1>().data[1], p.swizzle<0, 1>().data[0]) +
                  6.28f / (1.25f * 8.0f),
              6.28f / ((1.25f * 8.0f) * 0.5f)) +
          (1.25f - 1.f) * 6.28f / (1.25f * 8.0f) + f32x2(0, 11)));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() - (f32x2(0.075f)));
  float mask = sdCappedCylinderZ(__ctx, p, 0.05f, 0.1f);
  d = max(-mask, d);
  p = prevP;
  float d2 = sdCappedCylinderZ(__ctx, p, 0.1f, 0.015f) - 0.01f;
  d = min(d, d2);
  return d;
}
float reel(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  p.data[0] = abs(p.data[0]);
  float d = reelParts(__ctx, (p - f32x3(0.17f, 0.0f, 0.0f)) *
                                 f32x3(sign(prevP.data[0]), 1, 1));
  p = prevP;
  p.data[2] = abs(p.data[2]) - 0.07f;
  float d2 =
      sdBox(__ctx, p - f32x3(0.0f, -0.15f, 0.0f), f32x3(0.15f, 0.06f, 0.02f));
  p = prevP;
  float a = radians(30.f);
  p.data[0] = abs(p.data[0]) - 0.1f;
  p.data[1] += 0.1f;
  d2 = max(dot(p, f32x3(cos(a), sin(a), 0.f)), d2);
  d = min(d, d2);
  return d;
}
float body(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  float d = sdBox(__ctx, p, f32x3(0.185f, 0.15f, 0.1f)) - 0.015f;
  p.data[2] = abs(p.data[2]) - 0.08f;
  p.data[0] = abs(p.data[0]);
  p.data[1] = abs(p.data[1]);
  float d2 = length(p - f32x3(0.197f, 0.13f, 0.f)) - 0.013f;
  d = min(d, d2);
  p = prevP;
  d2 = reel(__ctx, p - f32x3(0.0f, 0.34f, 0.0f));
  d = min(d, d2);
  p.data[0] += 0.05f;
  p.data[0] = abs(p.data[0]) - 0.05f;
  d2 = sdCappedCylinderZ(__ctx, p, 0.03f, 0.13f) - 0.01f;
  d = min(d, d2);
  p = prevP;
  p.data[0] -= 0.11f;
  p.data[1] = abs(p.data[1]) - 0.05f;
  d2 = sdCappedCylinderZ(__ctx, p, 0.015f, 0.13f) - 0.01f;
  d = min(d, d2);
  return d;
}
float triPods(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  float d = sdCappedCylinderY(__ctx, p, 0.05f, 0.015f) - 0.01f;
  p.data[0] = abs(p.data[0]) - 0.02f;
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() *
                      (f32x2x2(cos(radians(-30.f)), -sin(radians(-30.f)),
                               sin(radians(-30.f)), cos(radians(-30.f)))));
  float d2 =
      sdCappedCylinderY(__ctx, p - f32x3(0.f, -0.12f, 0.f), 0.005f, 0.1f) -
      0.01f;
  d = min(d, d2);
  p = prevP;
  p.data[2] = abs(p.data[2]) - 0.02f;
  p.set_swizzle<2, 1>(p.swizzle<2, 1>() *
                      (f32x2x2(cos(radians(-30.f)), -sin(radians(-30.f)),
                               sin(radians(-30.f)), cos(radians(-30.f)))));
  d2 = sdCappedCylinderY(__ctx, p - f32x3(0.f, -0.12f, 0.f), 0.005f, 0.1f) -
       0.01f;
  d = min(d, d2);
  return d;
}
f32x2 GetDist(Context &__ctx, f32x3 p) {
  f32x3 prevP = p;
  p.data[1] += 0.05f;
  float d = body(__ctx, p);
  float d2 = mainLens(__ctx, p - f32x3(0.24f, 0.05f, 0.0f));
  d = min(d, d2);
  d2 = viewFinder(__ctx, p - f32x3(-0.23f, 0.0f, 0.0f));
  d = min(d, d2);
  d2 = triPods(__ctx, p - f32x3(0.0f, -0.17f, 0.0f));
  d = min(d, d2);
  return f32x2(d, 0);
}
f32x2 RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float side, int stepnum) {
  f32x2 dO = f32x2(0.0f);
  for (int i = 0; i < stepnum; i++) {
    f32x3 p = ro + rd * dO.data[0];
    f32x2 dS = GetDist(__ctx, p);
    dO.data[0] += dS.data[0] * side;
    dO.data[1] = dS.data[1];
    if (dO.data[0] > 100.f || abs(dS.data[0]) < .0005f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p).data[0];
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 R(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = p + f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i - p);
  return d;
}
float calcOcclusion(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = (min(iFrame, 0)); i < 3; i++) {
    float h = 0.01f + 0.15f * float(i) / 4.0f;
    f32x3 opos = pos + h * nor;
    float d = GetDist(__ctx, opos).data[0];
    occ += (h - d) * sca;
    sca *= 0.95f;
  }
  return clamp(1.0f - 2.0f * occ, 0.0f, 1.0f);
}
f32x3 diffuseMaterial(Context &__ctx, f32x3 n, f32x3 rd, f32x3 p, f32x3 col) {
  float occ = calcOcclusion(__ctx, p, n);
  f32x3 diffCol = f32x3(0.0f);
  f32x3 lightDir = normalize(f32x3(1, 10, 10));
  float diff = clamp(dot(n, lightDir), 0.0f, 1.0f);
  float skyDiff = clamp(0.5f + 0.5f * dot(n, f32x3(0, 1, 0)), 0.0f, 1.0f);
  float bounceDiff = clamp(0.5f + 0.5f * dot(n, f32x3(0, -1, 0)), 0.0f, 1.0f);
  diffCol = col * f32x3(-0.5f) * diff * occ;
  diffCol += col * f32x3(1.0f, 1.0f, 0.95f) * skyDiff * occ;
  diffCol += col * f32x3(0.95f) * bounceDiff * occ;
  diffCol += col * pow(max(dot(rd, reflect(lightDir, n)), 0.0f), 60.f) * occ;
  return diffCol;
}
f32x3 materials(Context &__ctx, int mat, f32x3 n, f32x3 rd, f32x3 p,
                f32x3 col) {
  col = diffuseMaterial(__ctx, n, rd, p, f32x3(1.f));
  return col;
}
float bgItem(Context &__ctx, f32x2 p) {
  p *= f32x2x2(cos(radians(90.f)), -sin(radians(90.f)), sin(radians(90.f)),
               cos(radians(90.f)));
  p.data[1] = abs(p.data[1]) - 0.05f;
  float d = abs(max(-dot(p, f32x2(cos(-radians(30.f)), sin(-radians(30.f)))),
                    max(dot(p, f32x2(cos(radians(30.f)), sin(radians(30.f)))),
                        max(abs(p).data[0] - f32x2(0.05f).data[0],
                            abs(p).data[1] - f32x2(0.05f).data[1])))) -
            0.0001f;
  return d;
}
float bg(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p -= iTime * 0.05f;
  p *= 0.8f;
  p.data[0] = mod(p.data[0], 0.095f) - 0.0475f;
  p.data[1] = mod(p.data[1], 0.055f) - 0.0275f;
  float d = bgItem(__ctx, p);
  return d;
}
float filmItem(Context &__ctx, f32x2 p, float dir) {
  f32x2 prevP = p;
  float d = max(abs(p).data[0] - f32x2(2.f, 0.07f).data[0],
                abs(p).data[1] - f32x2(2.f, 0.07f).data[1]);
  p.data[0] += iTime * 0.1f * dir;
  p.data[0] = mod(p.data[0], 0.16f) - 0.08f;
  float d2 = max(abs(p).data[0] - f32x2(0.075f, 0.035f).data[0],
                 abs(p).data[1] - f32x2(0.075f, 0.035f).data[1]);
  d = max(-d2, d);
  p = prevP;
  p.data[0] += iTime * 0.1f * dir;
  p.data[1] = abs(p.data[1]) - 0.0525f;
  p.data[0] = mod(p.data[0], 0.03f) - 0.015f;
  d2 = max(abs(p).data[0] - f32x2(0.012f).data[0],
           abs(p).data[1] - f32x2(0.012f).data[1]);
  d = max(-d2, d);
  return d;
}
float icon0(Context &__ctx, f32x2 p) {
  p *= f32x2x2(cos(radians(30.f * iTime)), -sin(radians(30.f * iTime)),
               sin(radians(30.f * iTime)), cos(radians(30.f * iTime)));
  f32x2 prevP = p;
  float thickness = 0.002f;
  float d = abs(length(p) - 0.06f) - thickness;
  p.data[0] -= 0.033f;
  p *= f32x2x2(cos(radians(90.f)), -sin(radians(90.f)), sin(radians(90.f)),
               cos(radians(90.f)));
  float d2 = abs(max(-dot(p, f32x2(cos(-radians(30.f)), sin(-radians(30.f)))),
                     max(dot(p, f32x2(cos(radians(30.f)), sin(radians(30.f)))),
                         max(abs(p).data[0] - f32x2(0.05f).data[0],
                             abs(p).data[1] - f32x2(0.05f).data[1])))) -
             thickness;
  d = min(d, d2);
  return d;
}
float icon1(Context &__ctx, f32x2 p) {
  p *= f32x2x2(cos(radians(-22.f * iTime)), -sin(radians(-22.f * iTime)),
               sin(radians(-22.f * iTime)), cos(radians(-22.f * iTime)));
  f32x2 prevP = p;
  p.data[0] *= 3.f;
  float d = abs(length(p) - 0.06f) - 0.003f;
  p = prevP;
  p.data[1] *= 3.f;
  float d2 = abs(length(p) - 0.06f) - 0.003f;
  d = min(d, d2);
  p = prevP;
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  p.data[0] *= 3.f;
  d2 = abs(length(p) - 0.06f) - 0.003f;
  d = min(d, d2);
  p = prevP;
  p *= f32x2x2(cos(radians(-45.f)), -sin(radians(-45.f)), sin(radians(-45.f)),
               cos(radians(-45.f)));
  p.data[0] *= 3.f;
  d2 = abs(length(p) - 0.06f) - 0.003f;
  d = min(d, d2);
  return d;
}
float icon2(Context &__ctx, f32x2 p) {
  p *= f32x2x2(cos(radians(25.f * iTime)), -sin(radians(25.f * iTime)),
               sin(radians(25.f * iTime)), cos(radians(25.f * iTime)));
  f32x2 prevP = p;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (2.f * 8.0f),
                          6.28f / ((2.f * 8.0f) * 0.5f)) +
                      (2.f - 1.f) * 6.28f / (2.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.045f);
  float d = max(-dot(p, f32x2(cos(-radians(45.f)), sin(-radians(45.f)))),
                max(dot(p, f32x2(cos(radians(45.f)), sin(radians(45.f)))),
                    max(abs(p).data[0] - f32x2(0.06f).data[0],
                        abs(p).data[1] - f32x2(0.06f).data[1])));
  return d;
}
float icon3(Context &__ctx, f32x2 p) {
  p *= f32x2x2(cos(radians(-15.f * iTime)), -sin(radians(-15.f * iTime)),
               sin(radians(-15.f * iTime)), cos(radians(-15.f * iTime)));
  f32x2 prevP = p;
  p = length(p.swizzle<0, 1>()) *
      cos(mod(atan(p.swizzle<0, 1>().data[1], p.swizzle<0, 1>().data[0]) +
                  6.28f / (2.f * 8.0f),
              6.28f / ((2.f * 8.0f) * 0.5f)) +
          (2.f - 1.f) * 6.28f / (2.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.045f);
  float d = max(-dot(p, f32x2(cos(-radians(30.f)), sin(-radians(30.f)))),
                max(dot(p, f32x2(cos(radians(30.f)), sin(radians(30.f)))),
                    max(abs(p).data[0] - f32x2(0.06f).data[0],
                        abs(p).data[1] - f32x2(0.06f).data[1])));
  p = prevP;
  d = min(abs(length(p) - 0.015f) - 0.002f, d);
  return d;
}
float random(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float items(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p *= 6.f;
  p -= iTime * 0.2f;
  f32x2 id = floor(p);
  f32x2 grd = fract(p) - 0.5f;
  grd *= 0.15f;
  float n = random(__ctx, id);
  float d = 10.f;
  if (n < 0.1f) {
    d = icon0(__ctx, grd);
  } else if (n >= 0.1f && n < 0.2f) {
    d = icon1(__ctx, grd);
  } else if (n >= 0.2f && n < 0.3f) {
    d = icon2(__ctx, grd);
  } else if (n >= 0.3f && n < 0.4f) {
    d = icon3(__ctx, grd);
  }
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 prevUV = uv;
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 0, -1.35f);
  if (iMouse.data[2] > 0.f) {
    ro.set_swizzle<1, 2>(
        ro.swizzle<1, 2>() *
        (f32x2x2(cos(m.data[1] * 3.14f + 1.f), -sin(m.data[1] * 3.14f + 1.f),
                 sin(m.data[1] * 3.14f + 1.f), cos(m.data[1] * 3.14f + 1.f))));
    ro.data[1] = max(-0.9f, ro.data[1]);
    ro.set_swizzle<0, 2>(
        ro.swizzle<0, 2>() *
        (f32x2x2(cos(-m.data[0] * 6.2831f), -sin(-m.data[0] * 6.2831f),
                 sin(-m.data[0] * 6.2831f), cos(-m.data[0] * 6.2831f))));
  } else {
    ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                         (f32x2x2(cos(radians(-5.0f)), -sin(radians(-5.0f)),
                                  sin(radians(-5.0f)), cos(radians(-5.0f)))));
    ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                         (f32x2x2(cos(radians(sin(iTime * 0.3f) * 60.0f)),
                                  -sin(radians(sin(iTime * 0.3f) * 60.0f)),
                                  sin(radians(sin(iTime * 0.3f) * 60.0f)),
                                  cos(radians(sin(iTime * 0.3f) * 60.0f)))));
  }
  f32x3 rd = R(__ctx, uv, ro, f32x3(0, 0.0f, 0), 1.0f);
  f32x2 d = RayMarch(__ctx, ro, rd, 1.f, 100);
  f32x3 col = f32x3(0.f);
  if (d.data[0] < 100.f) {
    f32x3 p = ro + rd * d.data[0];
    f32x3 n = GetNormal(__ctx, p);
    int mat = int(d.data[1]);
    col = materials(__ctx, mat, n, rd, p, col);
  } else {
    col = mix(col, f32x3(0.2f),
              smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                         0.0f, bg(__ctx, uv)));
    col = mix(col, f32x3(0.4f),
              smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                         0.0f, items(__ctx, uv)));
  }
  uv.data[1] = abs(uv.data[1]);
  float dd = filmItem(__ctx, uv - f32x2(0.0f, 0.43f), sign(prevUV.data[1]));
  col = mix(col, f32x3(0.9f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, dd));
  fragColor = f32x4(col, 1.0f);
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
