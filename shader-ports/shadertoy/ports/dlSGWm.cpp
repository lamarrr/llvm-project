// Port of https://www.shadertoy.com/view/dlSGWm; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlSGWm.json.
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
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
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
float dot2(Context &__ctx, f32x2 p) { return dot(p, p); }
f32x2 df(Context &__ctx, f32x2 p, float aa, float &h, float &sc) {
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
  float d0 = box(__ctx, pp, f32x2(1.45f * sz - 0.75f * aa)) - 0.05f * sz;
  float d1 = sqrt(sqrt(dot2(__ctx, pp * pp)));
  h = fract(r);
  sc = sz;
  return f32x2(d0, d1);
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
  p *= 2.0f;
  const f32x2x2 rot0 = f32x2x2(cos(1.0f), sin(1.0f), -sin(1.0f), cos(1.0f));
  const f32x2x2 rot1 = f32x2x2(cos(-2.0f), sin(-2.0f), -sin(-2.0f), cos(-2.0f));
  f32x2 off0 = 4.0f * cos(f32x2(1.0f, sqrt(0.5f)) * 0.23f * iTime);
  f32x2 off1 = 3.0f * cos(f32x2(1.0f, sqrt(0.5f)) * 0.13f * iTime);
  f32x2 sp0 = toSmith(__ctx, p);
  f32x2 sp1 = toSmith(__ctx, (p + off0) * rot0);
  f32x2 sp2 = toSmith(__ctx, (p - off1) * rot1);
  f32x2 pp = fromSmith(__ctx, sp0 + sp1 - sp2);
  p = pp;
  p += 0.25f * iTime;
  return p;
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 np, f32x2 pp) {
  p = transform(__ctx, p);
  np = transform(__ctx, np);
  float aa = distance(p, np) * sqrt(2.0f);
  float h = 0.0f;
  float sc = 0.0f;
  f32x2 d2 = df(__ctx, p, aa, h, sc);
  f32x3 col = f32x3(0.0f);
  f32x3 rgb = ((2.0f / 3.0f) *
                   (cos((2.0f * 3.141592654f) * h + f32x3(0.0f, 1.0f, 2.0f)) +
                    f32x3(1.0f)) -
               d2.data[1] / (3.0f * sc));
  col = mix(col, rgb, smoothstep(aa, -aa, d2.data[0]));
  const f32x3 gcol1 = f32x3(.5f, 2.0f, 3.0f);
  col += gcol1 * tanh_approx(__ctx, 0.025f * aa);
  col = clamp(col, 0.0f, 1.0f);
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
