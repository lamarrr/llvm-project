// Port of https://www.shadertoy.com/view/ss2fWz; author: scry.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ss2fWz.json.
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
f32x3 rgb2hsv(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
  f32x4 p = mix(f32x4(c.swizzle<2, 1>(), K.swizzle<3, 2>()),
                f32x4(c.swizzle<1, 2>(), K.swizzle<0, 1>()),
                step(c.data[2], c.data[1]));
  f32x4 q =
      mix(f32x4(p.swizzle<0, 1, 3>(), c.data[0]),
          f32x4(c.data[0], p.swizzle<1, 2, 0>()), step(p.data[0], c.data[0]));
  float d = q.data[0] - min(q.data[3], q.data[1]);
  float e = 1.0e-10f;
  return f32x3(abs(q.data[2] + (q.data[3] - q.data[1]) / (6.0f * d + e)),
               d / (q.data[0] + e), q.data[0]);
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
float bitm(Context &__ctx, f32x2 uv, int c) {
  float h = 5.f;
  float w = 3.f;
  int p = int(pow(2.f, w));
  float line1 = 9591.f;
  uv = floor(f32x2(uv.data[0] * w, uv.data[1] * h)) / f32x2(w, w);
  float c1 = 0.f;
  float cc = uv.data[0] + uv.data[1] * w;
  c1 = mod(floor(float(c) / exp2(ceil(cc * w - 0.6f))), 2.f);
  c1 *= step(0.f, uv.data[0]) * step(0.f, uv.data[1]);
  c1 *= step(0.f, (-uv.data[0] + 0.99f)) * step(0.f, (-uv.data[1] + 1.6f));
  return (c1);
}
f32x3 slogo(Context &__ctx, f32x2 uv, float ar, float size) {
  f32x2 px = f32x2(1.f / 3.f, 1.f / 5.f);
  float ls = 4.1f;
  uv.data[0] = 0.993f - uv.data[0];
  uv *= 8.f * ls * (1.f / size);
  ls += 2.f;
  float ul = length(uv);
  uv -= px.swizzle<1, 0>() * 0.5f * 0.5f * ls;
  ul = length(f32x2(uv.data[0] * 0.5f, uv.data[1]) - 0.5f);
  uv.data[0] *= ar * 1.75f;
  int s = 29671;
  int c = 29263;
  int r = 31469;
  int y = 23186;
  uv.data[0] = 5.f - uv.data[0];
  float b = bitm(__ctx, uv, s);
  uv.data[0] -= 1.f / 3.f * 4.f;
  b += bitm(__ctx, uv, c);
  uv.data[0] -= 1.f / 3.f * 4.f;
  b += bitm(__ctx, uv, r);
  uv.data[0] -= 1.f / 3.f * 4.f;
  b += bitm(__ctx, uv, y);
  float rr = step(0.f, uv.data[0] + px.data[0] * 13.f) *
             step(0.f, uv.data[1] + px.data[1]) *
             step(0.f, (-uv.data[0] + px.data[0] * 4.f)) *
             step(0.f, (-uv.data[1] + px.data[1] * 6.f));
  b = clamp(b, 0.f, 1.f);
  f32x3 l = hsv2rgb(__ctx, f32x3(b + iTime, 1.f, rr - b * 1.9f)) * rr;
  l -= 0.1f - clamp(ul * 0.1f, rr * 1.f - b, 0.1f);
  return f32x3(l);
}
f32x2x2 r2d(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
f32x4 map(Context &__ctx, f32x3 p) {
  f32x3 o = p;
  p.data[2] -= .2f;
  p.set_swizzle<0, 1>(
      p.swizzle<0, 1>() *
      (r2d(__ctx, sin(iTime * 3.141592654f / 5.f + p.data[2] * 4.f) *
                      p.data[2] * 0.1f)));
  f32x3 c = p;
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() *
                      (r2d(__ctx, iTime * 3.141592654f / 5.f +
                                      c.data[2] * 3.141592654f * 1.f)));
  float rp = 2.f;
  float pm = length(p);
  p = p / dot(p, p);
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() + (.1f));
  p.set_swizzle<1, 2>(
      p.swizzle<1, 2>() *
      (r2d(__ctx, iTime * 3.141592654f / 5.f + (c.data[1]) * 3.141592654f)));
  p.set_swizzle<0, 2>(
      p.swizzle<0, 2>() *
      (r2d(__ctx, iTime * 3.141592654f / 5.f + (c.data[0]) * 3.141592654f)));
  p.data[2] -= .1f;
  p = p / dot(p, p);
  p = (fract(p / rp) - 0.5f) * rp;
  float d = length(p) - 0.03f;
  d *= 1.25f - pm * 0.3f;
  return f32x4(p, d);
}
f32x2 RM(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  float ii = 0.f;
  for (int i = 0; i < 350; i++) {
    f32x3 p = ro + rd * dO;
    float dS = map(__ctx, p).data[3];
    dO += dS * 0.16f;
    ii += 0.016f;
    if (dO > 100.f || dS < 0.01f) {
      break;
    }
  }
  return f32x2(dO, ii);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 R = iResolution.swizzle<0, 1>();
  f32x2 tv = uv;
  float ar = R.data[0] / R.data[1];
  uv -= 0.5f;
  uv.data[0] *= ar;
  float dv = 4.f;
  f32x2 fv = floor(uv * dv);
  f32x3 col = f32x3(0.f);
  f32x3 ro = f32x3(0.f);
  f32x3 rd = normalize(f32x3(uv, 1.f));
  f32x2 d = RM(__ctx, ro, rd);
  col = f32x3(d.data[1] * 0.2f);
  col.data[0] += sin(d.data[1] * 4.f) / 10.f;
  col.data[2] += d.data[0] * 0.02f;
  col.data[1] *= d.data[1] * 0.3f;
  col -= sin(col * 40.f + iTime * 3.141592654f / 5.f) * 0.02f;
  col = sin(col * 3.f + 3.7f + d.data[0] * 7.f);
  col = cos(col * 1.57f);
  col = rgb2hsv(__ctx, f32x3(col.data[0], col.data[1], col.data[2]));
  col.data[0] *= 0.56f;
  col.data[0] += 0.5f;
  col = hsv2rgb(__ctx, col);
  col = clamp(col, 0.f, 1.f);
  col += slogo(__ctx, tv, ar, 1.f) / 24.f;
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
