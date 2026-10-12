// Port of https://www.shadertoy.com/view/cslXR7; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/cslXR7.json.
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
struct Context {
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float hash21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a, f32x2(22.34f, 35.34f))) * 483434.f);
}
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.f)) + min(max(d.data[0], d.data[1]), 0.f);
}
f32x2x2 trot;
float glo1 = 0.f, slo1 = 0.f, glo2 = 0.f, slo2 = 0.f;
const float ofx = 1.125f;
float gyroid(Context &__ctx, f32x3 p, float s, float t, float b) {
  p *= s;
  return abs(dot(sin(p * ofx), cos(p.swizzle<2, 0, 1>())) - b) / (s * ofx) - t;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(1e5f, 1.f);
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (trot));
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() + (f32x2(-1.25f, -iTime)));
  float m1 = 4.85f + 4.75f * sin(iTime * .15f);
  float m2 = 5.85f + 5.75f * cos(iTime * .35f);
  float d = .65f, mf = 1e5f;
  for (float j = -1.f; j < 2.f; j++) {
    f32x3 nf = p;
    nf.data[2] = round(nf.data[2] / d + j) * d;
    float idx = mod(nf.data[2] + 5.f, 10.f);
    float idy = mod(nf.data[2], 12.f);
    float fd = gyroid(__ctx, nf, .75f, .0f, .525f);
    fd = abs(fd) - .045f;
    nf.data[2] = clamp(p.data[2], nf.data[2] - d / 3.5f, nf.data[2] + d / 3.5f);
    fd = length(f32x2(max(.0f, fd), nf.data[2] - p.data[2]));
    if (idx < m1 + .15f && idx > m1)
      slo1 += .002f / (.015f + fd * fd);
    if (idy < m2 + .15f && idy > m2)
      slo2 += .002f / (.015f + fd * fd);
    mf = min(mf, fd);
  }
  if (mf < res.data[0])
    res = f32x2(mf, 3.f);
  return res;
}
f32x3 normal(Context &__ctx, f32x3 p, float t) {
  f32x2 e = f32x2(t * 1e-4f, 0.f);
  float d = map(__ctx, p).data[0];
  f32x3 n = d - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 topLayer(Context &__ctx, f32x3 C, f32x2 uv) {
  float ft = floor(iTime);
  uv *= rot(__ctx, .05f);
  float d = box(__ctx, uv - f32x2(.075f, .0f), f32x2(.85f, .4f)) - .01f;
  f32x2 pv = uv * 2.f;
  float md = floor((pv.data[1] + .05f) / .1f);
  float mf =
      mix(hash21(__ctx, f32x2(ft) + md), hash21(__ctx, f32x2(ft + 1.f) + md),
          pow(smoothstep(0.f, 1.f, fract(iTime)), 22.f)) *
      .22f;
  pv.data[1] = mod(pv.data[1] + .05f, .1f) - .05f;
  float ff = .075f;
  float e =
      box(__ctx, pv + f32x2(1.75f - ff, .0f), f32x2(.15f + ff, .025f)) - .01f;
  float ex = box(__ctx, pv + f32x2(1.89f - mf, .0f), f32x2(mf, .015f)) - .005f;
  float px = 4.f / iResolution.data[0];
  float ei = smoothstep(px, -px, abs(e) - .002f);
  ex = smoothstep(px, -px, ex);
  e = smoothstep(px, -px, e);
  px = 2.f / iResolution.data[0];
  float c = smoothstep(px, -px, d);
  d = smoothstep(px, -px, abs(abs(d) - .005f) - .00125f);
  uv.data[0] -= iTime * .05f;
  f32x2 f = fract(uv * 25.f * rot(__ctx, -.78f)) - .5f;
  C = mix(C, C.swizzle<0, 0, 0>() * .15f, 1.f - c);
  if (mod(f.data[0], 2.f) < 1.f)
    C = mix(C, C + f32x3(.015f), 1.f - c);
  C = mix(C, f32x3(1), d);
  if (abs(md) < 6.5f)
    C = mix(C, f32x3(.001f), e);
  if (abs(md) < 6.5f)
    C = mix(C,
            mix(f32x3(.0f, .09f, .3f), f32x3(.52f, .09f, .58f),
                clamp((uv.data[1] + .25f) * 2.f, 0.f, 1.f)),
            ex);
  if (abs(md) < 6.5f)
    C = mix(C, f32x3(1), ei);
  return C;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 F) {
  trot = rot(__ctx, iTime * .072f);
  f32x2 uv = (2.f * F.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  f32x2 vv = uv;
  f32x3 C = f32x3(0);
  f32x3 ro = f32x3(0, 0, .15f), rd = normalize(f32x3(uv, -1.0f));
  const float dof = .001f;
  const float dofdist = 1.f / 15.f;
  f32x2 off = f32x2(0);
  float x = iMouse.swizzle<0, 1>() == f32x2(0) || iMouse.data[2] < 0.f
                ? 0.f
                : -(iMouse.data[1] / iResolution.data[1] * 1.f - .5f) *
                      3.14159265358f;
  float y = iMouse.swizzle<0, 1>() == f32x2(0) || iMouse.data[2] < 0.f
                ? 0.f
                : -(iMouse.data[0] / iResolution.data[0] * 1.f - .5f) *
                      3.14159265358f;
  f32x2x2 rx = rot(__ctx, x),
          ry = rot(__ctx,
                   iMouse.data[2] < 1.f
                       ? y + 1.725f * sin(3.14159265358f * sin(iTime * .07f))
                       : y);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() * (rx));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (ry));
  rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() * (rx));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (ry));
  float fa = 0.f;
  for (int k = 0; k < 2; k++) {
    f32x3 p = ro;
    float d = 0.f, m = 0.f;
    f32x3 RC = f32x3(0);
    for (int i = 0; i < 172; i++) {
      if (mod(float(i), 2.f) < 1.f) {
        off = texture(__ctx.channel1, F.swizzle<0, 1>() / 1024.f)
                      .swizzle<0, 1>() *
                  2.f -
              1.f;
        f32x2 focus = off * dof;
        ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() + (focus * (d) * .01f));
        rd.set_swizzle<0, 1>(rd.swizzle<0, 1>() +
                             (focus * (d * d) * dofdist * .01f));
      }
      p = ro + d * rd;
      f32x2 ray = map(__ctx, p);
      m = ray.data[1];
      d += i < 82 ? ray.data[0] * .4f : ray.data[0] * .8f;
      if (ray.data[0] < d * 1e-3f || d > 40.f)
        break;
    }
    if (k == 0) {
      fa = d;
      glo1 = slo1;
      glo2 = slo2;
    }
    if (d < 40.f) {
      f32x3 n = normal(__ctx, p, d);
      f32x3 l = normalize(f32x3(-2, 15, -10) - p);
      float spec =
          .45f * pow(max(dot(normalize(p - ro), reflect(l, n)), 0.f), 24.f);
      RC += f32x3(.0f) + spec;
      ro = p + n * .001f;
      rd = reflect(rd, n);
    }
    if (k > 0)
      RC *= .3f;
    C += RC;
  }
  float sp = .2f + .2f * sin(uv.data[0] * 4.2f + iTime * .2f);
  f32x3 fog = mix(f32x3(.20f, .52f, .85f), f32x3(.69f, .16f, .77f),
                  clamp((uv.data[1] + .5f - sp), 0.f, 1.f));
  C = mix(C, f32x3(glo1 * .125f, glo1 * .55f, glo1), clamp(glo1, .0f, .6f));
  C = mix(C, f32x3(glo2, glo2 * .125f, glo2 * .75f), clamp(glo2, .0f, .8f));
  C = mix(C, fog, 1.f - exp(-.00015f * fa * fa * fa));
  C = topLayer(__ctx, C, uv);
  C = clamp(C, f32x3(0), f32x3(1));
  C = pow(C, f32x3(.4545f));
  fragColor = f32x4(C, 1.f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 1) texture_2d<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
  Context __ctx{{channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
