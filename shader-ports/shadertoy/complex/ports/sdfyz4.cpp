// Port of https://www.shadertoy.com/view/sdfyz4; author: gurkanizmirligil.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdfyz4.json.
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
float displacement(Context &__ctx, f32x3 p) {
  float stAnim = step(.7f, sin(iTime * .5f));
  return sin(1.f * p.data[0]) * sin(1.f * p.data[1]) *
         sin(.3f * p.data[2] + iTime * 2.0f - iTime * 1.5f * stAnim);
}
float sdPyramid(Context &__ctx, f32x3 p, float s) {
  float stAnim = step(.7f, sin(iTime * .5f));
  float angle =
      (100.f - 75.f * stAnim) * (sin(iTime * .1f + 3.14f) + iTime * .1f) + 7.f;
  float xx = p.data[0] * cos(angle) - p.data[2] * sin(angle);
  float zz = p.data[0] * sin(angle) + p.data[2] * cos(angle);
  p.set_swizzle<0, 2>(f32x2(xx, zz));
  p = abs(p);
  float m = p.data[0] + p.data[1] + p.data[2] - s;
  f32x3 q;
  if (3.0f * p.data[0] < m)
    q = p.swizzle<0, 1, 2>();
  else if (3.0f * p.data[1] < m)
    q = p.swizzle<1, 2, 0>();
  else if (3.0f * p.data[2] < m)
    q = p.swizzle<2, 0, 1>();
  else
    return m * 0.57735027f;
  float k = clamp(0.5f * (q.data[2] - q.data[1] + s), 0.0f, s);
  return length(f32x3(q.data[0], q.data[1] - s + k, q.data[2] - k));
}
float sdPlane(Context &__ctx, f32x3 p, f32x3 n, float h) {
  return dot(p, n) + h + displacement(__ctx, p) * .2f;
}
float sdBoxFrame(Context &__ctx, f32x3 p, f32x3 b, float e) {
  float stAnim = step(.7f, sin(iTime * .5f));
  p = abs(p) - b;
  f32x3 q = abs(p + e) - e;
  float bf = min(
      min(length(max(f32x3(p.data[0], q.data[1], q.data[2]), 0.0f)) +
              min(max(p.data[0], max(q.data[1], q.data[2])), 0.0f),
          length(max(f32x3(q.data[0], p.data[1], q.data[2]), 0.5f * stAnim)) +
              min(max(q.data[0], max(p.data[1], q.data[2])), 0.0f)),
      length(max(f32x3(q.data[0], q.data[1], p.data[2]), 0.5f)) +
          min(max(q.data[0], max(q.data[1], p.data[2])), 0.0f));
  return bf;
}
float opSmoothUnion(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float opRep(Context &__ctx, f32x3 p, f32x3 c) {
  float stAnim = step(.7f, sin(iTime * .5f));
  f32x3 q = mod(p + 0.5f * c, c) - 0.5f * c;
  return min(sdBoxFrame(__ctx, q, f32x3(1.5f), .2f),
             sdPyramid(__ctx, q + f32x3(0, .3f, 0), .4f));
}
float GetDist(Context &__ctx, f32x3 p) {
  float stAnim = step(.7f, sin(iTime * .5f));
  float ground =
      sdPlane(__ctx, p, f32x3(0.0f, 1.0f, 0.0f), -4.2f + stAnim * 4545.45f);
  float bf = opRep(__ctx, p, f32x3(4.75f + stAnim * 5.f));
  float gr = opSmoothUnion(__ctx, bf, ground, .5f);
  float d = gr;
  return d;
}
float raymarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float d0 = 0.0f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + d0 * rd;
    float dS = GetDist(__ctx, p);
    d0 += dS;
    if (dS < .01f || d0 > 50.0f)
      break;
  }
  return d0;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(0.01f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
float GetLight(Context &__ctx, f32x3 p, f32x3 ld) {
  f32x3 lightDir = ld;
  f32x3 l = normalize(lightDir);
  f32x3 n = GetNormal(__ctx, p);
  float dif = clamp(dot(n, l), 0.f, 1.0f);
  f32x3 l2 = f32x3(-l.data[0], l.swizzle<1, 2>());
  float dif2 = clamp(dot(n, l2), 0.f, 1.0f);
  dif = mix(dif, dif2, .5f);
  float lightDistance = 5.0f;
  return dif;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float stAnim = step(.7f, sin(iTime * .5f));
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 bg =
      f32x3(mix(.9f, 0.f, uv.data[1]), mix(0.2f, 0.0f, uv.data[1]), 0.0f);
  if (stAnim > .5f)
    bg = f32x3(1.f, .99f, .95f);
  f32x3 ro = f32x3(0, 5.f, -5) - f32x3(0, 0, iTime - iTime * .75f * stAnim) *
                                     ((-stAnim * 1.6f) + 1.f);
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1], 1));
  float d = raymarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  float anim =
      clamp(abs(sin(sin(iTime))) / (10.f * sin(iTime * 3.f) * sin(iTime * 3.f)),
            .5f, 4.f);
  float dif =
      GetLight(__ctx, p, f32x3(10.f * sin(iTime * .1f) - 5.f, 5.f, 5.f));
  float dif2 = GetLight(__ctx, p, f32x3(-15.f, 1.f + stAnim * 10.f, -5.f));
  dif = mix(dif * anim, dif2, stAnim);
  float m = clamp(d / 50.0f, 0.f, 1.f);
  f32x3 mat = f32x3(100.f, 0, 0.f) / 255.f;
  if (stAnim > .5f)
    mat = f32x3(1.5f, 1.5f, 1.5f);
  f32x3 s = f32x3(dif) * mat;
  f32x3 col = mix(s, bg, m);
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
