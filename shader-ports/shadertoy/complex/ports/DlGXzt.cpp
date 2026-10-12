// Port of https://www.shadertoy.com/view/DlGXzt; author: iterati.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlGXzt.json.
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
f32x3 palette(Context &__ctx, float t) {
  f32x3 a = f32x3(0.5f);
  f32x3 b = f32x3(0.5f);
  f32x3 c = f32x3(1.f);
  f32x3 d = f32x3(0.0f, 0.333f, 0.667f);
  return a + b * cos(6.28318f * (c * t + d));
}
float sinT(Context &__ctx, float t, float l) {
  return (sin(((t * 2.f * 3.1415926538f) / l) + (0.5f * 3.1415926538f)) *
          0.5f) +
         0.5f;
}
float sawT(Context &__ctx, float t, float l) {
  return abs((mod(t / l, 1.f) - 0.5f) * 2.0f);
}
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float ndot(Context &__ctx, f32x2 a, f32x2 b) {
  return a.data[0] * b.data[0] - a.data[1] * b.data[1];
}
float sdRhombus(Context &__ctx, f32x2 p, f32x2 b) {
  p = abs(p);
  float h = clamp(ndot(__ctx, b - 2.0f * p, b) / dot(b, b), -1.0f, 1.0f);
  float d = length(p - 0.5f * b * f32x2(1.0f - h, 1.0f + h));
  return d * sign(p.data[0] * b.data[1] + p.data[1] * b.data[0] -
                  b.data[0] * b.data[1]);
}
float sdEquilateralTriangle(Context &__ctx, f32x2 p, float r) {
  const float k = sqrt(3.0f);
  p.data[0] = abs(p.data[0]) - r;
  p.data[1] = p.data[1] + r / k;
  if (p.data[0] + k * p.data[1] > 0.0f)
    p = f32x2(p.data[0] - k * p.data[1], -k * p.data[0] - p.data[1]) / 2.0f;
  p.data[0] -= clamp(p.data[0], -2.0f * r, 0.0f);
  return -length(p) * sign(p.data[1]);
}
float sdPentagon(Context &__ctx, f32x2 p, float r) {
  const f32x3 k = f32x3(0.809016994f, 0.587785252f, 0.726542528f);
  p.data[0] = abs(p.data[0]);
  p -= 2.0f * min(dot(f32x2(-k.data[0], k.data[1]), p), 0.0f) *
       f32x2(-k.data[0], k.data[1]);
  p -= 2.0f * min(dot(f32x2(k.data[0], k.data[1]), p), 0.0f) *
       f32x2(k.data[0], k.data[1]);
  p -= f32x2(clamp(p.data[0], -r * k.data[2], r * k.data[2]), r);
  return length(p) * sign(p.data[1]);
}
float sdHexagon(Context &__ctx, f32x2 p, float r) {
  const f32x3 k = f32x3(-0.866025404f, 0.5f, 0.577350269f);
  p = abs(p);
  p -= 2.0f * min(dot(k.swizzle<0, 1>(), p), 0.0f) * k.swizzle<0, 1>();
  p -= f32x2(clamp(p.data[0], -k.data[2] * r, k.data[2] * r), r);
  return length(p) * sign(p.data[1]);
}
float sdOctogon(Context &__ctx, f32x2 p, float r) {
  const f32x3 k = f32x3(-0.9238795325f, 0.3826834323f, 0.4142135623f);
  p = abs(p);
  p -= 2.0f * min(dot(f32x2(k.data[0], k.data[1]), p), 0.0f) *
       f32x2(k.data[0], k.data[1]);
  p -= 2.0f * min(dot(f32x2(-k.data[0], k.data[1]), p), 0.0f) *
       f32x2(-k.data[0], k.data[1]);
  p -= f32x2(clamp(p.data[0], -k.data[2] * r, k.data[2] * r), r);
  return length(p) * sign(p.data[1]);
}
float sdHexagram(Context &__ctx, f32x2 p, float r) {
  const f32x4 k = f32x4(-0.5f, 0.8660254038f, 0.5773502692f, 1.7320508076f);
  p = abs(p);
  p -= 2.0f * min(dot(k.swizzle<0, 1>(), p), 0.0f) * k.swizzle<0, 1>();
  p -= 2.0f * min(dot(k.swizzle<1, 0>(), p), 0.0f) * k.swizzle<1, 0>();
  p -= f32x2(clamp(p.data[0], r * k.data[2], r * k.data[3]), r);
  return length(p) * sign(p.data[1]);
}
float sdStar(Context &__ctx, f32x2 p, float r, int n, float m) {
  float an = 3.1415926538f / float(n);
  float en = 3.1415926538f / m;
  f32x2 acs = f32x2(cos(an), sin(an));
  f32x2 ecs = f32x2(cos(en), sin(en));
  float bn = mod(atan(p.data[0], p.data[1]), 2.0f * an) - an;
  p = length(p) * f32x2(cos(bn), abs(sin(bn)));
  p -= r * acs;
  p += ecs * clamp(-dot(p, ecs), 0.0f, r * acs.data[1] / ecs.data[1]);
  return length(p) * sign(p.data[0]);
}
float sdCross(Context &__ctx, f32x2 p, f32x2 b, float r) {
  p = abs(p);
  p = (p.data[1] > p.data[0]) ? p.swizzle<1, 0>() : p.swizzle<0, 1>();
  f32x2 q = p - b;
  float k = max(q.data[1], q.data[0]);
  f32x2 w = (k > 0.0f) ? q : f32x2(b.data[1] - p.data[0], -k);
  return sign(k) * length(max(w, 0.0f)) + r;
}
float sdBlobbyCross(Context &__ctx, f32x2 pos, float he) {
  pos = abs(pos);
  pos =
      f32x2(abs(pos.data[0] - pos.data[1]), 1.0f - pos.data[0] - pos.data[1]) /
      sqrt(2.0f);
  float p = (he - pos.data[1] - 0.25f / he) / (6.0f * he);
  float q = pos.data[0] / (he * he * 16.0f);
  float h = q * q - p * p * p;
  float x;
  if (h > 0.0f) {
    float r = sqrt(h);
    x = pow(q + r, 1.0f / 3.0f) - pow(abs(q - r), 1.0f / 3.0f) * sign(r - q);
  } else {
    float r = sqrt(p);
    x = 2.0f * r * cos(acos(q / (p * r)) / 3.0f);
  }
  x = min(x, sqrt(2.0f) / 2.0f);
  f32x2 z = f32x2(x, he * (1.0f - 2.0f * x * x)) - pos;
  return length(z) * sign(z.data[1]);
}
float sdRoundedX(Context &__ctx, f32x2 p, float w, float r) {
  p = abs(p);
  return length(p - min(p.data[0] + p.data[1], w) * 0.5f) - r;
}
float sdMoon(Context &__ctx, f32x2 p, float d, float ra, float rb) {
  p.data[1] = abs(p.data[1]);
  float a = (ra * ra - rb * rb + d * d) / (2.0f * d);
  float b = sqrt(max(ra * ra - a * a, 0.0f));
  if (d * (p.data[0] * b - p.data[1] * a) > d * d * max(b - p.data[1], 0.0f))
    return length(p - f32x2(a, b));
  return max((length(p) - ra), -(length(p - f32x2(d, 0)) - rb));
}
f32x2 rotate(Context &__ctx, f32x2 v, float a) {
  float s = sin(a);
  float c = cos(a);
  f32x2x2 m = f32x2x2(c, -s, s, c);
  return m * v;
}
int _N = 2;
float morphStar(Context &__ctx, f32x2 p, float r, float f) {
  float rtn = 0.f;
  float x = mod(f, 1.f) * 2.f * 3.1415926538f;
  float y = sinT(__ctx, f, 1.f);
  int n = (int(f) + 1) * 4;
  return (sdCircle(__ctx, p, r) * y) +
         (sdStar(__ctx, p, r, n, float(n)) * (1.f - y));
}
float sheen(Context &__ctx, float d, float x, float g) {
  return 1.f - smoothstep(0.0f, x, abs(d));
}
void mainImage0(Context &__ctx, f32x4 &O, f32x2 U);
void mainImage(Context &__ctx, f32x4 &O, f32x2 U) {
  f32x4 o;
  O = f32x4(0);
  for (int k = 0; k < _N * _N; k++) {
    mainImage0(__ctx, o,
               U + f32x2(k % _N - _N / 2, k / _N - _N / 2) / float(_N));
    O += o;
  }
  O /= float(_N * _N);
  O = pow(O, f32x4(1.f / 2.2f));
}
void mainImage0(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.0f - iResolution.swizzle<0, 1>()) / iResolution.data[0];
  f32x2 uv0 = uv;
  f32x3 finalColor = f32x3(0.0f);
  f32x3 col = f32x3(0.f);
  float d = 0.f;
  float f = mod(iTime / 7.f, 4.f);
  float r = sinT(__ctx, iTime, 11.f) * 0.2f + 0.4f;
  float x = sinT(__ctx, iTime, 30.f) * 1.f + 3.5f;
  float y = sinT(__ctx, iTime, 10.f) * 45.f;
  float z = mod(iTime / 45.f, 2.f * 3.1415926538f);
  uv = rotate(__ctx, uv, z);
  d = sheen(__ctx,
            morphStar(__ctx, uv, r, f) *
                exp(-length(uv0) * sinT(__ctx, iTime, 5.f) * 2.f),
            0.0025f, 1.1f);
  col = palette(__ctx, (length(uv0) + sin(iTime)) / 2.0f);
  finalColor += col * d;
  for (float i = 0.0f; i < 4.0f; i++) {
    float rot =
        (int(i) % 2 == 0) ? 3.1415926538f * 0.25f : -3.1415926538f * 0.25f;
    uv = fract(rotate(__ctx, uv, rot) * x * (i + 1.f) * 0.25f) - 0.5f;
    d = sheen(__ctx,
              morphStar(__ctx, uv, r, f) *
                  exp(-length(uv0) * sinT(__ctx, iTime, 5.f) * 2.f),
              0.0025f * (i + 1.f) * x, 1.1f);
    col = palette(__ctx, (length(uv0) + sin(iTime) + i + 1.f) / 2.0f);
    finalColor += col * d;
  }
  fragColor = f32x4(finalColor, 1.0f);
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
