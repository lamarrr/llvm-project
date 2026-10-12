// Port of https://www.shadertoy.com/view/DtK3WK; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/DtK3WK.json.
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
float lsp(Context &__ctx, float b, float e, float t) {
  return clamp((t - b) / (e - b), 0.f, 1.f);
}
float eoc(Context &__ctx, float t) { return (t = t - 1.f) * t * t + 1.f; }
float opx(Context &__ctx, float d, float z, float h) {
  f32x2 w = f32x2(d, abs(z) - h);
  return min(max(w.data[0], w.data[1]), 0.f) + length(max(w, 0.f));
}
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.f)) + min(max(d.data[0], d.data[1]), 0.f);
}
float box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.f);
}
float tspeed = 0.f, tmod = 0.f, ga1 = 0.f, ga2 = 0.f, ga3 = 0.f, ga4 = 0.f, gtk,
      stk;
const float sz = 3.5f;
const float db = sz * 4.f;
const float hf = sz / 2.f;
const float rd = .025f;
const float thict = .275f;
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(1e5f, 0);
  p += hf;
  p.data[2] += ga1;
  p.data[0] += ga2;
  float thick =
      .14f + .1f * sin(p.data[2] * .5f + p.data[1] * .75f + iTime * 1.75f);
  thick -= .1f + .05f * sin(p.data[0] * 1.25f + iTime);
  thick = mix(thict, thick, .5f + .5f * sin(iTime * .2f));
  gtk = thick;
  f32x3 q = p, id = floor((q + hf) / sz);
  float chk =
      mod(id.data[1] + mod(id.data[2] + id.data[0], 2.f), 2.f) * 2.f - 1.f;
  q = mod(q + hf, sz) - hf;
  float hs = hash21(__ctx, id.swizzle<0, 2>() + id.data[1]);
  float xhs = fract(35.37f * hs);
  if (hs > .5f)
    q.data[1] = -q.data[1];
  if (chk > .5f)
    q.set_swizzle<0, 1>(-q.swizzle<0, 1>());
  f32x3 q1, q2, q3;
  float trh, trx, jre;
  if (xhs > .75f) {
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
  t *= 1e-4f;
  float d = map(__ctx, p).data[0];
  f32x2 e = f32x2(t, 0);
  f32x3 n = d - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 rgb = clamp(abs(mod(c.data[0] * 6.f + f32x3(0, 4, 2), 6.f) - 3.f) - 1.f,
                    0.f, 1.0f);
  return c.data[2] * mix(f32x3(1), rgb, c.data[1]);
}
f32x4 FC = f32x4(.005f);
f32x4 render(Context &__ctx, f32x3 &ro, f32x3 &rd, f32x3 &ref, bool last,
             float &d, f32x2 uv) {
  f32x3 C = f32x3(0);
  f32x3 p = ro;
  float m;
  for (int i = 0; i < 80; i++) {
    p = ro + rd * d;
    f32x2 ray = map(__ctx, p);
    if (ray.data[0] < 1e-4f * d || d > 50.f)
      break;
    d += i < 30 ? ray.data[0] * .35f : ray.data[0] * .9f;
    m = ray.data[1];
  }
  stk = gtk;
  if (d < 50.f) {
    f32x3 n = normal(__ctx, p, d);
    f32x3 lpos = f32x3(hf, hf, -hf);
    f32x3 l = normalize(lpos - p);
    float diff = clamp(dot(n, l), .05f, 1.f);
    float spec = pow(max(dot(reflect(l, n), rd), .1f), 25.f) * .75f;
    f32x3 clr =
        hsv2rgb(__ctx, f32x3(p.data[2] * .062f + stk * 1.5f + iTime * .02f,
                             .75f, .35f));
    f32x3 h = clr * clamp(diff + spec, 0.f, 1.f);
    ref = h * .65f;
    C = h;
    ro = p + n * 1e-4f;
    rd = reflect(rd, n);
  }
  C = mix(FC.swizzle<0, 1, 2>(), C, exp(-.00055f * d * d * d));
  return f32x4(C, 1.f);
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  tspeed = iTime * .65f;
  tmod = mod(tspeed, 12.f);
  float t1 = lsp(__ctx, 00.f, 03.f, tmod);
  t1 = eoc(__ctx, t1);
  t1 = t1 * t1 * t1;
  float t2 = lsp(__ctx, 03.f, 06.f, tmod);
  t2 = eoc(__ctx, t2);
  t2 = t2 * t2 * t2;
  float t3 = lsp(__ctx, 06.f, 9.f, tmod);
  t3 = eoc(__ctx, t3);
  t3 = t3 * t3 * t3;
  float t4 = lsp(__ctx, 9.f, 12.f, tmod);
  t4 = eoc(__ctx, t4);
  t4 = t4 * t4 * t4;
  ga1 = (t1 * db) - (t3 * db);
  ga2 = (t2 * db) - (t4 * db);
  ga3 = (t1 - t3) * 3.141592653f;
  ga4 = (t2 - t4) * 3.141592653f;
  f32x2 uv = (2.f * F.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  uv *= rot(__ctx, ga3 + ga4);
  f32x3 ro = f32x3(0, 0, .1f);
  f32x3 rd = normalize(f32x3(uv, -1));
  float x =
      iMouse.swizzle<0, 1>() == f32x2(0) || iMouse.data[2] < 1.f
          ? 0.f
          : (iMouse.data[1] / iResolution.data[1] * .5f - .25f) * 6.283185307f;
  float y =
      iMouse.swizzle<0, 1>() == f32x2(0) || iMouse.data[2] < 1.f
          ? 0.f
          : -(iMouse.data[0] / iResolution.data[0] * .5f - .25f) * 6.283185307f;
  f32x2x2 rx = rot(__ctx, x + ga3), ry = rot(__ctx, y - ga4);
  ro.set_swizzle<2, 1>(ro.swizzle<2, 1>() * (rx));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (ry));
  rd.set_swizzle<2, 1>(rd.swizzle<2, 1>() * (rx));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (ry));
  f32x3 C = f32x3(0), ref = f32x3(0), fil = f32x3(1);
  float d = 0.f;
  for (float i = 0.f; i < 2.f; i++) {
    f32x4 pass = render(__ctx, ro, rd, ref, i == 2.f - 1.f, d, uv);
    C += pass.swizzle<0, 1, 2>() * fil;
    fil *= ref;
    if (i == 0.f)
      FC = f32x4(FC.swizzle<0, 1, 2>(), exp(-.00055f * d * d * d));
  }
  C = mix(C, FC.swizzle<0, 1, 2>(), 1.f - FC.data[3]);
  C = pow(C, f32x3(.4545f));
  O = f32x4(C, 1);
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
