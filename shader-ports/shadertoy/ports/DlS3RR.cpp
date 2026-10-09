// Port of https://www.shadertoy.com/view/DlS3RR; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlS3RR.json.
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
struct Context {
  TextureCube channel0;
  Texture3D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

f32x2x2 rotMat(Context &__ctx, float r) {
  return f32x2x2(cos(r), -sin(r), sin(r), cos(r));
}
f32x3 rotX(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rotY(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rotZ(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rot(Context &__ctx, f32x3 v, f32x3 k, float t) {
  k = normalize(k);
  return mix(k * dot(k, v), v, cos(t)) + cross(k, v) * sin(t);
}
f32x3 planeIntersect(Context &__ctx, f32x3 ro, f32x3 rd, float y, bool &hit) {
  hit = sign(-rd.data[1]) == sign(ro.data[1] - y);
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() +
                       rd.swizzle<0, 2>() * (y - ro.data[1]) / rd.data[1]);
  return f32x3(ro.data[0], y, ro.data[2]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) / iResolution.data[1];
  f32x2 m = (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * 0.5f) /
            iResolution.data[1] * step(0.01f, iMouse.data[2]);
  f32x2 rot = f32x2(3.14f * 0.5f, 0) - m.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 1, 0);
  f32x3 rd = rotY(__ctx, rotX(__ctx, normalize(f32x3(uv, 1)), rot.data[1]),
                  rot.data[0]);
  bool hit;
  f32x3 pli = planeIntersect(__ctx, ro, rd, 0.f, hit);
  f32x3 env =
      texture(
          __ctx.channel0,
          mix(reflect(rd, normalize(
                              f32x3(0, 1, 0) +
                              0.02f * (texture(__ctx.channel1,
                                               0.1f * (pli.swizzle<0, 1, 2>() +
                                                       iTime))
                                           .swizzle<0, 1, 2>() -
                                       0.5f))),
              rd, 1.f - float(hit)))
          .swizzle<0, 1, 2>();
  fragColor = f32x4(env, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_cube<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_3d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
