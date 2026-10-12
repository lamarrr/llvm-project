// Port of https://www.shadertoy.com/view/7ltcW8; author: guil.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7ltcW8.json.
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

const float pi = 3.1415926535897932f;
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
f32x2 cadd(Context &__ctx, f32x2 a, float s) {
  return f32x2(a.data[0] + s, a.data[1]);
}
f32x2 cmul(Context &__ctx, f32x2 a, f32x2 b) {
  return f32x2(a.data[0] * b.data[0] - a.data[1] * b.data[1],
               a.data[0] * b.data[1] + a.data[1] * b.data[0]);
}
f32x2 cinv(Context &__ctx, f32x2 z) {
  return f32x2(z.data[0], -z.data[1]) / dot(z, z);
}
f32x2 cdiv(Context &__ctx, f32x2 a, f32x2 b) {
  float d = dot(b, b);
  return f32x2(dot(a, b), a.data[1] * b.data[0] - a.data[0] * b.data[1]) / d;
}
f32x2 csqr(Context &__ctx, f32x2 a) {
  return f32x2(a.data[0] * a.data[0] - a.data[1] * a.data[1],
               2.0f * a.data[0] * a.data[1]);
}
f32x2 csqrt(Context &__ctx, f32x2 z) {
  float m = length(z);
  return sqrt(0.5f * f32x2(m + z.data[0], m - z.data[0])) *
         f32x2(1.0f, sign(z.data[1]));
}
f32x2 conj(Context &__ctx, f32x2 z) { return f32x2(z.data[0], -z.data[1]); }
f32x2 cpow(Context &__ctx, f32x2 z, float n) {
  float r = length(z);
  float a = atan(z.data[1], z.data[0]);
  return pow(r, n) * f32x2(cos(a * n), sin(a * n));
}
f32x2 cpow(Context &__ctx, float n, f32x2 z) {
  return pow(n, z.data[0]) *
         f32x2(cos(z.data[1] * log(n)), sin(z.data[1] * log(n)));
}
f32x2 cexp(Context &__ctx, f32x2 z) {
  return exp(z.data[0]) * f32x2(cos(z.data[1]), sin(z.data[1]));
}
f32x2 clog(Context &__ctx, f32x2 z) {
  return f32x2(0.5f * log(z.data[0] * z.data[0] + z.data[1] * z.data[1]),
               atan(z.data[1], z.data[0]));
}
f32x2 csin(Context &__ctx, f32x2 z) {
  float r = exp(z.data[1]);
  return 0.5f * f32x2((r + 1.0f / r) * sin(z.data[0]),
                      (r - 1.0f / r) * cos(z.data[0]));
}
f32x2 cpow(Context &__ctx, f32x2 a, f32x2 b) {
  return cexp(__ctx, cmul(__ctx, b, clog(__ctx, a)));
}
f32x2 ccos(Context &__ctx, f32x2 z) {
  float r = exp(z.data[1]);
  return 0.5f * f32x2((r + 1.0f / r) * cos(z.data[0]),
                      -(r - 1.0f / r) * sin(z.data[0]));
}
f32x2 gamma(Context &__ctx, f32x2 z) {
  const int N = 8;
  float c = sqrt(2.f * pi);
  f32x2 s = f32x2(c, 0.f);
  float f = 1.f;
  for (int k = 1; k < N; k++) {
    c = exp(float(N - k)) * pow(float(N - k), float(k) - .5f) / f;
    f *= -float(k);
    s += cinv(__ctx, z + f32x2(float(k), 0.f)) * c;
  }
  s = cmul(__ctx, s,
           cmul(__ctx, cexp(__ctx, -z - f32x2(float(N), 0.f)),
                cpow(__ctx, z + f32x2(float(N), 0.f), z + f32x2(.5f, 0.f))));
  return cdiv(__ctx, s, z);
}
f32x2 zeta3(Context &__ctx, f32x2 s) {
  const int N = 8;
  f32x2 sum1 = f32x2(0);
  float a = 1.0f;
  for (int i = 1; i <= N; i++) {
    sum1 += a * (cpow(__ctx, float(i), -s));
    a = -a;
  }
  f32x2 sum2 = f32x2(0);
  a = -1.0f;
  float bk = 1.0f / pow(2.f, float(N));
  float ek = bk;
  for (int i = 0; i < N; i++) {
    sum2 += a * ek * (cpow(__ctx, float(2 * N - i), -s));
    bk *= float(N - i) / float(i + 1);
    ek += bk;
    a = -a;
  }
  return cdiv(__ctx, sum1 + sum2,
              f32x2(1, 0) - cpow(__ctx, 2.f, f32x2(1, 0) - s));
}
f32x2 khi(Context &__ctx, f32x2 z) {
  return cmul(
      __ctx,
      cmul(__ctx, cpow(__ctx, 2.f * pi, z) / pi, csin(__ctx, pi * z / 2.f)),
      gamma(__ctx, f32x2(1.f, 0.f) - z));
}
f32x2 zeta(Context &__ctx, f32x2 z) {
  if (z.data[0] < .5f && abs(z.data[1]) < 55.f)
    return cmul(__ctx, khi(__ctx, z), zeta3(__ctx, f32x2(1.f, 0.f) - z));
  return zeta3(__ctx, z);
}
f32x2 boxIntersection(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 rad) {
  f32x3 m = 1.0f / rd;
  f32x3 n = m * ro;
  f32x3 k = abs(m) * rad;
  f32x3 t1 = -n - k;
  f32x3 t2 = -n + k;
  float tN = max(max(t1.data[0], t1.data[1]), t1.data[2]);
  float tF = min(min(t2.data[0], t2.data[1]), t2.data[2]);
  if (tN > tF || tF < 0.0f)
    return f32x2(-1.0f);
  return f32x2(tN, tF);
}
f32x2 f(Context &__ctx, f32x2 s) {
  return cdiv(__ctx, cmul(__ctx, s, s - 1.f), s + 1.f);
}
f32x3 field(Context &__ctx, f32x3 p) {
  f32x2 z = p.swizzle<0, 2>();
  z = zeta(__ctx, 5.f * z);
  float d = p.data[1] - log(length(z) + 1.f);
  return f32x3(z, d);
}
f32x3 col(Context &__ctx, f32x2 u) {
  return hsv2rgb(__ctx, f32x3(atan(u.data[1], u.data[0]) / 6.28f, .6f, 1.f));
}
f32x3 raycast(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.f, dt = .05f, d, dd, tmax;
  f32x2 vt = boxIntersection(__ctx, ro, rd, f32x3(3));
  t = vt.data[0];
  if (t < 0.f)
    return f32x3(1);
  tmax = vt.data[1];
  f32x3 v, c, v1;
  for (int i = 0; i < 100; i++) {
    v = field(__ctx, ro + t * rd);
    d = v.data[2];
    t += dt * abs(d);
    if (t > tmax)
      return f32x3(.5f);
    if (t < vt.data[0])
      return f32x3(.5f);
    if (abs(d) < .0001f)
      break;
  }
  f32x3 l = normalize(f32x3(.0f, .2f, .2f));
  l.set_swizzle<0, 2>(l.swizzle<0, 2>() * (rot(__ctx, t)));
  ro = ro + t * rd;
  t = .02f;
  d = 1.f;
  for (int i = 0; i < 15; i++) {
    d = min((field(__ctx, ro + t * l).data[2]) / t, d);
    t += .2f;
    if (abs(d) < .0001f)
      break;
  }
  c = col(__ctx, v.swizzle<0, 1>()) * clamp(d, .5f, 1.f);
  return c;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float t = iTime * .2f;
  f32x2 q = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 p = -1.0f + 2.0f * q;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 ro = f32x3(8.f, 6.f, 0.f);
  f32x3 rd = -normalize(f32x3(p.data[1], 2.f, p.data[0]));
  rd.set_swizzle<0, 1>(rd.swizzle<0, 1>() * (rot(__ctx, 1.f)));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (rot(__ctx, t)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (rot(__ctx, t)));
  f32x3 col = raycast(__ctx, ro, rd);
  col = clamp(col, 0.f, 1.f);
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
