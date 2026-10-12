// Port of https://www.shadertoy.com/view/ddtSRs; author: spolsh.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddtSRs.json.
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
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float gG = 10e8f;
float gG1 = 10e8f;
f32x3 cA = f32x3(0.2f, 0.6f, 1.2f);
float plas(Context &__ctx, f32x2 v, float time) {
  float c = 0.5f + sin(v.data[0] * 10.0f) + cos(sin(time + v.data[1]) * 20.0f);
  f32x4 d = f32x4(sin(c * 0.2f + cos(time)), c * 0.15f,
                  cos(c * 0.1f + time / .4f) * .25f, 1.0f);
  return dot(d.swizzle<0, 1, 2>(), d.swizzle<0, 1, 2>());
}
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), -sin(a), sin(a), cos(a));
}
float box(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, f32x3(0.f))) +
         min(max(max(q.data[0], q.data[1]), q.data[2]), 0.f);
}
float map(Context &__ctx, f32x3 p) {
  float s = p.data[1] + 0.5f + 0.3f * plas(__ctx, p.swizzle<0, 2>(), iTime) +
            0.1f * plas(__ctx, 0.05f * p.swizzle<0, 2>(), iTime);
  p.data[2] -=
      12.f +
      2.f * sin(100.f *
                (0.005f * iTime +
                 0.004f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]));
  p.data[1] -=
      200.f * (0.002f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]);
  for (float b = 0.f; b < 3.f; ++b) {
    f32x3 p2 = p;
    p2.data[1] +=
        80.f * (0.002f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]);
    p2.set_swizzle<2, 1>(
        p2.swizzle<2, 1>() *
        (rot(__ctx, 20.f * iTime * fract(sin(100.f * b) * 435758.5354f))));
    p2.data[0] -= 0.1f * fract(sin(100.f * b) * 435758.5354f);
    float s4 =
        length(p2) - 0.2f -
        170.f * (0.002f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]);
    s = min(s, s4);
    gG = min(gG, s4);
  }
  f32x3 p1 = p;
  p1.data[2] +=
      20.f * (0.005f * iTime +
              0.004f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]) +
      10.f * iTime;
  p1.data[0] = abs(p1.data[0]);
  p1.data[0] -= 1.9f;
  p1.data[2] = mod(p1.data[2] + 1.0f, 2.f) - 1.0f;
  p1.set_swizzle<0, 1>(p1.swizzle<0, 1>() * (rot(__ctx, 0.7f)));
  s = min(s, box(__ctx, p1, f32x3(0.3f, 5.f, 0.3f)));
  for (float b = 0.f; b < 5.f; ++b) {
    p = abs(p);
    p -= f32x3(.9f, 0.1f, 1.4f);
    p.set_swizzle<1, 2>(
        p.swizzle<1, 2>() *
        (rot(__ctx,
             40.f * (0.005f * iTime +
                     0.004f *
                         texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]))));
    p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rot(__ctx, iTime)));
    float s3 =
        length(p) - 0.01f -
        50.f * (0.001f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]) -
        0.01f * b;
    float s1 = box(
        __ctx, p,
        f32x3(
            0.01f + 50.f * (0.002f *
                            texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]),
            15.f *
                (0.002f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]),
            25.f *
                (0.002f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0])));
    float s2 = box(__ctx, p, f32x3(0.001f, 10.f, 0.005f));
    gG1 = min(gG1, s2);
    if (b > 3.f) {
      gG = min(gG, s1);
    }
    s = min(s, s3);
    s = min(s, s2);
    s = min(s, s1);
  }
  return s;
}
void mainImage(Context &__ctx, f32x4 &out_color, f32x2 fragCoord) {
  f32x2 uv = gl_FragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv -= 0.5f;
  uv /= f32x2(iResolution.data[1] / iResolution.data[0], 1);
  f32x3 col = f32x3(0.f);
  out_color.set_swizzle<0, 1, 2>(col);
  if (abs(uv.data[1]) > 0.4f)
    return;
  f32x3 ro = f32x3(
      0.2f *
          sin(20.f * (0.005f * iTime +
                      0.004f *
                          texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]) +
              iTime),
      0.f, 0.f);
  f32x3 rd = normalize(f32x3(uv, 4.f));
  rd.set_swizzle<0, 1>(
      rd.swizzle<0, 1>() *
      (rot(__ctx, 0.2f * cos(20.f * (0.005f * iTime +
                                     0.004f * texture(__ctx.channel0,
                                                      f32x2(0.0f, 0.5f))
                                                  .data[0]) +
                             iTime))));
  float t = 0.f;
  float tt = 10e8f;
  f32x3 tint = f32x3(1.f);
  for (float bi = 0.f; bi < 3.f; ++bi) {
    for (float i = 0.f; i < 64.f; ++i) {
      t += map(__ctx, ro + rd * t);
      if (t < 0.001f || t > 40.f)
        break;
    }
    tt = bi == 0.f ? t : tt;
    f32x3 p = ro + rd * t;
    f32x2 e = 0.001f * f32x2(-1.f, 1.f);
    f32x3 n =
        normalize(e.swizzle<0, 0, 0>() * map(__ctx, p + e.swizzle<0, 0, 0>()) +
                  e.swizzle<1, 0, 0>() * map(__ctx, p + e.swizzle<1, 0, 0>()) +
                  e.swizzle<0, 1, 0>() * map(__ctx, p + e.swizzle<0, 1, 0>()) +
                  e.swizzle<0, 0, 1>() * map(__ctx, p + e.swizzle<0, 0, 1>()));
    float fre = pow(dot(rd, n) + 1.f, 6.f);
    col += 0.1f * cA * fre;
    tint *= fre;
    rd =
        normalize(reflect(rd, n) + 0.1f * fract(sin(100.f * p) * 435758.5354f));
    ro = p + 0.1f * rd;
  }
  col = mix(1.2f * cA.swizzle<2, 1, 0>(), 0.2f * col, exp(-0.005f * tt * tt));
  float sb = .1f + 40.f * (0.002f *
                           texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]);
  col += sb * cA * exp(-50.f * gG);
  col += sb * cA * exp(-1.f * gG);
  sb *= 2.f;
  col += sb * cA.swizzle<1, 0, 2>() * exp(-50.f * gG1);
  col += sb * cA.swizzle<1, 0, 2>() * exp(-10.f * gG1);
  col += sb * cA.swizzle<1, 0, 2>() * exp(-5.f * gG1);
  col += sb * cA.swizzle<1, 0, 2>() * exp(-1.f * gG1);
  col += 50.f * (0.001f * texture(__ctx.channel0, f32x2(0.0f, 0.5f)).data[0]);
  out_color.set_swizzle<0, 1, 2>(col);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
