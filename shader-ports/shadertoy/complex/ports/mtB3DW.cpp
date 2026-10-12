// Port of https://www.shadertoy.com/view/mtB3DW; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mtB3DW.json.
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
const f32x4 hsv2rgb_K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 p =
      abs(fract(c.swizzle<0, 0, 0>() + hsv2rgb_K.swizzle<0, 1, 2>()) * 6.0f -
          hsv2rgb_K.swizzle<3, 3, 3>());
  return c.data[2] * mix(hsv2rgb_K.swizzle<0, 0, 0>(),
                         clamp(p - hsv2rgb_K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
float circle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
f32x2 mod2(Context &__ctx, f32x2 &p, f32x2 size) {
  f32x2 c = floor((p + size * 0.5f) / size);
  p = mod(p + size * 0.5f, size) - size * 0.5f;
  return c;
}
float hash(Context &__ctx, f32x2 uv) {
  return fract(sin(dot(uv, f32x2(12.9898f, 78.233f))) * 43758.5453f);
}
float tanh_approx(Context &__ctx, float x) {
  float x2 = x * x;
  return clamp(x * (27.0f + x2) / (27.0f + 9.0f * x2), -1.0f, 1.0f);
}
float df(Context &__ctx, f32x2 p, float &n, float &sc) {
  f32x2 pp = p;
  float sz = 2.0f;
  float r = 0.0f;
  for (int i = 0; i < 5; ++i) {
    f32x2 nn = mod2(__ctx, pp, f32x2(sz));
    sz /= 3.0f;
    float rr = hash(__ctx, nn + 123.4f);
    r += rr;
    if (rr < 0.5f)
      break;
  }
  float d = circle(__ctx, pp, 1.25f * sz);
  n = fract(r);
  sc = sz;
  return d;
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
  p *= 3.0f;
  const f32x2x2 rot0 = f32x2x2(cos(1.0f), sin(1.0f), -sin(1.0f), cos(1.0f));
  const f32x2x2 rot1 = f32x2x2(cos(-2.0f), sin(-2.0f), -sin(-2.0f), cos(-2.0f));
  f32x2 off0 = 4.0f * cos(f32x2(1.0f, sqrt(0.5f)) * 0.23f * iTime);
  f32x2 off1 = 3.0f * cos(f32x2(1.0f, sqrt(0.5f)) * 0.13f * iTime);
  f32x2 sp0 = toSmith(__ctx, p);
  f32x2 sp1 = toSmith(__ctx, (p + off0) * rot0);
  f32x2 sp2 = toSmith(__ctx, (p - off1) * rot1);
  f32x2 pp = fromSmith(__ctx, sp0 + sp1 - sp2);
  pp += 0.25f * iTime;
  return pp;
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 np, f32x2 pp) {
  p = transform(__ctx, p);
  np = transform(__ctx, np);
  float aa = distance(p, np) * sqrt(2.0f);
  const float r = 25.0f;
  float a = 0.05f * (2.0f * 3.141592654f) * iTime / r;
  const float z = 1.0f;
  p /= z;
  float n = 0.0f;
  float sc = 0.0f;
  float d = df(__ctx, p, n, sc) * z;
  f32x3 col = f32x3(0.0f);
  f32x3 hsv = f32x3(n - 0.25f * d / sc, 0.5f + 0.5f * d / sc, 1.0f);
  f32x3 rgb = hsv2rgb(__ctx, hsv);
  col = mix(col, rgb, smoothstep(aa, -aa, d));
  const f32x3 gcol1 =
      (f32x3(0.55f, 0.6667f, 3.0f).data[2] *
       mix(hsv2rgb_K.swizzle<0, 0, 0>(),
           clamp(abs(fract(f32x3(0.55f, 0.6667f, 3.0f).swizzle<0, 0, 0>() +
                           hsv2rgb_K.swizzle<0, 1, 2>()) *
                         6.0f -
                     hsv2rgb_K.swizzle<3, 3, 3>()) -
                     hsv2rgb_K.swizzle<0, 0, 0>(),
                 0.0f, 1.0f),
           f32x3(0.55f, 0.6667f, 3.0f).data[1]));
  col *= smoothstep(0.25f, 0.f, aa);
  col += gcol1 * tanh_approx(__ctx, 0.05f * aa);
  col *= smoothstep(1.5f, 0.5f, length(pp));
  col = sqrt(col);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  f32x2 pp = p;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 np = p + 1.0f / iResolution.data[1];
  f32x3 col = effect(__ctx, p, np, pp);
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
