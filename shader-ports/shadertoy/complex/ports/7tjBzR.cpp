// Port of https://www.shadertoy.com/view/7tjBzR; author: Maadahmed.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tjBzR.json.
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

float vmax(Context &__ctx, f32x2 v) { return max(v.data[0], v.data[1]); }
float fBox2(Context &__ctx, f32x2 p, f32x2 b) {
  return vmax(__ctx, abs(p) - b);
}
f32x2x2 rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, s, -s, c);
}
float wf1(Context &__ctx, f32x2 p) { return sin(p.data[0]) + cos(p.data[1]); }
float cappedCylinder(Context &__ctx, f32x3 p, float h, float r) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 2>()), p.data[1])) - f32x2(r, h);
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
f32x3 gl = f32x3(0.f);
float gl1 = 0.f;
f32x3 gl2 = f32x3(0.f);
f32x3 map(Context &__ctx, f32x3 p) {
  f32x3 r = f32x3(0.f);
  f32x3 d = f32x3(0.f);
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot(__ctx, iTime * .5f)));
  f32x3 m = p;
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() *
                      (rot(__ctx, sin(-p.data[1] * .5f) * 1.1f)));
  p.set_swizzle<0, 2>(abs(p.swizzle<2, 0>()) - f32x2(.8f));
  float i = sin(p.data[1] * 3.f + iTime * 10.f) * .5f + .5f;
  float b = cappedCylinder(__ctx, p, 5.5f,
                           ((i - .5f) * 2.f * .3f) * cos(p.data[1] * .2f));
  gl += (.0004f / (.01f + b * b)) *
        mix(f32x3(1.f, 0.f, 1.f), f32x3(1.f, 1.f, 0.f), p.data[1]);
  r.data[0] = max(cappedCylinder(__ctx, p, 2.f, .3f + .2f * i),
                  -cappedCylinder(__ctx, p, 3.f, .2f + .25f * i));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() *
                      (rot(__ctx, p.data[1] * 3.f + iTime * 2.f)));
  f32x3 q = p;
  q.set_swizzle<0, 2>(q.swizzle<0, 2>() * (rot(__ctx, 3.14f / 2.f)));
  if (fBox2(__ctx, p.swizzle<0, 1>(), f32x2(.2f, 10.f)) < 0.f)
    r.set_swizzle<1, 2>(f32x2(3.f, 0.f));
  else if (fBox2(__ctx, q.swizzle<0, 1>(), f32x2(.2f, 10.f)) < 0.f)
    r.set_swizzle<1, 2>(f32x2(4.f, 0.f));
  else
    r.set_swizzle<1, 2>(f32x2(1.f));
  gl1 += (.000001f / (.000001f + pow(r.data[0] + .003f, 2.f)));
  d.data[0] = min(r.data[0], cappedCylinder(__ctx, p, 8.5f,
                                            (.25f + (i - .5f) * 2.f * .15f) *
                                                cos(p.data[1] * .2f)));
  d.data[1] = 2.f;
  if (r.data[0] > d.data[0])
    r = d;
  p = m;
  d.data[0] = length(p) - .45f - .1f * (sin(iTime * 10.f) * .5f + .5f);
  gl2 += (.0006f / (.01f + d.data[0] * d.data[0])) *
         mix(f32x3(1.f, 0.f, 1.f), f32x3(1.f, 1.f, 0.f), m.data[1]);
  if (r.data[0] > d.data[0])
    r = d;
  p = m;
  if (p.data[1] > 0.f)
    p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rot(__ctx, .3f)));
  p = abs(p);
  p.set_swizzle<2, 0>(p.swizzle<2, 0>() * (rot(__ctx, -3.14f / 4.f)));
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rot(__ctx, -3.14f / 4.f)));
  p.data[1] -= 1.f;
  q = p;
  p.set_swizzle<1, 0>(p.swizzle<1, 0>() *
                      (rot(__ctx, sin(p.data[1] * 3.14f) * .3f)));
  d.data[0] = cappedCylinder(__ctx, p, 1.f, (.06f + (i - .5f) * 2.f * .04f));
  p = q;
  p.data[1] -= 1.f;
  d.data[0] = min(d.data[0], length(p) - .15f -
                                 .05f * (sin(iTime * 10.f + 1.5f) * .5f + .5f));
  gl2 += (.0003f / (.01f + d.data[0] * d.data[0])) *
         mix(f32x3(1.f, 0.f, 1.f), f32x3(1.f, 1.f, 0.f), -m.data[1]);
  d.data[1] = 2.f;
  if (r.data[0] > d.data[0])
    r = d;
  return r;
}
const f32x2 e = f32x2(.00035f, -.00035f);
f32x3 norm(Context &__ctx, f32x3 po) {
  return normalize(
      e.swizzle<1, 1, 0>() * map(__ctx, po + e.swizzle<1, 1, 0>()).data[0] +
      e.swizzle<1, 0, 1>() * map(__ctx, po + e.swizzle<1, 0, 1>()).data[0] +
      e.swizzle<0, 1, 1>() * map(__ctx, po + e.swizzle<0, 1, 1>()).data[0] +
      e.swizzle<0, 0, 0>() * map(__ctx, po + e.swizzle<0, 0, 0>()).data[0]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x3 ro = f32x3(0.f, 3.f, -6.f), rd = normalize(f32x3(uv, 1.f)), p, h;
  rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() * (rot(__ctx, -.4f)));
  float t = 0.f;
  for (int i = 0; i < 120; i++) {
    p = ro + rd * t;
    h = map(__ctx, p);
    if (h.data[0] < .0001f || t > 40.f) {
      if (h.data[2] == 1.f)
        h.data[0] = abs(h.data[0]) + .0001f;
      else
        break;
    };
    t += h.data[0] * .7f;
  }
  f32x3 ld = f32x3(0.f, 1.f, 0.f);
  f32x3 ld1 = f32x3(3.f, 3.f, 0.f);
  ld1.set_swizzle<0, 2>(ld1.swizzle<0, 2>() * (rot(__ctx, iTime * .3f)));
  f32x3 col = f32x3(.1f);
  if (h.data[0] < .0001f) {
    if (h.data[1] == 1.f)
      col = f32x3(.1f, .3f, .2f);
    if (h.data[1] == 2.f)
      col = f32x3(.7f, .7f, .3f);
    if (h.data[1] == 3.f)
      col = f32x3(.5f, .9f, .5f);
    if (h.data[1] == 4.f)
      col = f32x3(.5f, .5f, .9f);
  }
  col = mix(col, f32x3(.1f, .3f, .2f), clamp(gl1, 0.f, 1.f));
  col += gl;
  col += gl2;
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
