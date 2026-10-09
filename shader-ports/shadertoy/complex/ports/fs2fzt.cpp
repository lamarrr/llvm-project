// Port of https://www.shadertoy.com/view/fs2fzt; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fs2fzt.json.
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
float cc(Context &__ctx, float a, float b) {
  float f = tanh(a * cos(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
float cs(Context &__ctx, float a, float b) {
  float f = tanh(a * sin(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float mlength(Context &__ctx, f32x2 uv) {
  return max(abs(uv.data[0]), abs(uv.data[1]));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
float smin(Context &__ctx, float a, float b) {
  float k = 0.12f;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
float sdEllipse(Context &__ctx, f32x2 p, f32x2 ab) {
  p = abs(p);
  if (p.data[0] > p.data[1]) {
    p = p.swizzle<1, 0>();
    ab = ab.swizzle<1, 0>();
  }
  float l = ab.data[1] * ab.data[1] - ab.data[0] * ab.data[0];
  float m = ab.data[0] * p.data[0] / l;
  float m2 = m * m;
  float n = ab.data[1] * p.data[1] / l;
  float n2 = n * n;
  float c = (m2 + n2 - 1.0f) / 3.0f;
  float c3 = c * c * c;
  float q = c3 + m2 * n2 * 2.0f;
  float d = c3 + m2 * n2;
  float g = m + m * n2;
  float co;
  if (d < 0.0f) {
    float h = acos(q / c3) / 3.0f;
    float s = cos(h);
    float t = sin(h) * sqrt(3.0f);
    float rx = sqrt(-c * (s + t + 2.0f) + m2);
    float ry = sqrt(-c * (s - t + 2.0f) + m2);
    co = (ry + sign(l) * rx + abs(g) / (rx * ry) - m) / 2.0f;
  } else {
    float h = 2.0f * m * n * sqrt(d);
    float s = sign(q + h) * pow(abs(q + h), 1.0f / 3.0f);
    float u = sign(q - h) * pow(abs(q - h), 1.0f / 3.0f);
    float rx = -s - u - c * 4.0f + 2.0f * m2;
    float ry = (s - u) * sqrt(3.0f);
    float rm = sqrt(rx * rx + ry * ry);
    co = (ry / sqrt(rm - rx) + 2.0f * g / rm - m) / 2.0f;
  }
  f32x2 r = ab * f32x2(co, sqrt(1.0f - co * co));
  return length(r - p) * sign(p.data[1] - r.data[1]);
}
float invs(Context &__ctx, float y) {
  return 0.5f - sin(asin(1.0f - 2.0f * y) / 3.0f);
}
float isBetween(Context &__ctx, float a, float b, float c) {
  return smoothstep(-0.5f, 0.f,
                    -mod(c - a, 2.f * 3.14159f) +
                        mod(b - a - 0.1f * 3.14159f, 2.f * 3.14159f));
}
float ellipse(Context &__ctx, f32x2 uv, f32x2 p, f32x2 q, float i) {
  float quadTest =
      0.5f * (sign(q.data[0] - p.data[0]) * sign(q.data[1] - p.data[1]) + 1.f);
  i = 1.f - quadTest;
  f32x2 c =
      (i == 1.f) ? f32x2(p.data[0], q.data[1]) : f32x2(q.data[0], p.data[1]);
  float x = abs(q.data[0] - p.data[0]), y = abs(q.data[1] - p.data[1]);
  float d = sdEllipse(__ctx, uv - c, f32x2(x, y));
  float k = 1.f / iResolution.data[1];
  float s = smoothstep(-k, k, -abs(d) + 0.006f);
  s = exp(-100.f * abs(d));
  float a1 = atan(p.data[0] - c.data[0], p.data[1] - c.data[1]);
  float a2 = atan(q.data[0] - c.data[0], q.data[1] - c.data[1]);
  float b = atan(uv.data[0] - c.data[0], uv.data[1] - c.data[1]);
  float as = isBetween(__ctx, a1, a2, b);
  return s;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  uv *= 1.1f + 0.15f * cos(uv.data[1] - 0.6f * iTime);
  uv.data[1] += 0.02f * cos(iTime);
  f32x2 p = 0.1f * f32x2(cos(0.913f * iTime), sin(iTime));
  f32x2 q = 0.2f * f32x2(cos(0.81f * iTime), sin(0.73f * iTime));
  float t = 10.f * iTime + 8.f * h21(__ctx, uv) +
            15.f * exp(-0.01f * length(uv)) * (650.f + iTime);
  int f = int(floor(t));
  float d = 10.f;
  float s = 0.f;
  f32x2 pp = f32x2(0.f);
  f32x3 e = f32x3(1);
  f32x3 col = f32x3(0);
  float n = 20.f;
  for (float i = 0.f; i <= n; i++) {
    float f2 = 0.0001f * float(f);
    float f3 = 0.0001f * float(f + 1);
    f32x2 qp = pp;
    pp = f32x2(h21(__ctx, f32x2(f2)), h21(__ctx, f32x2(0.01f + f2)));
    pp = pow(4.f * pp * (1.f - pp), f32x2(4));
    f32x2 pp2 = f32x2(h21(__ctx, f32x2(f3)), h21(__ctx, f32x2(0.01f + f3)));
    float fr = fract(t);
    fr = smoothstep(0.f, 1.f, fr);
    pp = mix(pp, pp2, fr);
    pp = 0.3f * (pp - 0.5f);
    f++;
    float s2;
    if (i > 0.f)
      s2 = ellipse(__ctx, uv, pp, qp, 0.f);
    s = clamp(s + s2, 0.f, 1.f);
    f32x3 col2 = pal(__ctx, i / n, e, e, e, (i / n) * f32x3(0, 1, 2) / 3.f);
    col = mix(col, col2, s2);
  }
  col += 0.03f;
  col += 0.35f * exp(-3.f * length(uv));
  fragColor = f32x4(col, 1.0f);
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
