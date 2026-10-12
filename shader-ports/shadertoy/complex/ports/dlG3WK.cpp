// Port of https://www.shadertoy.com/view/dlG3WK; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/dlG3WK.json.
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
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float hash21(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(23.73f, 59.71f) + iDate.data[2])) * 4832.3234f);
}
float opx(Context &__ctx, float d, float z, float h) {
  f32x2 w = f32x2(d, abs(z) - h);
  return min(max(w.data[0], w.data[1]), 0.f) + length(max(w, 0.f));
}
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.f)) + min(max(d.data[0], d.data[1]), 0.f);
}
f32x2 path(Context &__ctx, float z) {
  f32x2 p1 = f32x2(7.38f * sin(z * .15f) + 5.38f * cos(z * .075f),
                   2.4f * cos(z * .0945f));
  f32x2 p2 = f32x2(5.2f * sin(z * .089f),
                   2.31f * sin(z * .127f) + 3.5f * cos(z * .095f));
  return (p1 - p2) * .3f;
}
f32x2x2 r90;
float tspeed = 0.f;
f32x3 lp = f32x3(0);
const float sz = 2.65f;
const float hf = sz / 2.f;
const float rd = .025f;
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(1e5f, 0);
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() + (hf));
  f32x2 tun = p.swizzle<0, 1>() - path(__ctx, p.data[2]);
  f32x3 q = f32x3(tun, p.data[2]), id = floor((q + hf) / sz);
  float thick = .0825f + .0625f * sin(p.data[2] * .75f);
  thick -= .0125f * cos(p.data[1] * .62f) + .0125f * sin(p.data[0] * 1.25f);
  float chk =
      mod(id.data[1] + mod(id.data[2] + id.data[0], 2.f), 2.f) * 2.f - 1.f;
  q = mod(q + hf, sz) - hf;
  float hs = hash21(__ctx, id.swizzle<0, 2>() + id.data[1]);
  float xhs = fract(2.33f * hs + id.data[1]);
  if (hs > .5f)
    q.set_swizzle<0, 2>(q.swizzle<0, 2>() * (r90));
  if (chk > .5f)
    q.set_swizzle<2, 1>(q.swizzle<2, 1>() * (r90));
  f32x3 q1, q2, q3;
  float trh, trx, jre;
  if (xhs > .65f) {
    q1 = q;
    q2 = q + f32x3(0, hf, hf);
    q3 = q - f32x3(0, hf, hf);
    trh = opx(__ctx, box(__ctx, q1.swizzle<0, 2>(), f32x2(sz, thick)),
              q1.data[1], thick) -
          rd;
    trx = opx(__ctx, abs(length(q2.swizzle<1, 2>()) - hf) - thick, q.data[0],
              thick) -
          rd;
    jre = opx(__ctx, abs(length(q3.swizzle<1, 2>()) - hf) - thick, q.data[0],
              thick) -
          rd;
  } else {
    q1 = q + f32x3(hf, 0, -hf);
    q2 = q + f32x3(0, hf, hf);
    q3 = q - f32x3(hf, hf, 0);
    trh = opx(__ctx, abs(length(q1.swizzle<0, 2>()) - hf) - thick, q.data[1],
              thick) -
          rd;
    trx = opx(__ctx, abs(length(q2.swizzle<1, 2>()) - hf) - thick, q.data[0],
              thick) -
          rd;
    jre = opx(__ctx, abs(length(q3.swizzle<0, 1>()) - hf) - thick, q.data[2],
              thick) -
          rd;
  }
  if (trh < res.data[0])
    res = f32x2(trh, 2.f);
  if (trx < res.data[0])
    res = f32x2(trx, 3.f);
  if (jre < res.data[0])
    res = f32x2(jre, 4.f);
  return res;
}
f32x3 normal(Context &__ctx, f32x3 p, float t) {
  float e = 1e-4f * t;
  f32x2 h = f32x2(1, -1) * 1.5773f;
  f32x3 n =
      h.swizzle<0, 1, 1>() * map(__ctx, p + h.swizzle<0, 1, 1>() * e).data[0] +
      h.swizzle<1, 1, 0>() * map(__ctx, p + h.swizzle<1, 1, 0>() * e).data[0] +
      h.swizzle<1, 0, 1>() * map(__ctx, p + h.swizzle<1, 0, 1>() * e).data[0] +
      h.swizzle<0, 0, 0>() * map(__ctx, p + h.swizzle<0, 0, 0>() * e).data[0];
  return normalize(n);
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 rgb = clamp(abs(mod(c.data[0] * 6.f + f32x3(0, 4, 2), 6.f) - 3.f) - 1.f,
                    0.f, 1.0f);
  return c.data[2] * mix(f32x3(1), rgb, c.data[1]);
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  tspeed = iTime * 1.25f;
  r90 = rot(__ctx, 1.5707f);
  f32x3 C = f32x3(0);
  f32x2 uv = (2.f * F.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  f32x3 ro = f32x3(0, 0, .1f);
  f32x3 rd = normalize(f32x3(uv, -1.f));
  float x =
      iMouse.swizzle<0, 1>() == f32x2(0) || (iMouse.data[2] < 1.f)
          ? (iTime * .0095f) * 6.283185307f
          : -(iMouse.data[1] / iResolution.data[1] * .8f - .4f) * 6.283185307f;
  float y =
      iMouse.swizzle<0, 1>() == f32x2(0) || (iMouse.data[2] < 1.f)
          ? (iTime * -.0125f) * 6.283185307f
          : -(iMouse.data[0] / iResolution.data[0] * 2.f - 1.f) * 6.283185307f;
  f32x2x2 rx = rot(__ctx, x), ry = rot(__ctx, y);
  ro.set_swizzle<2, 1>(ro.swizzle<2, 1>() * (rx));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (ry));
  rd.set_swizzle<2, 1>(rd.swizzle<2, 1>() * (rx));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (ry));
  ro.data[2] -= tspeed;
  ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() + (path(__ctx, ro.data[2])));
  float d, m;
  f32x3 p = f32x3(0);
  for (int i = 0; i < 98; i++) {
    p = ro + rd * d;
    f32x2 ray = map(__ctx, p);
    d += i < 32 ? ray.data[0] * .35f : ray.data[0] * .85f;
    m = ray.data[1];
    if (ray.data[0] < 1e-4f * d || d > 60.f)
      break;
  }
  if (d < 60.f) {
    C = hsv2rgb(__ctx,
                f32x3(clamp(d * .035f, 0.f, 1.f) + iTime * .05f, 1.f, .5f));
  }
  C = mix(f32x3(.01f), C, exp(-.00125f * d * d * d));
  C = pow(C, f32x3(.4545f));
  O = f32x4(C, 1.f);
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
