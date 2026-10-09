// Port of https://www.shadertoy.com/view/dtfSzs; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtfSzs.json.
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
const float rep = 32.0f;
const float over = 4.0f;
const float nstep = 1.0f / (rep * over);
const float astep = (2.0f * 3.141592654f) * nstep;
const float pm = 17.0f;
float modPolar(Context &__ctx, f32x2 &p, float aa) {
  const float angle = 2.0f * 3.141592654f / rep;
  float a = aa + angle / 2.f;
  float r = length(p);
  float c = floor(a / angle);
  a = mod(a, angle) - angle / 2.f;
  p = f32x2(cos(a), sin(a)) * r;
  if (abs(c) >= (rep / 2.0f))
    c = abs(c);
  return c;
}
f32x3 sRGB(Context &__ctx, f32x3 t) {
  return mix(1.055f * pow(t, f32x3(1.f / 2.4f)) - 0.055f, 12.92f * t,
             step(t, f32x3(0.0031308f)));
}
f32x3 aces_approx(Context &__ctx, f32x3 v) {
  v = max(v, 0.0f);
  v *= 0.6f;
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
}
float segmentx(Context &__ctx, f32x2 p, float l, float w) {
  p = abs(p);
  p.data[0] -= l * 0.5f - w;
  float d0 = length(p) - w;
  float d1 = p.data[1] - w;
  float d = p.data[0] > 0.0f ? d0 : d1;
  return d;
}
f32x2 df(Context &__ctx, f32x2 p, float noff, float a, float &n) {
  const float ll = 0.5f;
  const float ss = 0.0015f;
  const float bb = ss * 4.0f;
  n = modPolar(__ctx, p, a) / rep + noff;
  float m = 16.0f * sin(iTime * (2.0f * 3.141592654f));
  float anim = sin((2.0f * 3.141592654f) * iTime / 10.0f +
                   pm * noff * (2.0f * 3.141592654f));
  p.data[0] -= 0.75f + 0.25f * anim;
  float l = ll * mix(0.5f, 1.0f, smoothstep(-0.9f, 0.9f, anim));
  float s = ss;
  float b = bb;
  f32x2 p0 = p;
  f32x2 p1 = p;
  p1.data[0] = abs(p1.data[0]);
  p1.data[0] -= l * 0.5f - s;
  float d0 = segmentx(__ctx, p0, l, s);
  float d1 = length(p1) - b;
  return f32x2(d0, d1);
}
f32x2 hextile(Context &__ctx, f32x2 &p) {
  const f32x2 sz = f32x2(1.0f, sqrt(3.0f));
  const f32x2 hsz = 0.5f * sz;
  f32x2 p1 = mod(p, sz) - hsz;
  f32x2 p2 = mod(p - hsz, sz) - hsz;
  f32x2 p3 = dot(p1, p1) < dot(p2, p2) ? p1 : p2;
  f32x2 n = ((p3 - p + hsz) / sz);
  p = p3;
  n -= f32x2(0.5f);
  return round(n * 2.0f) * 0.5f;
}
float hash(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 58.233f))) *
               13758.5453f);
}
f32x3 effect0(Context &__ctx, f32x2 p, float aa) {
  const float zz = 2.75f;
  p /= zz;
  f32x2 hn = hextile(__ctx, p);
  float hh = hash(__ctx, hn);
  p *= zz;
  float n;
  f32x3 col = f32x3(0.0f);
  const f32x2x2 rr = f32x2x2(cos((2.0f * 3.141592654f) / (rep * over)),
                             sin((2.0f * 3.141592654f) / (rep * over)),
                             -sin((2.0f * 3.141592654f) / (rep * over)),
                             cos((2.0f * 3.141592654f) / (rep * over)));
  f32x2 pp = p;
  float a = atan(p.data[1], p.data[0]);
  float ll = length(p);
  for (float i = 0.0f; i < over; ++i) {
    float noff = i * nstep;
    float aoff = i * astep;
    f32x2 d = df(__ctx, p, noff, a - aoff, n);
    d /= aa;
    float g0 = 2.0f / max(max(d.data[0], 0.0f), 0.001f);
    float g1 = 8.0f / max((d.data[1] * d.data[1]), 0.000001f);
    f32x3 a3 = (2.0f * 3.141592654f) * hh + f32x3(0.0f, 1.0f, 2.0f) +
               0.5f * ll + 0.5f * 3.141592654f * abs(n) - 0.1f * iTime;
    f32x3 ccol = 0.5f * (1.0f + cos(a3));
    col += 2.0f * ccol * ccol * g0;
    col += 4.0f * ccol * g1;
    p *= rr;
  }
  col *= smoothstep(0.5f * zz, 0.25f * zz, ll);
  const f32x3 gcol0 = 10000.0f * f32x3(0.0f, 0.5f, 1.0f);
  const f32x3 gcol1 = 0.025f * f32x3(0.0f, 0.25f, 1.0f);
  col += gcol0 * aa * aa + gcol1 / dot(p, p);
  col /= (600.0f * aa);
  return col;
}
f32x2 toSmith(Context &__ctx, f32x2 p) {
  float d = (1.0f - p.data[0]) * (1.0f - p.data[0]) + p.data[1] * p.data[1];
  float x = (1.0f + p.data[0]) * (1.0f - p.data[0]) - p.data[1] * p.data[1];
  float y = 2.0f * p.data[1];
  return f32x2(x, y) / d;
}
f32x2 fromSmith(Context &__ctx, f32x2 p) {
  float d = (p.data[0] + 1.0f) * (p.data[0] + 1.0f) + p.data[1] * p.data[1];
  float x = (p.data[0] + 1.0f) * (p.data[0] - 1.0f) + p.data[1] * p.data[1];
  float y = 2.0f * p.data[1];
  return f32x2(x, y) / d;
}
f32x2 transform(Context &__ctx, f32x2 p) {
  f32x2 off0 = sin(f32x2(1.0f, sqrt(0.5f)) * 0.23f * iTime);
  f32x2 off1 = sin(f32x2(1.0f, sqrt(0.5f)) * 0.13f * iTime);
  f32x2 sp0 = toSmith(__ctx, p);
  f32x2 sp1 = toSmith(__ctx, p + off0);
  f32x2 sp2 = toSmith(__ctx, p - off1);
  f32x2 pp = fromSmith(__ctx, sp0 + sp1 - sp2);
  pp += 0.1f * iTime;
  return pp;
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 np, f32x2 pp) {
  p = transform(__ctx, p);
  np = transform(__ctx, np);
  float aa = distance(p, np) * sqrt(2.0f);
  f32x3 col = effect0(__ctx, p, aa);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  f32x2 pp = p;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 np = p + 1.0f / iResolution.data[1];
  f32x3 col = effect(__ctx, p, np, pp);
  col = aces_approx(__ctx, col);
  col = sRGB(__ctx, col);
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
