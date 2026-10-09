// Port of https://www.shadertoy.com/view/cd2yRh; author: scry.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cd2yRh.json.
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
f32x3 cs = f32x3(1.f, 2.f, 3.f);
f32x2x2 r2d(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float sdBox(Context &__ctx, f32x2 p, f32x2 s) {
  f32x2 q = abs(p) - s;
  return length(max(q, 0.f)) + min(0.f, max(q.data[0], q.data[1]));
}
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
f32x3 slogo(Context &__ctx, f32x2 uv, float ar_, float size) {
  size = 240.f / size;
  uv.data[0] = 1.f - uv.data[0];
  f32x2 px = f32x2(1.f / 3.f, 1.f / 5.f);
  float ls = 4.1f;
  uv *= 240.f / 5.25f / size;
  ls += 2.f;
  float ul = length(uv);
  ul = length(f32x2(uv.data[0] * 0.5f, uv.data[1]) - 0.5f);
  uv -= 0.4f;
  uv.data[0] *= iResolution.swizzle<0, 1>().data[0] /
                iResolution.swizzle<0, 1>().data[1] * 1.75f;
  uv.data[1] *= 1.04f;
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
  f32x3 l = hsv2rgb(__ctx, f32x3(b + iTime / 40.f, 0.1f, rr - b * 1.9f)) * rr;
  l -= 0.1f - clamp(ul * 0.1f, rr * 1.f - b, 0.1f);
  return f32x3(l);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 tv = uv;
  uv -= 0.5f;
  uv.data[0] *=
      iResolution.swizzle<0, 1>().data[0] / iResolution.swizzle<0, 1>().data[1];
  uv *= 4.f;
  f32x3 col = f32x3(0.f);
  float b = sdBox(__ctx, uv, f32x2(0.f));
  f32x2 s = f32x2(0.2f, 0.7f);
  float t = iTime * acos(-1.f) / 10.f;
  float rtime = iTime * acos(-1.f) / 10.f / 4.f;
  float sv = 16.f;
  for (int i = 0; i < 8; i++) {
    uv += sin(uv.data[0] * 14.f) * float(i + 1) * 0.02f;
    uv *= r2d(__ctx, acos(-1.f) / 180.f * 45.f + sin(rtime * sv) / sv + rtime);
    t = -t;
  }
  uv += f32x2(sin(iTime * acos(-1.f) / 10.f), cos(iTime * acos(-1.f) / 10.f)) *
        0.9f;
  for (int i = 0; i < 25; i++) {
    float ii = float(i);
    float b_ = sdBox(__ctx, uv + s, f32x2(s));
    uv *= r2d(__ctx, acos(-1.f) / 180.f * 90.f);
    b = min(b_, b);
    col +=
        smoothstep(0.1f, 0.f, abs(b_ - 0.3f)) *
        (sin(cs + float(i) * 0.9f + iTime * acos(-1.f) / 10.f) * 0.3f + 0.7f) *
        0.2f;
    col +=
        smoothstep(0.5f, 0.f, (b_ - 0.25f)) *
        (sin(cs + float(i) * 2.5f + iTime * acos(-1.f) / 10.f) * 0.3f + 0.7f) *
        0.2f;
    ii += 0.2f;
    f32x2 ua = f32x2(sin(ii * ii * 0.5f), cos(ii * ii * 0.5f));
    ua = floor(ua * 2.f) / 3.f;
    ua += f32x2(sin(iTime * acos(-1.f) / 10.f + ii),
                cos(iTime * acos(-1.f) / 10.f - ii)) *
          0.6f;
    uv += ua * 2.2f;
  }
  col *= 1.8f;
  col -= 0.8f;
  col = clamp(col, -0.5f, 1.f);
  col += slogo(__ctx, tv, 1.f, 300.f / 2.f) *
         (sin(f32x3(cs * 0.6f + tv.data[0] * 20.f)) * 0.4f + 0.6f) * 0.5f;
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
