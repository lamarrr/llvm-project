// Port of https://www.shadertoy.com/view/ftGfzV; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/ftGfzV.json.
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
float hash21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a, f32x2(21.23f, 41.232f))) * 4123.2323f);
}
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float smin(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(.5f + .5f * (d2 - d1) / k, .0f, 1.f);
  return mix(d2, d1, h) - k * h * (1.f - h);
}
float torus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<1, 0>()) - t.data[0], p.data[2]);
  return length(q) - t.data[1];
}
float vcap(Context &__ctx, f32x3 p, float h, float r) {
  p.data[1] -= clamp(p.data[1], 0.0f, h);
  return length(p) - r;
}
float cap(Context &__ctx, f32x3 p, float r, float h) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 2>()), p.data[1])) - f32x2(h, r);
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
float time;
f32x2x2 rx, ry, turn, r25;
f32x3 hit, hitpoint;
float ghost(Context &__ctx, f32x3 p, float hs) {
  float res = 1e5f;
  f32x3 q = p - f32x3(0, -1.25f, 0);
  f32x3 q2 = q - f32x3(0, 1.5f, .25f);
  f32x3 q3 =
      f32x3(abs(q.data[0]), q.swizzle<1, 2>()) - f32x3(.5f, 2.72f, 1.25f);
  f32x3 q4 = q - f32x3(0, 2.72f, 1.65f);
  q4.data[0] = abs(q4.data[0]);
  q4.set_swizzle<0, 2>(q4.swizzle<0, 2>() * (r25));
  float ghst = vcap(__ctx, q, 2.75f, 1.6f);
  float eyes = length(q3) - .45f;
  float lids = torus(__ctx, q4 - f32x3(.6f, 0, 0), f32x2(.4f, .09f));
  float chst = vcap(__ctx, q, 2.7f, 1.25f);
  float a = atan(q.data[2], q.data[0]);
  float tw = .2f * sin(a * 6.f);
  tw *= pow(length(q.swizzle<0, 2>()), 1.f);
  float cuting = cap(__ctx, q + f32x3(0, 1.f, 0), 1.f - tw, 1.65f) * .75f;
  ghst = max(ghst, -cuting);
  ghst = max(ghst, -eyes);
  ghst = smin(__ctx, lids, ghst, .08f);
  ghst = max(ghst, -chst);
  if (ghst < res) {
    res = ghst;
  }
  return res;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(100.f, 0.f);
  f32x3 q = p;
  float tt = .35f + .35f * sin(time * 1.05f) + cos(.5f * cos(time * .3f) * 5.f);
  float pt = 2.5f * sin(time * 1.4f);
  p.data[1] += .5f + pt;
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (turn));
  p.set_swizzle<0, 2>(abs(p.swizzle<0, 2>()) - 3.35f);
  f32x2 np = p.swizzle<0, 2>();
  float lt = length(np * 3.14159265358f);
  float amp = (1.f / lt);
  amp = clamp(amp, 0.f, .2f);
  float wb = amp * sin(lt - time * 6.28318530718f);
  q.data[1] -= (wb);
  float be = q.data[1] + 2.5f;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                      (rot(__ctx, tt * 3.14159265358f * .75f)));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot(__ctx, tt * 3.14159265358f)));
  float me = ghost(__ctx, p - f32x3(0, .15f, 0), 2.f);
  me = smin(__ctx, me, be, 1.3f);
  if (me < res.data[0]) {
    res = f32x2(me, 1.f);
    hit = q;
  }
  return res;
}
f32x3 normal(Context &__ctx, f32x3 p, float t, float mindist) {
  float e = mindist * t;
  f32x2 h = f32x2(1.f, -1.f) * .5773f;
  return normalize(
      h.swizzle<0, 1, 1>() * map(__ctx, p + h.swizzle<0, 1, 1>() * e).data[0] +
      h.swizzle<1, 1, 0>() * map(__ctx, p + h.swizzle<1, 1, 0>() * e).data[0] +
      h.swizzle<1, 0, 1>() * map(__ctx, p + h.swizzle<1, 0, 1>() * e).data[0] +
      h.swizzle<0, 0, 0>() * map(__ctx, p + h.swizzle<0, 0, 0>() * e).data[0]);
}
f32x3 hue(Context &__ctx, float t) {
  f32x3 d = f32x3(.173f, .471f, .788f);
  return .675f + .425f * cos(6.28318530718f * (f32x3(1.25f, 1, .75f) * t + d));
}
f32x3 shade(Context &__ctx, f32x3 p, f32x3 rd, float d, float m, f32x3 &n,
            f32x2 F) {
  n = normal(__ctx, p, d, 1.f);
  f32x3 lpos = f32x3(2.f, 5, -2);
  f32x3 l = normalize(lpos - p);
  float diff = clamp(dot(n, l), .075f, 1.f);
  f32x3 th = f32x3(.1f);
  float dd = clamp((hitpoint.data[2] + 3.f) * .1f, 0.f, 1.f);
  f32x3 r = fract(hitpoint * .25f) - .5f;
  f32x3 f = fract(hitpoint * .75f) - .5f;
  f32x3 clr = mix(f32x3(.31f, .13f, .01f), f32x3(0.188f, 0.012f, 0.310f), dd);
  f32x3 clx =
      mix(f32x3(0.361f, 0.439f, 0.459f), f32x3(0.188f, 0.012f, 0.310f), dd);
  th = (r.data[0] * r.data[2] > 0.f) ? clr : clx;
  th = (f.data[0] * f.data[2] > 0.f) ? f32x3(.02f) : th;
  f32x3 h = f32x3(.1f);
  h = mix(f32x3(.09f, .19f, .19f), th, clamp(1.f / d, 0.f, 1.f));
  return h * diff;
}
f32x3 render(Context &__ctx, f32x2 uv, f32x2 F) {
  time = iTime;
  turn = rot(__ctx, iTime * 10.f * 3.14159265358f / 180.f);
  f32x3 C = f32x3(.0f);
  r25 = rot(__ctx, -.35f);
  float zoom = 10.f;
  f32x3 ro = f32x3(uv * zoom, -(zoom + 15.f));
  f32x3 rd = f32x3(0, 0, 1.f);
  float x = iMouse.swizzle<0, 1>() == f32x2(0)
                ? 0.f
                : -(iMouse.data[1] / iResolution.data[1] * .5f - .25f) *
                      3.14159265358f;
  float y = iMouse.swizzle<0, 1>() == f32x2(0)
                ? 0.f
                : -(iMouse.data[0] / iResolution.data[0] * .5f - .25f) *
                      3.14159265358f;
  rx = rot(__ctx, -.58f);
  ry = rot(__ctx, .78f + y);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() * (rx));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (ry));
  rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() * (rx));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (ry));
  f32x3 p = ro + rd * .1f;
  float atten = 1.f;
  float k = 1.f, b = 5.f;
  float iv = 1.f;
  for (int i = 0; i < 128; i++) {
    f32x2 ray = map(__ctx, p);
    f32x3 n = f32x3(0);
    float d = ray.data[0] * .8f;
    float m = ray.data[1];
    p += rd * d * k;
    if (d * d < 1e-8f) {
      hitpoint = hit;
      C = mix(C, shade(__ctx, p, rd, d, ray.data[1], n, F), atten);
      b--;
      if (b < 0.f || m == 4.f)
        break;
      atten *= .85f;
      p += rd * .05f;
      k = sign(map(__ctx, p).data[0]);
      f32x3 rf = refract(rd, n, iv > 0.f ? .875f : 1.1f);
      iv *= -1.f;
      if (length(rf) == 0.f)
        rf = reflect(rd, n);
      rd = rf;
      p += -n * .005f;
    }
    if (distance(p, rd) > 25.f) {
      break;
    }
  }
  return C;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  time = fract(iTime);
  turn = rot(__ctx, time * 10.f * 3.14159265358f / 180.f);
  f32x3 C = f32x3(0.0f);
  for (int m = (min(iFrame, 0)); m < 2; m++)
    for (int n = (min(iFrame, 0)); n < 2; n++) {
      f32x2 o = f32x2(float(m), float(n)) / float(2) - 0.5f;
      f32x2 uv = (-iResolution.swizzle<0, 1>() + 2.0f * (F + o)) /
                 max(iResolution.data[0], iResolution.data[1]);
      f32x3 color = render(__ctx, uv, F);
      color = 1.35f * color / (1.0f + color);
      color = pow(color, f32x3(0.4545f));
      C += color;
    }
  C /= float(2 * 2);
  O = f32x4(C, 1.0f);
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
