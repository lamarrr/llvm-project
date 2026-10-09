// Port of https://www.shadertoy.com/view/clsSz4; author: yasuo.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clsSz4.json.
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
float Hash21(Context &__ctx, f32x2 p) {
  p = fract(p * f32x2(234.56f, 789.34f));
  p += dot(p, p + 34.56f);
  return fract(p.data[0] + p.data[1]);
}
float randomDotLine(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[0] += iTime * 0.08f;
  f32x2 gv = fract(p * 20.0f) - 0.5f;
  f32x2 id = floor(p * 20.0f);
  float n = Hash21(__ctx, id);
  float d = max(abs(gv).data[0] - f32x2(0.25f * (n * 2.0f), 0.2f).data[0],
                abs(gv).data[1] - f32x2(0.25f * (n * 2.0f), 0.2f).data[1]);
  p = prevP;
  p.data[1] += 0.012f;
  d = max(abs(p.data[1]) - 0.01f, max(abs(p.data[0]) - 0.08f, d));
  return d;
}
float segBase(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float size = 0.02f;
  float padding = 0.05f;
  float w = padding * 3.0f;
  float h = padding * 5.0f;
  p = mod(p, 0.05f) - 0.025f;
  float thickness = 0.005f;
  float gridMask = min(abs(p.data[0]) - thickness, abs(p.data[1]) - thickness);
  p = prevP;
  float d = max(abs(p).data[0] - f32x2(w * 0.5f, h * 0.5f).data[0],
                abs(p).data[1] - f32x2(w * 0.5f, h * 0.5f).data[1]);
  float a = radians(40.0f);
  p.data[0] = abs(p.data[0]) - 0.1f;
  p.data[1] = abs(p.data[1]) - 0.05f;
  float d2 = dot(p, f32x2(cos(a), sin(a)));
  return d;
}
float seg0(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  float mask = max(abs(p).data[0] - f32x2(size, size * 2.7f).data[0],
                   abs(p).data[1] - f32x2(size, size * 2.7f).data[1]);
  d = max(-mask, d);
  return d;
}
float seg1(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[0] += size;
  p.data[1] += size;
  float mask = max(abs(p).data[0] - f32x2(size * 2.f, size * 3.7f).data[0],
                   abs(p).data[1] - f32x2(size * 2.f, size * 3.7f).data[1]);
  d = max(-mask, d);
  p = prevP;
  p.data[0] += size * 1.9f;
  p.data[1] -= size * 3.2f;
  mask = max(abs(p).data[0] - f32x2(size, size + 0.01f).data[0],
             abs(p).data[1] - f32x2(size, size + 0.01f).data[1]);
  d = max(-mask, d);
  return d;
}
float seg2(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[0] += size;
  p.data[1] -= 0.05f;
  float mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
                   abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  p = prevP;
  p.data[0] -= size;
  p.data[1] += 0.05f;
  mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
             abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  return d;
}
float seg3(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[1] = abs(p.data[1]);
  p.data[0] += size;
  p.data[1] -= 0.05f;
  float mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
                   abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  p = prevP;
  p.data[0] += 0.06f;
  mask = max(abs(p).data[0] - f32x2(size, size + 0.01f).data[0],
             abs(p).data[1] - f32x2(size, size + 0.01f).data[1]);
  d = max(-mask, d);
  return d;
}
float seg4(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[0] += size;
  p.data[1] += 0.08f;
  float mask = max(abs(p).data[0] - f32x2(size * 2.f, size * 2.0f).data[0],
                   abs(p).data[1] - f32x2(size * 2.f, size * 2.0f).data[1]);
  d = max(-mask, d);
  p = prevP;
  p.data[1] -= 0.08f;
  mask = max(abs(p).data[0] - f32x2(size, size * 2.0f).data[0],
             abs(p).data[1] - f32x2(size, size * 2.0f).data[1]);
  d = max(-mask, d);
  return d;
}
float seg5(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[0] -= size;
  p.data[1] -= 0.05f;
  float mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
                   abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  p = prevP;
  p.data[0] += size;
  p.data[1] += 0.05f;
  mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
             abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  return d;
}
float seg6(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[0] -= size;
  p.data[1] -= 0.05f;
  float mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
                   abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  p = prevP;
  p.data[1] += 0.05f;
  mask = max(abs(p).data[0] - f32x2(size, size).data[0],
             abs(p).data[1] - f32x2(size, size).data[1]);
  d = max(-mask, d);
  return d;
}
float seg7(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[0] += size;
  p.data[1] += size;
  float mask = max(abs(p).data[0] - f32x2(size * 2.f, size * 3.7f).data[0],
                   abs(p).data[1] - f32x2(size * 2.f, size * 3.7f).data[1]);
  d = max(-mask, d);
  return d;
}
float seg8(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[1] = abs(p.data[1]);
  p.data[1] -= 0.05f;
  float mask = max(abs(p).data[0] - f32x2(size, size).data[0],
                   abs(p).data[1] - f32x2(size, size).data[1]);
  d = max(-mask, d);
  return d;
}
float seg9(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = segBase(__ctx, p);
  float size = 0.03f;
  p.data[1] -= 0.05f;
  float mask = max(abs(p).data[0] - f32x2(size, size).data[0],
                   abs(p).data[1] - f32x2(size, size).data[1]);
  d = max(-mask, d);
  p = prevP;
  p.data[0] += size;
  p.data[1] += 0.05f;
  mask = max(abs(p).data[0] - f32x2(size * 2.f, size).data[0],
             abs(p).data[1] - f32x2(size * 2.f, size).data[1]);
  d = max(-mask, d);
  return d;
}
float checkChar(Context &__ctx, int targetChar, int char) {
  return 1.f - abs(sign(float(targetChar) - float(char)));
}
float drawFont(Context &__ctx, f32x2 p, int char) {
  float d = seg0(__ctx, p) * checkChar(__ctx, 0, char);
  d += seg1(__ctx, p) * checkChar(__ctx, 1, char);
  d += seg2(__ctx, p) * checkChar(__ctx, 2, char);
  d += seg3(__ctx, p) * checkChar(__ctx, 3, char);
  d += seg4(__ctx, p) * checkChar(__ctx, 4, char);
  d += seg5(__ctx, p) * checkChar(__ctx, 5, char);
  d += seg6(__ctx, p) * checkChar(__ctx, 6, char);
  d += seg7(__ctx, p) * checkChar(__ctx, 7, char);
  d += seg8(__ctx, p) * checkChar(__ctx, 8, char);
  d += seg9(__ctx, p) * checkChar(__ctx, 9, char);
  return abs(d) - 0.004f;
}
float dLine(Context &__ctx, f32x2 p, f32x2 a, f32x2 b, float w) {
  f32x2 v = normalize(b - a);
  f32x2 right =
      normalize(cross(f32x3(v, 0.0f), f32x3(0.0f, 0.0f, 1.0f)).swizzle<0, 1>());
  f32x2 newRightVec = right * (w * 0.5f);
  f32x2 a1 = a + newRightVec, a2 = a - newRightVec, b1 = b + newRightVec,
        b2 = b - newRightVec;
  f32x2 mida =
      f32x2((a1.data[0] + b1.data[0]) * 0.5f, (a1.data[1] + b1.data[1]) * 0.5f);
  f32x2 midb =
      f32x2((a2.data[0] + b2.data[0]) * 0.5f, (a2.data[1] + b2.data[1]) * 0.5f);
  float rad1 = -atan(b1.data[0] - a1.data[0], b1.data[1] - a1.data[1]);
  float rad2 = -atan(a2.data[0] - a1.data[0], a2.data[1] - a1.data[1]);
  float mad = dot(p - mida, f32x2(cos(rad1), sin(rad1)));
  float mbd = dot(p - midb, f32x2(cos(rad1), sin(rad1)));
  float ad = dot(p - a, f32x2(cos(rad2), sin(rad2)));
  float bd = dot(p - b, f32x2(cos(rad2), sin(rad2)));
  float d = max(max(mad, -mbd), max(-ad, bd));
  return d;
}
float ringItem(Context &__ctx, f32x2 p, float r, float r2, float deg) {
  f32x2 prevP = p;
  float thickness = 0.001f;
  float d = abs(length(p) - r) - thickness;
  float rad = radians(deg * iTime);
  float x = r * cos(rad) + p.data[0];
  float y = r * sin(rad) + p.data[1];
  float d2 = length(f32x2(x, y)) - r2;
  d = max(-d2, d);
  d = min(d, abs(d2) - thickness);
  d2 = length(p) - 0.005f;
  d = min(d, d2);
  d2 = length(f32x2(x, y)) - 0.005f;
  d = min(d, d2);
  return d;
}
float smallCircle(Context &__ctx, f32x2 p, float r) {
  float thickness = 0.001f;
  float d = abs(length(p) - r) - thickness;
  float d2 = length(p) - 0.005f;
  d = min(d, d2);
  return d;
}
float ringCircle(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float thickness = 0.0005f;
  float R = 0.41f;
  float R2 = 0.29f;
  float r = R;
  float r2 = R2;
  float r3 = 0.465f;
  float d = abs(length(p) - r) - thickness;
  for (float i = 0.f; i < 4.f; i++) {
    float startAngle = i * 25.f;
    float rad = radians(startAngle + 10.f * -iTime);
    float x = r * cos(rad) + p.data[0];
    float y = r * sin(rad) + p.data[1];
    float dir = (mod(i, 2.f) == 0.f) ? -1.f : 1.f;
    float d2 = ringItem(__ctx, f32x2(x, y), 0.055f - (i * 0.005f),
                        0.02f - (i * 0.002f), (30.f + (i * 5.f)) * dir);
    d = min(d, d2);
  }
  float d2 = abs(length(p) - r2) - thickness;
  d = min(d, d2);
  float rad =
      radians(((3.f * 25.f) + 10.f * -iTime) + sin(iTime * 1.2f) * -10.f);
  float x = r2 * cos(rad) + p.data[0];
  float y = r2 * sin(rad) + p.data[1];
  d2 = smallCircle(__ctx, f32x2(x, y), 0.02f);
  d = min(d, d2);
  x = r2 * cos(rad);
  y = r2 * sin(rad);
  rad = radians((2.f * 25.f) + 10.f * -iTime);
  float deg = (30.f + (2.f * 5.f));
  float rad2 = radians(deg * -iTime);
  r2 = 0.055f - (2.f * 0.005f);
  float x2 = (r * cos(rad)) + (r2 * cos(rad2));
  float y2 = (r * sin(rad)) + (r2 * sin(rad2));
  d2 = dLine(__ctx, p, f32x2(-x, -y), f32x2(-x2, -y2), 0.001f);
  d = min(d, d2);
  r2 = R2;
  rad = radians((10.f * -iTime) + sin(iTime * 1.2f) * 10.f);
  x = r2 * cos(rad) + p.data[0];
  y = r2 * sin(rad) + p.data[1];
  d2 = smallCircle(__ctx, f32x2(x, y), 0.02f);
  d = min(d, d2);
  x = r2 * cos(rad);
  y = r2 * sin(rad);
  rad = radians(25.f + 10.f * -iTime);
  deg = 35.f;
  rad2 = radians(deg * iTime);
  r2 = 0.055f - (1.f * 0.005f);
  x2 = (r * cos(rad)) + (r2 * cos(rad2));
  y2 = (r * sin(rad)) + (r2 * sin(rad2));
  d2 = dLine(__ctx, p, f32x2(-x, -y), f32x2(-x2, -y2), 0.001f);
  d = min(d, d2);
  for (float i = 0.f; i < 4.f; i++) {
    float startAngle = i * 25.f;
    float rad = radians(180.f + startAngle + 10.f * -iTime);
    float x = r * cos(rad) + p.data[0];
    float y = r * sin(rad) + p.data[1];
    float dir = (mod(i, 2.f) == 0.f) ? -1.f : 1.f;
    float d2 = ringItem(__ctx, f32x2(x, y), 0.055f - (i * 0.005f),
                        0.02f - (i * 0.002f), (30.f + (i * 5.f)) * dir);
    d = min(d, d2);
  }
  r2 = R2;
  rad = radians((180.f + (3.f * 25.f) + 10.f * -iTime) +
                sin(iTime * 1.3f) * 10.f);
  x = r2 * cos(rad) + p.data[0];
  y = r2 * sin(rad) + p.data[1];
  d2 = smallCircle(__ctx, f32x2(x, y), 0.02f);
  d = min(d, d2);
  x = r2 * cos(rad);
  y = r2 * sin(rad);
  rad = radians(180.f + (3.f * 25.f) + 10.f * -iTime);
  deg = -(30.f + (3.f * 5.f));
  rad2 = radians(deg * -iTime);
  r2 = 0.055f - (3.f * 0.005f);
  x2 = (r * cos(rad)) + (r2 * cos(rad2));
  y2 = (r * sin(rad)) + (r2 * sin(rad2));
  d2 = dLine(__ctx, p, f32x2(-x, -y), f32x2(-x2, -y2), 0.001f);
  d = min(d, d2);
  r2 = R2;
  rad = radians(180.f + (10.f * -iTime) + sin(iTime * 1.2f) * -10.f);
  x = r2 * cos(rad) + p.data[0];
  y = r2 * sin(rad) + p.data[1];
  d2 = smallCircle(__ctx, f32x2(x, y), 0.02f);
  d = min(d, d2);
  x = r2 * cos(rad);
  y = r2 * sin(rad);
  rad = radians(180.f + 10.f * -iTime);
  deg = -30.f;
  rad2 = radians(deg * iTime);
  r2 = 0.055f;
  x2 = (r * cos(rad)) + (r2 * cos(rad2));
  y2 = (r * sin(rad)) + (r2 * sin(rad2));
  d2 = dLine(__ctx, p, f32x2(-x, -y), f32x2(-x2, -y2), 0.001f);
  d = min(d, d2);
  p *= f32x2x2(cos(radians(sin(iTime * 1.5f) * 120.f)),
               -sin(radians(sin(iTime * 1.5f) * 120.f)),
               sin(radians(sin(iTime * 1.5f) * 120.f)),
               cos(radians(sin(iTime * 1.5f) * 120.f)));
  d2 = abs(length(p) - r3) - 0.0001f;
  d2 = max(abs(p.data[0]) - 0.3f, d2);
  d = min(d, d2);
  return d;
}
float circleRuler(Context &__ctx, f32x2 p) {
  p *= f32x2x2(cos(radians(iTime * -25.f)), -sin(radians(iTime * -25.f)),
               sin(radians(iTime * -25.f)), cos(radians(iTime * -25.f)));
  f32x2 prevP = p;
  float r = 0.23f;
  float d = abs(length(p) - r) - 0.0001f;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (5.f * 8.0f),
                          6.28f / ((5.f * 8.0f) * 0.5f)) +
                      (5.f - 1.f) * 6.28f / (5.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.163f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  float d2 = max(abs(p).data[0] - f32x2(0.001f, 0.02f).data[0],
                 abs(p).data[1] - f32x2(0.001f, 0.02f).data[1]);
  d = min(d, d2);
  p = prevP;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (20.f * 8.0f),
                          6.28f / ((20.f * 8.0f) * 0.5f)) +
                      (20.f - 1.f) * 6.28f / (20.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.163f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.0005f, 0.01f).data[0],
           abs(p).data[1] - f32x2(0.0005f, 0.01f).data[1]);
  d = min(d, d2);
  return d;
}
float circleRuler2(Context &__ctx, f32x2 p) {
  p *= f32x2x2(
      cos(radians(45.f + iTime * 20.f)), -sin(radians(45.f + iTime * 20.f)),
      sin(radians(45.f + iTime * 20.f)), cos(radians(45.f + iTime * 20.f)));
  f32x2 prevP = p;
  float r = 0.188f;
  float d = abs(length(p) - r) - 0.0001f;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (5.f * 8.0f),
                          6.28f / ((5.f * 8.0f) * 0.5f)) +
                      (5.f - 1.f) * 6.28f / (5.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.125f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  float d2 = max(abs(p).data[0] - f32x2(0.001f, 0.012f).data[0],
                 abs(p).data[1] - f32x2(0.001f, 0.012f).data[1]);
  d = min(d, d2);
  p = prevP;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (10.f * 8.0f),
                          6.28f / ((10.f * 8.0f) * 0.5f)) +
                      (10.f - 1.f) * 6.28f / (10.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.128f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.001f, 0.006f).data[0],
           abs(p).data[1] - f32x2(0.001f, 0.006f).data[1]);
  d = min(d, d2);
  return d;
}
float circleItem0(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p *= f32x2x2(cos(radians(10.f + sin(iTime * 0.5f) * -150.f)),
               -sin(radians(10.f + sin(iTime * 0.5f) * -150.f)),
               sin(radians(10.f + sin(iTime * 0.5f) * -150.f)),
               cos(radians(10.f + sin(iTime * 0.5f) * -150.f)));
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (2.f * 8.0f),
                          6.28f / ((2.f * 8.0f) * 0.5f)) +
                      (2.f - 1.f) * 6.28f / (2.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.24f);
  p *= f32x2x2(cos(radians(225.f)), -sin(radians(225.f)), sin(radians(225.f)),
               cos(radians(225.f)));
  float d = abs(max(-dot(p, f32x2(cos(-radians(45.f)), sin(-radians(45.f)))),
                    max(dot(p, f32x2(cos(radians(45.f)), sin(radians(45.f)))),
                        max(abs(p).data[0] - f32x2(0.015f, 0.015f).data[0],
                            abs(p).data[1] - f32x2(0.015f, 0.015f).data[1])))) -
            0.0005f;
  return d;
}
float circleItem1(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p *= f32x2x2(cos(radians(-15.f * iTime)), -sin(radians(-15.f * iTime)),
               sin(radians(-15.f * iTime)), cos(radians(-15.f * iTime)));
  float d = abs(length(p) - 0.145f) - 0.007f;
  float a = radians(45.f);
  p.data[0] = abs(p.data[0]);
  d = max(dot(p, f32x2(cos(a), sin(a))), d);
  p = prevP;
  float d2 = abs(length(p) - 0.145f) - 0.0005f;
  d2 = max(-d, d2);
  d = min(abs(d) - 0.0005f, d2);
  return d;
}
float centerNumber(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p *= 2.7f;
  p *= f32x2x2(1.0f, tan(-0.4f), 0.0f, 1.0f);
  float d =
      drawFont(__ctx, p - f32x2(-0.09f, 0.0f), int(mod(iTime * 4.f, 10.f)));
  float d2 =
      drawFont(__ctx, p - f32x2(0.09f, 0.0f), int(mod(iTime * 8.f, 10.f)));
  d = min(d, d2);
  p = prevP;
  d2 = abs(max(abs(p).data[0] - f32x2(0.095f, 0.06f).data[0],
               abs(p).data[1] - f32x2(0.095f, 0.06f).data[1])) -
       0.0005f;
  d2 = max(-(abs(p.data[0]) - 0.085f), d2);
  d2 = max(-(abs(p.data[1]) - 0.05f), d2);
  d = min(d, d2);
  return d;
}
float centerCircleUI(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = ringCircle(__ctx, p);
  float d2 = circleRuler(__ctx, p);
  d = min(d, d2);
  d2 = circleRuler2(__ctx, p);
  d = min(d, d2);
  d2 = circleItem0(__ctx, p);
  d = min(d, d2);
  d2 = circleItem1(__ctx, p);
  d = min(d, d2);
  d2 = centerNumber(__ctx, p);
  d = min(d, d2);
  return d;
}
float sideUIItem0Base(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = max(abs(p).data[0] - f32x2(0.17f, 0.025f).data[0],
                abs(p).data[1] - f32x2(0.17f, 0.025f).data[1]);
  p.data[0] -= 0.01f;
  p.data[1] -= 0.02f;
  float d2 = max(abs(p).data[0] - f32x2(0.08f, 0.02f).data[0],
                 abs(p).data[1] - f32x2(0.08f, 0.02f).data[1]);
  float a = radians(-60.f);
  p.data[0] = abs(p.data[0]) - 0.07f;
  d2 = max((dot(p, f32x2(cos(a), sin(a)))), d2);
  d = max(-d2, d);
  p = prevP;
  a = radians(240.f);
  p.data[0] += 0.12f;
  d = max((dot(p, f32x2(cos(a), sin(a)))), d);
  return d;
}
float sideUIItem0(Context &__ctx, f32x2 p) {
  p *= 1.1f;
  p.data[0] *= -1.f;
  f32x2 prevP = p;
  p.data[0] += 0.08f;
  p.data[1] -= 0.08f;
  float d = sideUIItem0Base(__ctx, p);
  p = prevP;
  p.data[0] *= -1.f;
  p.data[0] += 0.08f;
  p.data[1] += 0.08f;
  p *= f32x2x2(cos(radians(-90.f)), -sin(radians(-90.f)), sin(radians(-90.f)),
               cos(radians(-90.f)));
  float d2 = sideUIItem0Base(__ctx, p);
  d = min(d, d2);
  p = prevP;
  float a = radians(45.f);
  p.data[0] -= 0.14f;
  d = max(dot(p, f32x2(cos(a), sin(a))), d);
  p = prevP;
  p -= f32x2(0.05f, 0.05f);
  p *= f32x2x2(cos(radians(-45.f)), -sin(radians(-45.f)), sin(radians(-45.f)),
               cos(radians(-45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.02f, 0.04f).data[0],
           abs(p).data[1] - f32x2(0.02f, 0.04f).data[1]);
  d = abs(min(d, d2)) - 0.0005f;
  p = prevP;
  p -= f32x2(0.052f, 0.052f);
  p *= f32x2x2(cos(radians(-45.f)), -sin(radians(-45.f)), sin(radians(-45.f)),
               cos(radians(-45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.0005f, 0.04f).data[0],
           abs(p).data[1] - f32x2(0.0005f, 0.04f).data[1]);
  d = min(d, d2);
  p = prevP;
  d2 = randomDotLine(__ctx, p - f32x2(-0.07f, 0.055f));
  d = min(d, d2);
  d2 = max(
      abs(p - f32x2(-0.065f, 0.037f)).data[0] - f32x2(0.075f, 0.0001f).data[0],
      abs(p - f32x2(-0.065f, 0.037f)).data[1] - f32x2(0.075f, 0.0001f).data[1]);
  d = min(d, d2);
  return d;
}
float sideUIItem1(Context &__ctx, f32x2 p) {
  p *= 1.1f;
  f32x2 prevP = p;
  float d = max(abs(p).data[0] - f32x2(0.003f, 0.36f).data[0],
                abs(p).data[1] - f32x2(0.003f, 0.36f).data[1]);
  p.data[0] += 0.027f;
  p.data[1] = abs(p.data[1]) - 0.36f;
  float d2 = max(abs(p).data[0] - f32x2(0.03f, 0.003f).data[0],
                 abs(p).data[1] - f32x2(0.03f, 0.003f).data[1]);
  d = min(d, d2);
  float speed = -0.1f;
  p = prevP;
  p.data[1] += iTime * speed;
  p.data[0] += 0.015f;
  p.data[1] = mod(p.data[1], 0.02f) - 0.01f;
  d2 = max(abs(p).data[0] - f32x2(0.01f, 0.0005f).data[0],
           abs(p).data[1] - f32x2(0.01f, 0.0005f).data[1]);
  p = prevP;
  d2 = max(abs(p.data[1]) - 0.36f, d2);
  d = min(d, d2);
  p = prevP;
  p.data[1] += iTime * speed;
  p.data[0] += 0.025f;
  p.data[1] = mod(p.data[1], 0.06f) - 0.03f;
  d2 = max(abs(p).data[0] - f32x2(0.015f, 0.001f).data[0],
           abs(p).data[1] - f32x2(0.015f, 0.001f).data[1]);
  p = prevP;
  d2 = max(abs(p.data[1]) - 0.36f, d2);
  d = min(d, d2);
  p = prevP;
  p.data[0] += 0.06f;
  p.data[1] += sin(iTime * 0.5f) * 0.3f;
  p *= f32x2x2(cos(radians(90.f)), -sin(radians(90.f)), sin(radians(90.f)),
               cos(radians(90.f)));
  d2 = abs(max(-dot(p, f32x2(cos(-radians(45.f)), sin(-radians(45.f)))),
               max(dot(p, f32x2(cos(radians(45.f)), sin(radians(45.f)))),
                   max(abs(p).data[0] - f32x2(0.015f).data[0],
                       abs(p).data[1] - f32x2(0.015f).data[1])))) -
       0.0005f;
  d = min(d, d2);
  return d;
}
float graphItem(Context &__ctx, f32x2 p, float speed, float start, float h) {
  f32x2 prevP;
  float endTime = 5.f;
  float dist = h - start;
  float t = mod(iTime * speed, endTime);
  float animVal = endTime * 0.5f;
  float val = t / animVal;
  if (t < endTime * 0.5f) {
    val = (t / animVal) * dist;
  } else {
    val = (1.f - ((t - animVal) / animVal)) * dist;
  }
  float w = 0.013f;
  float h2 = start + val;
  float d = max(abs(p - f32x2(0.f, val)).data[0] - f32x2(w, h2).data[0],
                abs(p - f32x2(0.f, val)).data[1] - f32x2(w, h2).data[1]);
  float d2 = abs(max(abs(p - f32x2(0.f, dist)).data[0] -
                         f32x2(0.022f, h + start + 0.005f).data[0],
                     abs(p - f32x2(0.f, dist)).data[1] -
                         f32x2(0.022f, h + start + 0.005f).data[1])) -
             0.0001f;
  d2 = max(-(abs(p.data[0]) - (w - 0.002f)), d2);
  d2 = max(-(abs(p.data[1] - dist) - h), d2);
  d = min(d, d2);
  return d;
}
float graphUI(Context &__ctx, f32x2 p) {
  float d = graphItem(__ctx, p, 2.5f, 0.01f, 0.1f);
  float d2 = graphItem(__ctx, p - f32x2(0.055f, 0.f), 2.f, 0.01f, 0.1f);
  d = min(d, d2);
  d2 = graphItem(__ctx, p - f32x2(-0.055f, 0.f), 2.7f, 0.01f, 0.1f);
  d = min(d, d2);
  return d;
}
float meterUI(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = abs(length(p) - 0.06f) - 0.0001f;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (4.f * 8.0f),
                          6.28f / ((4.f * 8.0f) * 0.5f)) +
                      (4.f - 1.f) * 6.28f / (4.f * 8.0f) + f32x2(0, 11));
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  float d2 = max(abs(p).data[0] - f32x2(0.005f, 0.2f).data[0],
                 abs(p).data[1] - f32x2(0.005f, 0.2f).data[1]);
  d = max(-d2, d);
  p = prevP;
  p *= f32x2x2(cos(radians(sin(iTime * 0.5f) * 190.f)),
               -sin(radians(sin(iTime * 0.5f) * 190.f)),
               sin(radians(sin(iTime * 0.5f) * 190.f)),
               cos(radians(sin(iTime * 0.5f) * 190.f)));
  p.data[0] = abs(p.data[0]) - 0.085f;
  p *= f32x2x2(cos(radians(90.f)), -sin(radians(90.f)), sin(radians(90.f)),
               cos(radians(90.f)));
  d2 = abs(max(-dot(p, f32x2(cos(-radians(45.f)), sin(-radians(45.f)))),
               max(dot(p, f32x2(cos(radians(45.f)), sin(radians(45.f)))),
                   max(abs(p).data[0] - f32x2(0.015f, 0.015f).data[0],
                       abs(p).data[1] - f32x2(0.015f, 0.015f).data[1])))) -
       0.0005f;
  d = min(d, d2);
  p = prevP;
  d2 = max(abs(p).data[0] - f32x2(0.0002f, 0.04f).data[0],
           abs(p).data[1] - f32x2(0.0002f, 0.04f).data[1]);
  d = min(d, d2);
  d2 = max(abs(p).data[0] - f32x2(0.04f, 0.0002f).data[0],
           abs(p).data[1] - f32x2(0.04f, 0.0002f).data[1]);
  d = min(d, d2);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  d = max(-max(abs(p).data[0] - f32x2(0.02f).data[0],
               abs(p).data[1] - f32x2(0.02f).data[1]),
          d);
  d2 = abs(max(abs(p).data[0] - f32x2(0.01f).data[0],
               abs(p).data[1] - f32x2(0.01f).data[1])) -
       0.00001f;
  d = min(d, d2);
  p = prevP;
  p *= f32x2x2(cos(radians(iTime * -30.f)), -sin(radians(iTime * -30.f)),
               sin(radians(iTime * -30.f)), cos(radians(iTime * -30.f)));
  d2 = max(abs(p - f32x2(0, 0.025f)).data[0] - f32x2(0.0002f, 0.025f).data[0],
           abs(p - f32x2(0, 0.025f)).data[1] - f32x2(0.0002f, 0.025f).data[1]);
  d = min(d, d2);
  return d;
}
float sideUIItem2(Context &__ctx, f32x2 p) {
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  f32x2 prevP = p;
  float d = abs(max(abs(p).data[0] - f32x2(0.06f).data[0],
                    abs(p).data[1] - f32x2(0.06f).data[1])) -
            0.0001f;
  float d2 = abs(max(abs(p).data[0] - f32x2(0.06f).data[0],
                     abs(p).data[1] - f32x2(0.06f).data[1])) -
             0.002f;
  p *= f32x2x2(cos(radians(20.f * iTime)), -sin(radians(20.f * iTime)),
               sin(radians(20.f * iTime)), cos(radians(20.f * iTime)));
  float mask = min(abs(p.data[0]) - 0.03f, abs(p.data[1]) - 0.03f);
  d2 = max(-mask, d2);
  d = min(d, d2);
  p = prevP;
  d2 = abs(max(abs(p).data[0] - f32x2(0.04f).data[0],
               abs(p).data[1] - f32x2(0.04f).data[1])) -
       0.0001f;
  d = min(d, d2);
  d2 = abs(max(abs(p).data[0] - f32x2(0.04f).data[0],
               abs(p).data[1] - f32x2(0.04f).data[1])) -
       0.002f;
  p *= f32x2x2(cos(radians(-25.f * iTime)), -sin(radians(-25.f * iTime)),
               sin(radians(-25.f * iTime)), cos(radians(-25.f * iTime)));
  mask = min(abs(p.data[0]) - 0.015f, abs(p.data[1]) - 0.015f);
  d2 = max(-mask, d2);
  d = min(d, d2);
  p = prevP;
  p *= 1.f + sin(iTime * 5.f) * 0.1f;
  d2 = abs(max(abs(p).data[0] - f32x2(0.015f).data[0],
               abs(p).data[1] - f32x2(0.015f).data[1])) -
       0.0001f;
  mask = min(abs(p.data[0]) - 0.005f, abs(p.data[1]) - 0.005f);
  d2 = max(-mask, d2);
  d = min(d, d2);
  return d;
}
float sliderItem(Context &__ctx, f32x2 p, float speed) {
  f32x2 prevP = p;
  float d = abs(max(abs(p).data[0] - f32x2(0.02f, 0.1f).data[0],
                    abs(p).data[1] - f32x2(0.02f, 0.1f).data[1])) -
            0.0001f;
  d = max(-(abs(p.data[1]) - 0.09f), d);
  float d2 = max(abs(p).data[0] - f32x2(0.0001f, 0.09f).data[0],
                 abs(p).data[1] - f32x2(0.0001f, 0.09f).data[1]);
  d = min(d, d2);
  p.data[1] = mod(p.data[1], 0.02f) - 0.01f;
  d2 = max(abs(p).data[0] - f32x2(0.005f, 0.001f).data[0],
           abs(p).data[1] - f32x2(0.005f, 0.001f).data[1]);
  p = prevP;
  d2 = max((abs(p.data[1]) - 0.08f), d2);
  d = min(d, d2);
  p.data[1] += sin(iTime * speed) * 0.08f;
  d2 = max(abs(p).data[0] - f32x2(0.018f, 0.005f).data[0],
           abs(p).data[1] - f32x2(0.018f, 0.005f).data[1]);
  d = min(d, d2);
  p.data[0] = abs(p.data[0]) - 0.025f;
  p *= f32x2x2(cos(radians(-90.f)), -sin(radians(-90.f)), sin(radians(-90.f)),
               cos(radians(-90.f)));
  d2 = abs(max(-dot(p, f32x2(cos(-radians(45.f)), sin(-radians(45.f)))),
               max(dot(p, f32x2(cos(radians(45.f)), sin(radians(45.f)))),
                   max(abs(p).data[0] - f32x2(0.013f, 0.013f).data[0],
                       abs(p).data[1] - f32x2(0.013f, 0.013f).data[1])))) -
       0.0005f;
  d = min(d, d2);
  return d;
}
float sideUIItem3(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = sliderItem(__ctx, p - f32x2(-0.045f, 0.f), 1.f);
  float d2 = sliderItem(__ctx, p - f32x2(0.045f, 0.f), 1.5f);
  d = min(d, d2);
  return d;
}
float smallCircleItem(Context &__ctx, f32x2 p, float speed) {
  f32x2 prevP = p;
  p *= f32x2x2(
      cos(radians(20.f * iTime * speed)), -sin(radians(20.f * iTime * speed)),
      sin(radians(20.f * iTime * speed)), cos(radians(20.f * iTime * speed)));
  float d = abs(length(p) - 0.029f) - 0.01f;
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (2.f * 8.0f),
                          6.28f / ((2.f * 8.0f) * 0.5f)) +
                      (2.f - 1.f) * 6.28f / (2.f * 8.0f) + f32x2(0, 11));
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  float d2 = max(abs(p).data[0] - f32x2(0.005f, 0.2f).data[0],
                 abs(p).data[1] - f32x2(0.005f, 0.2f).data[1]);
  d = max(-d2, d);
  d2 = length(p) - 0.005f;
  d = min(d, d2);
  return d;
}
float sideUIItem4(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = smallCircleItem(__ctx, p - f32x2(-0.045f, 0.f), 1.f);
  float d2 = smallCircleItem(__ctx, p - f32x2(0.045f, 0.f), -2.f);
  d = min(d, d2);
  return d;
}
float sideUIItem5(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p *= f32x2x2(cos(radians(-25.f * iTime)), -sin(radians(-25.f * iTime)),
               sin(radians(-25.f * iTime)), cos(radians(-25.f * iTime)));
  p = length(p) * cos(mod(atan(p.data[1], p.data[0]) + 6.28f / (5.f * 8.0f),
                          6.28f / ((5.f * 8.0f) * 0.5f)) +
                      (5.f - 1.f) * 6.28f / (5.f * 8.0f) + f32x2(0, 11));
  p -= f32x2(0.04f);
  p *= f32x2x2(cos(radians(45.f)), -sin(radians(45.f)), sin(radians(45.f)),
               cos(radians(45.f)));
  float d = max(abs(p).data[0] - f32x2(0.001f, 0.01f).data[0],
                abs(p).data[1] - f32x2(0.001f, 0.01f).data[1]);
  float speed = 2.f;
  p = prevP;
  p *= f32x2x2(
      cos(radians(25.f * iTime * speed)), -sin(radians(25.f * iTime * speed)),
      sin(radians(25.f * iTime * speed)), cos(radians(25.f * iTime * speed)));
  float d2 = abs(length(p) - 0.08f) - 0.005f;
  float a = radians(45.f);
  p.data[0] = abs(p.data[0]);
  d2 = max(dot(p, f32x2(cos(a), sin(a))), d2);
  d = min(d, d2);
  p = prevP;
  p *= f32x2x2(
      cos(radians(25.f * iTime * speed)), -sin(radians(25.f * iTime * speed)),
      sin(radians(25.f * iTime * speed)), cos(radians(25.f * iTime * speed)));
  d2 = abs(length(p) - 0.08f) - 0.005f;
  a = radians(-45.f);
  p.data[0] = abs(p.data[0]);
  d2 = max(dot(p, f32x2(cos(a), sin(a))), d2);
  d = min(d, d2);
  p = prevP;
  p *= f32x2x2(cos(radians(sin(iTime * 2.f) * 45.f)),
               -sin(radians(sin(iTime * 2.f) * 45.f)),
               sin(radians(sin(iTime * 2.f) * 45.f)),
               cos(radians(sin(iTime * 2.f) * 45.f)));
  d2 = max(abs(p).data[0] - f32x2(0.03f, 0.001f).data[0],
           abs(p).data[1] - f32x2(0.03f, 0.001f).data[1]);
  d = min(d, d2);
  d2 = max(abs(p).data[0] - f32x2(0.001f, 0.03f).data[0],
           abs(p).data[1] - f32x2(0.001f, 0.03f).data[1]);
  d = min(d, d2);
  d = max(-max(abs(p).data[0] - f32x2(0.015f).data[0],
               abs(p).data[1] - f32x2(0.015f).data[1]),
          d);
  d2 = length(p) - 0.005f;
  d = min(d, d2);
  return d;
}
float triangle2(Context &__ctx, f32x2 p, f32x2 s, float thick) {
  float d = max(abs(p).data[0] - s.data[0], abs(p).data[1] - s.data[1]);
  float a = radians(225.f);
  d = abs(max(dot(p, f32x2(cos(a), sin(a))), d)) - thick;
  return d;
}
float sideUIItem6(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  float d = triangle2(__ctx, p, f32x2(0.08f), 0.0005f);
  float d2 = triangle2(__ctx, p, f32x2(0.08f), 0.003f);
  p *= f32x2x2(cos(radians(-25.f * iTime)), -sin(radians(-25.f * iTime)),
               sin(radians(-25.f * iTime)), cos(radians(-25.f * iTime)));
  d2 = max(-min(abs(p.data[0]) - 0.03f, abs(p.data[1]) - 0.03f), d2);
  d = min(d, d2);
  p = prevP;
  d2 = triangle2(__ctx, p - f32x2(0.018f), f32x2(0.035f), 0.0005f);
  d = min(d, d2);
  d2 = triangle2(__ctx, p - f32x2(0.018f), f32x2(0.035f), 0.003f);
  p -= f32x2(0.018f);
  p *= f32x2x2(cos(radians(25.f * iTime)), -sin(radians(25.f * iTime)),
               sin(radians(25.f * iTime)), cos(radians(25.f * iTime)));
  d2 = max(-min(abs(p.data[0]) - 0.015f, abs(p.data[1]) - 0.015f), d2);
  d = min(d, d2);
  return d;
}
float plus(Context &__ctx, f32x2 p, float speed) {
  p *= f32x2x2(
      cos(radians(10.f * iTime * speed)), -sin(radians(10.f * iTime * speed)),
      sin(radians(10.f * iTime * speed)), cos(radians(10.f * iTime * speed)));
  float d = max(abs(p).data[0] - f32x2(0.0001f, 0.01f).data[0],
                abs(p).data[1] - f32x2(0.0001f, 0.01f).data[1]);
  float d2 = max(abs(p).data[0] - f32x2(0.01f, 0.0001f).data[0],
                 abs(p).data[1] - f32x2(0.01f, 0.0001f).data[1]);
  d = min(d, d2);
  return d;
}
float sideAreaUI(Context &__ctx, f32x2 p) {
  f32x2 prevP = p;
  p.data[0] = abs(p.data[0]) - 0.57f;
  p.data[1] = abs(p.data[1]) - 0.35f;
  float d = sideUIItem0(__ctx, p);
  p = prevP;
  p.data[0] = abs(p.data[0]) - 0.79f;
  float d2 = sideUIItem1(__ctx, p);
  d = min(d, d2);
  p = prevP;
  d2 = graphUI(__ctx, p - f32x2(0.625f, 0.125f));
  d = min(d, d2);
  d2 = meterUI(__ctx, p - f32x2(-0.62f, 0.23f));
  d = min(d, d2);
  d2 = sideUIItem2(__ctx, p - f32x2(-0.62f, 0.f));
  d = min(d, d2);
  d2 = sideUIItem3(__ctx, p - f32x2(-0.62f, -0.25f));
  d = min(d, d2);
  d2 = sideUIItem4(__ctx, p - f32x2(0.62f, 0.f));
  d = min(d, d2);
  d2 = sideUIItem5(__ctx, p - f32x2(0.62f, -0.22f));
  d = min(d, d2);
  p.data[0] = abs(p.data[0]) - 0.37f;
  p.data[1] = abs(p.data[1]) - 0.36f;
  d2 = sideUIItem6(__ctx, p);
  d = min(d, d2);
  p = prevP;
  p -= f32x2(-0.62f, 0.f);
  p.data[0] = abs(p.data[0]) - 0.08f;
  p.data[1] = abs(p.data[1]) - 0.1f;
  d2 = plus(__ctx, p, 5.f);
  d = min(d, d2);
  p = prevP;
  p -= f32x2(0.62f, -0.22f);
  p.data[0] = abs(p.data[0]) - 0.07f;
  p.data[1] = abs(p.data[1]) - 0.11f;
  d2 = plus(__ctx, p, -8.f);
  d = min(d, d2);
  return d;
}
float bg(Context &__ctx, f32x2 p) {
  p = mod(p, 0.04f) - 0.02f;
  float d = abs(p.data[0]) - 0.0001f;
  float d2 = abs(p.data[1]) - 0.0001f;
  d = min(d, d2);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0.f);
  float d = centerCircleUI(__ctx, p);
  float d2 = sideAreaUI(__ctx, p);
  d = min(d, d2);
  float bgd = bg(__ctx, p);
  col = mix(col, f32x3(0.03f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, bgd));
  col = mix(col, f32x3(1.f),
            smoothstep(1.0f / min(iResolution.data[1], iResolution.data[0]),
                       0.0f, d));
  fragColor = f32x4(sqrt(col), 1.0f);
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
