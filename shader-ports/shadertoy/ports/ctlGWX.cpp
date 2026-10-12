// Port of https://www.shadertoy.com/view/ctlGWX; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctlGWX.json.
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
float opSmoothUnion(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float SimpleVesicaDistanceY(Context &__ctx, f32x2 p, float r, float d) {
  p.data[0] = abs(p.data[0]);
  p.data[0] += d;
  return length(p) - r;
}
float dSlopeLines(Context &__ctx, f32x2 p) {
  float lineSize = 200.f;
  float d = tan((mix(p.data[0], p.data[1], 0.5f) + (-iTime * 5.f / lineSize)) *
                lineSize) *
            lineSize;
  return d;
}
float dDots(Context &__ctx, f32x2 p, float s, float dist, float speed) {
  p -= iTime * speed;
  p = mod(p, dist) - (dist * 0.5f);
  float d = length(p) - s;
  return d;
}
float roundBar(Context &__ctx, f32x2 p, f32x2 s) {
  float d =
      max(abs(p).data[0] - f32x2(s).data[0], abs(p).data[1] - f32x2(s).data[1]);
  p.data[0] = abs(p.data[0]) - s.data[0];
  float d2 = length(p) - s.data[1];
  return min(d, d2);
}
float char2(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = max(abs(p - f32x2(0.1f / 5.f * 2.f, 0.1f / 5.f * 4.f)).data[0] -
                    f32x2(0.1f / 5.f * 3.f, 0.1f / 5.f).data[0],
                abs(p - f32x2(0.1f / 5.f * 2.f, 0.1f / 5.f * 4.f)).data[1] -
                    f32x2(0.1f / 5.f * 3.f, 0.1f / 5.f).data[1]);
  float d2 = max(abs(p - f32x2(0.1f / 5.f * 4.f, 0.1f / 5.f * 2.f)).data[0] -
                     f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[0],
                 abs(p - f32x2(0.1f / 5.f * 4.f, 0.1f / 5.f * 2.f)).data[1] -
                     f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[1]);
  d = min(d, d2);
  d2 = max(abs(p - f32x2(0.1f / 5.f * 2.f, 0.f)).data[0] -
               f32x2(0.1f / 5.f * 3.f, 0.1f / 5.f).data[0],
           abs(p - f32x2(0.1f / 5.f * 2.f, 0.f)).data[1] -
               f32x2(0.1f / 5.f * 3.f, 0.1f / 5.f).data[1]);
  d = min(d, d2);
  d2 = max(abs(p - f32x2(0.f, -0.1f / 5.f * 4.f)).data[0] -
               f32x2(0.1f, 0.1f / 5.f).data[0],
           abs(p - f32x2(0.f, -0.1f / 5.f * 4.f)).data[1] -
               f32x2(0.1f, 0.1f / 5.f).data[1]);
  d = min(d, d2);
  p -= f32x2(-0.1f / 5.f * 2.f, 0.1f / 5.f * 2.f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[0],
           abs(p).data[1] - f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[1]);
  d = min(d, d2);
  p = prevP;
  p -= f32x2(-0.1f / 5.f * 2.f, -0.1f / 5.f * 2.f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[0],
           abs(p).data[1] - f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[1]);
  d = min(d, d2);
  return d;
}
float char3(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] = abs(p.data[1]);
  float d = max(abs(p - f32x2(0.f, 0.1f / 5.f * 4.f)).data[0] -
                    f32x2(0.1f, 0.1f / 5.f).data[0],
                abs(p - f32x2(0.f, 0.1f / 5.f * 4.f)).data[1] -
                    f32x2(0.1f, 0.1f / 5.f).data[1]);
  p = prevP;
  p.data[0] = abs(p.data[0]);
  float d2 = max(abs(p - f32x2(0.1f / 5.f * 4.f, -0.1f / 5.f * 2.f)).data[0] -
                     f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[0],
                 abs(p - f32x2(0.1f / 5.f * 4.f, -0.1f / 5.f * 2.f)).data[1] -
                     f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[1]);
  d = min(d, d2);
  p = prevP;
  d2 = max(abs(p - f32x2(0.1f / 5.f * 2.f, 0.f)).data[0] -
               f32x2(0.1f / 5.f * 3.f, 0.1f / 5.f).data[0],
           abs(p - f32x2(0.1f / 5.f * 2.f, 0.f)).data[1] -
               f32x2(0.1f / 5.f * 3.f, 0.1f / 5.f).data[1]);
  d = min(d, d2);
  p -= f32x2(0.1f / 5.f * 2.f, 0.1f / 5.f * 2.f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[0],
           abs(p).data[1] - f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[1]);
  d = min(d, d2);
  return d;
}
float char0(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = max(abs(p - f32x2(-0.1f / 5.f * 4.f, 0.f)).data[0] -
                    f32x2(0.1f / 5.f, 0.1f).data[0],
                abs(p - f32x2(-0.1f / 5.f * 4.f, 0.f)).data[1] -
                    f32x2(0.1f / 5.f, 0.1f).data[1]);
  float d2 = max(abs(p - f32x2(-0.1f / 5.f * 2.f, 0.1f / 5.f * 4.f)).data[0] -
                     f32x2(0.1f / 5.f * 3.f, 0.1f / 5.f).data[0],
                 abs(p - f32x2(-0.1f / 5.f * 2.f, 0.1f / 5.f * 4.f)).data[1] -
                     f32x2(0.1f / 5.f * 3.f, 0.1f / 5.f).data[1]);
  d = min(d, d2);
  d2 = max(abs(p - f32x2(0.0f, -0.1f / 5.f * 4.f)).data[0] -
               f32x2(0.1f, 0.1f / 5.f).data[0],
           abs(p - f32x2(0.0f, -0.1f / 5.f * 4.f)).data[1] -
               f32x2(0.1f, 0.1f / 5.f).data[1]);
  d = min(d, d2);
  p -= f32x2(0.1f / 5.f * 2.f, 0.1f / 5.f * 2.f);
  p *= f32x2x2(cos(radians(-45.f)), -sin(radians(-45.f)), sin(radians(-45.f)),
               cos(radians(-45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[0],
           abs(p).data[1] - f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[1]);
  d = min(d, d2);
  p = prevP;
  d2 = max(abs(p - f32x2(0.1f / 5.f * 4.f, -0.1f / 5.f * 2.f)).data[0] -
               f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[0],
           abs(p - f32x2(0.1f / 5.f * 4.f, -0.1f / 5.f * 2.f)).data[1] -
               f32x2(0.1f / 5.f, 0.1f / 5.f * 3.f).data[1]);
  d = min(d, d2);
  return d;
}
float bonsaiLeafBase(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = length(p) - 0.06f;
  p.data[0] = abs(p.data[0]) - 0.06f;
  p.data[1] += 0.02f;
  float d2 = length(p) - 0.06f;
  d = min(d, d2);
  p = prevP;
  p.data[0] = abs(p.data[0]) - 0.1f;
  p.data[1] += 0.05f;
  d2 = length(p) - 0.06f;
  d = min(d, d2);
  p = prevP;
  p.data[1] += 0.05f;
  d2 = length(p) - 0.06f;
  d = min(d, d2);
  return d;
}
float bonsaiLeaf(Context &__ctx, f32x2 p, float dir) {
  f32x2 prevP = p;
  float d = abs(bonsaiLeafBase(__ctx, p)) - 0.002f;
  float dots = dDots(__ctx, p, 0.001f, 0.02f, 0.01f);
  dots = max(dots, bonsaiLeafBase(__ctx, p));
  d = min(d, dots);
  p.data[1] += 0.0985f;
  p *= f32x2x2(
      cos(radians(20.f * iTime * dir)), -sin(radians(20.f * iTime * dir)),
      sin(radians(20.f * iTime * dir)), cos(radians(20.f * iTime * dir)));
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (4.0f * 8.0f),
                          6.28f / ((4.0f * 8.0f) * 0.5f)) +
                      (4.0f - 1.f) * 6.28f / (4.0f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.045f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  p.data[1] *= 0.4f;
  float d2 = abs(SimpleVesicaDistanceY(__ctx, p, 0.017f, 0.012f)) - 0.001f;
  p = prevP;
  d2 = max(bonsaiLeafBase(__ctx, p), d2);
  d = min(d, d2);
  return d;
}
float branches(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[1] += 0.095f;
  p *= f32x2x2(cos(radians(20.f)), -sin(radians(20.f)), sin(radians(20.f)),
               cos(radians(20.f)));
  float d = max(abs(p).data[0] - f32x2(0.045f, 0.15f).data[0],
                abs(p).data[1] - f32x2(0.045f, 0.15f).data[1]);
  p = prevP;
  p -= f32x2(-0.03f, 0.06f);
  p *= f32x2x2(cos(radians(30.f)), -sin(radians(30.f)), sin(radians(30.f)),
               cos(radians(30.f)));
  float d2 = max(abs(p).data[0] - f32x2(0.13f, 0.04f).data[0],
                 abs(p).data[1] - f32x2(0.13f, 0.04f).data[1]);
  d = min(d, d2);
  p = prevP;
  p -= f32x2(-0.065f, 0.2f);
  p *= f32x2x2(cos(radians(-60.f)), -sin(radians(-60.f)), sin(radians(-60.f)),
               cos(radians(-60.f)));
  d2 = max(abs(p).data[0] - f32x2(0.13f, 0.03f).data[0],
           abs(p).data[1] - f32x2(0.13f, 0.03f).data[1]);
  d = min(d, d2);
  p = prevP;
  p -= f32x2(-0.14f, -0.05f);
  p *= f32x2x2(cos(radians(-10.f)), -sin(radians(-10.f)), sin(radians(-10.f)),
               cos(radians(-10.f)));
  d2 = max(abs(p).data[0] - f32x2(0.13f, 0.02f).data[0],
           abs(p).data[1] - f32x2(0.13f, 0.02f).data[1]);
  d = min(d, d2);
  p = prevP;
  p -= f32x2(0.2f, -0.08f);
  p *= f32x2x2(cos(radians(-15.f)), -sin(radians(-15.f)), sin(radians(-15.f)),
               cos(radians(-15.f)));
  d2 = max(abs(p).data[0] - f32x2(0.2f, 0.02f).data[0],
           abs(p).data[1] - f32x2(0.2f, 0.02f).data[1]);
  d = min(d, d2);
  p = prevP;
  p -= f32x2(0.1f, 0.15f);
  p *= f32x2x2(cos(radians(-20.f)), -sin(radians(-20.f)), sin(radians(-20.f)),
               cos(radians(-20.f)));
  d2 = max(abs(p).data[0] - f32x2(0.2f, 0.02f).data[0],
           abs(p).data[1] - f32x2(0.2f, 0.02f).data[1]);
  d = min(d, d2);
  return d;
}
float bonsai(Context &__ctx, f32x2 p) {
  p *= 1.7f;
  f32x2 prevP = p;
  float d = abs(branches(__ctx, p)) - 0.002f;
  float dots = dDots(__ctx, p, 0.0035f, 0.03f, -0.02f);
  dots = max(dots, branches(__ctx, p));
  d = min(d, dots);
  float d2 = bonsaiLeafBase(__ctx, p - f32x2(0.38f, 0.05f));
  d = max(-d2, d);
  d2 = bonsaiLeafBase(__ctx, p - f32x2(-0.2f, -0.01f));
  d = max(-d2, d);
  d2 = bonsaiLeafBase(__ctx, p - f32x2(-0.02f, 0.37f));
  d = max(-d2, d);
  d2 = bonsaiLeafBase(__ctx, p - f32x2(0.07f, 0.12f));
  d = max(-d2, d);
  d2 = bonsaiLeafBase(__ctx, p - f32x2(0.29f, 0.28f));
  d = max(-d2, d);
  d2 = bonsaiLeafBase(__ctx, p - f32x2(-0.25f, 0.2f));
  d = max(-d2, d);
  d2 = bonsaiLeaf(__ctx, p - f32x2(0.38f, 0.05f), 1.f);
  d = min(d, d2);
  d2 = bonsaiLeaf(__ctx, p - f32x2(-0.2f, -0.01f), -1.f);
  d = min(d, d2);
  d2 = bonsaiLeaf(__ctx, p - f32x2(-0.02f, 0.37f), 1.f);
  d = min(d, d2);
  d2 = bonsaiLeaf(__ctx, p - f32x2(0.07f, 0.12f), -1.f);
  d = min(d, d2);
  d2 = bonsaiLeaf(__ctx, p - f32x2(0.29f, 0.28f), 1.f);
  d = min(d, d2);
  d2 = bonsaiLeaf(__ctx, p - f32x2(-0.25f, 0.2f), -1.f);
  d = min(d, d2);
  d = max(-(p.data[1] + 0.2f), d);
  d2 =
      abs(max(
          abs(p - f32x2(0.0f, -0.22f)).data[0] - f32x2(0.17f, 0.02f).data[0],
          abs(p - f32x2(0.0f, -0.22f)).data[1] - f32x2(0.17f, 0.02f).data[1])) -
      0.002f;
  d = min(d, d2);
  d2 = max(abs(p - f32x2(0.0f, -0.3f)).data[0] - f32x2(0.2f, 0.06f).data[0],
           abs(p - f32x2(0.0f, -0.3f)).data[1] - f32x2(0.2f, 0.06f).data[1]);
  float a = radians(-10.f);
  p.data[0] = abs(p.data[0]) - 0.18f;
  d2 = max(dot(p, f32x2(cos(a), sin(a))), d2);
  p = prevP;
  float slope = dSlopeLines(__ctx, p);
  slope = max(d2, slope);
  d = min(d, abs(d2) - 0.002f);
  d = min(d, slope);
  d2 =
      abs(max(
          abs(p - f32x2(0.0f, -0.37f)).data[0] - f32x2(0.08f, 0.01f).data[0],
          abs(p - f32x2(0.0f, -0.37f)).data[1] - f32x2(0.08f, 0.01f).data[1])) -
      0.002f;
  d = min(d, d2);
  return d;
}
float infoGraphic1(Context &__ctx, f32x2 p, float dir, float s) {
  p *= f32x2x2(
      cos(radians(10.f * iTime * dir)), -sin(radians(10.f * iTime * dir)),
      sin(radians(10.f * iTime * dir)), cos(radians(10.f * iTime * dir)));
  p *= s;
  f32x2 prevP = p;
  float size = 0.05f;
  float d = abs(length(p - f32x2(0.0f, size)) - size) - 0.001f;
  float d2 = abs(length(p - f32x2(0.0f, -size)) - size) - 0.001f;
  d = min(d, d2);
  d2 = abs(length(p - f32x2(-size, 0.0f)) - size) - 0.001f;
  d = min(d, d2);
  d2 = abs(length(p - f32x2(size, 0.0f)) - size) - 0.001f;
  d = min(d, d2);
  return d;
}
float infoGraphic2(Context &__ctx, f32x2 p) {
  float d = roundBar(__ctx, p - f32x2(0.0f, 0.05f), f32x2(0.1f, 0.03f));
  float d2 = roundBar(__ctx, p - f32x2(0.06f, -0.05f), f32x2(0.1f, 0.03f));
  d = min(d, d2);
  d2 = max(abs(p - f32x2(0.03f, 0.00f)).data[0] - f32x2(0.04f, 0.03f).data[0],
           abs(p - f32x2(0.03f, 0.00f)).data[1] - f32x2(0.04f, 0.03f).data[1]);
  d = opSmoothUnion(__ctx, d, d2, 0.02f);
  return abs(d) - 0.001f;
}
float infoGraphic3(Context &__ctx, f32x2 p) {
  p *= 2.f;
  f32x2 prevP = p;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (2.f * 8.0f),
                          6.28f / ((2.f * 8.0f) * 0.5f)) +
                      (2.f - 1.f) * 6.28f / (2.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.042f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  p.data[1] *= 0.4f;
  float d = abs(SimpleVesicaDistanceY(__ctx, p, 0.025f, 0.012f)) - 0.001f;
  p = prevP;
  p *= f32x2x2(cos(radians(23.f)), -sin(radians(23.f)), sin(radians(23.f)),
               cos(radians(23.f)));
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (2.f * 8.0f),
                          6.28f / ((2.f * 8.0f) * 0.5f)) +
                      (2.f - 1.f) * 6.28f / (2.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.06f);
  float d2 = length(p) - 0.007f;
  d = min(d, d2);
  return d;
}
float infoGraphic4(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (2.f * 8.0f),
                          6.28f / ((2.f * 8.0f) * 0.5f)) +
                      (2.f - 1.f) * 6.28f / (2.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.02f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  p.data[1] *= 0.9f;
  float d = abs(SimpleVesicaDistanceY(__ctx, p, 0.02f, 0.015f)) - 0.001f;
  p = prevP;
  p *= f32x2x2(cos(radians(-20.f)), -sin(radians(-20.f)), sin(radians(-20.f)),
               cos(radians(-20.f)));
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (1.25f * 8.0f),
                          6.28f / ((1.25f * 8.0f) * 0.5f)) +
                      (1.25f - 1.f) * 6.28f / (1.25f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.05f);
  float d2 = abs(length(p) - 0.05f) - 0.001f;
  p = prevP;
  d2 = max(-(length(p) - 0.08f), d2);
  d = min(d, d2);
  return d;
}
float eye(Context &__ctx, f32x2 p) {
  p.data[0] = abs(p.data[0]) - 0.03f;
  p.data[1] += 0.03f;
  float d = max(abs(p).data[0] - f32x2(0.001f, 0.01f).data[0],
                abs(p).data[1] - f32x2(0.001f, 0.01f).data[1]);
  float d2 = max(abs(p).data[0] - f32x2(0.008f, 0.001f).data[0],
                 abs(p).data[1] - f32x2(0.008f, 0.001f).data[1]);
  return min(d, d2);
}
float bunny(Context &__ctx, f32x2 p) {
  f32x2 prevP2 = p;
  p.data[1] -= iTime * 0.1f;
  p *= 1.5f;
  p.data[0] = mod(p.data[0], 0.16f) - 0.08f;
  p.data[1] = mod(p.data[1], 0.26f) - 0.13f;
  p.data[1] += 0.025f;
  f32x2 prevP = p;
  p.data[0] = abs(p.data[0]) - 0.04f;
  p.data[1] -= 0.05f;
  p *= f32x2x2(cos(radians(10.f)), -sin(radians(10.f)), sin(radians(10.f)),
               cos(radians(10.f)));
  float d = SimpleVesicaDistanceY(__ctx, p, 0.2f, 0.17f);
  p = prevP;
  p.data[1] *= 1.2f;
  float d2 = length(p - f32x2(0.f, -0.05f)) - 0.07f;
  d = abs(min(d, d2)) - 0.001f;
  p = prevP;
  d2 = eye(__ctx, p);
  d = min(d, d2);
  p = prevP2;
  d = max(-(length(p) - 0.5f), d);
  return d;
}
float year(Context &__ctx, f32x2 p) {
  float d = char2(__ctx, p - f32x2(-0.375f, 0.0f));
  float d2 = char0(__ctx, p - f32x2(-0.125f, 0.0f));
  d = min(d, d2);
  d2 = char2(__ctx, p - f32x2(0.125f, 0.0f));
  d = min(d, d2);
  d2 = char3(__ctx, p - f32x2(0.375f, 0.0f));
  d = min(d, d2);
  return d;
}
float sun(Context &__ctx, f32x2 p) {
  p *= 1.6f;
  f32x2 prevP = p;
  float speed = -0.05f;
  float d = length(p) - 0.2f;
  p += iTime * speed;
  p = mod(p, 0.1f) - 0.05f;
  float d2 = abs(length(p) - 0.05f) - 0.001f;
  p = prevP;
  p += iTime * speed;
  p -= f32x2(0.05f);
  p = mod(p, 0.1f) - 0.05f;
  float d3 = abs(length(p) - 0.05f) - 0.001f;
  d2 = min(d2, d3);
  d2 = min(abs(d) - 0.002f, d2);
  d = max(d, d2);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 prevP = p;
  f32x3 col = f32x3(0.f);
  float d = bunny(__ctx, p);
  col = mix(col, f32x3(0.7f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  p.data[0] += iTime * 0.1f;
  p.data[0] += 0.3f;
  p.data[0] = mod(p.data[0], 0.6f) - 0.3f;
  d = infoGraphic2(__ctx, p - f32x2(0.0f, 0.25f));
  p = prevP;
  d = max((length(p) - 0.38f), d);
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  d = length(p - f32x2(-0.2f, 0.16f)) - 0.125f;
  col = mix(col, f32x3(0.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  d = sun(__ctx, p - f32x2(-0.2f, 0.16f));
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  d = bonsai(__ctx, p - f32x2(0.02f, -0.12f));
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  d = infoGraphic1(__ctx, p - f32x2(0.2f, -0.25f), 1.5f, 2.2f);
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  d = infoGraphic1(__ctx, p - f32x2(-0.27f, -0.19f), -1.f, 2.5f);
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  d = infoGraphic1(__ctx, p - f32x2(-0.15f, -0.28f), 2.f, 2.f);
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  p *= f32x2x2(cos(radians(-10.f * iTime)), -sin(radians(-10.f * iTime)),
               sin(radians(-10.f * iTime)), cos(radians(-10.f * iTime)));
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (4.5f * 8.0f),
                          6.28f / ((4.5f * 8.0f) * 0.5f)) +
                      (4.5f - 1.f) * 6.28f / (4.5f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.4f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  d = infoGraphic3(__ctx, p - f32x2(0.02f, -0.12f));
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  p = prevP;
  d = year(__ctx, p);
  col *= mix(col, f32x3(0.1f),
             smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                        0.0f, d));
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, abs(d) - 0.001f));
  fragColor = f32x4(sqrt(col), 1.0f);
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
