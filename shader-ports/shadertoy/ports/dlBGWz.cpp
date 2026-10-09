// Port of https://www.shadertoy.com/view/dlBGWz; author: ManuManu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlBGWz.json.
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
uniform float time;
uniform f32x2 mouse;
uniform f32x2 resolution;
f32x3 pin(Context &__ctx, f32x3 v) {
  f32x3 q = f32x3(0.0f);
  q.data[0] = sin(v.data[0]) * 0.5f + 0.5f;
  q.data[1] = sin(v.data[1] + 1.0471975511965977461542144610932f) * 0.5f + 0.5f;
  q.data[2] = sin(v.data[2] + 4.1887902047863909846168473723972f) * 0.5f + 0.5f;
  return normalize(q);
}
f32x3 spin(Context &__ctx, f32x3 v) {
  for (int i = 0; i < 3; i++) {
    v = pin(__ctx, v.swizzle<1, 2, 0>() * 6.283185307179586476925286766559f);
  }
  return v.swizzle<2, 0, 1>();
}
float map(Context &__ctx, f32x3 p) {
  f32x3 val = spin(__ctx, p);
  float k = val.data[0] + val.data[1] + val.data[2];
  return (cos(p.data[0]) + cos(p.data[1] * 0.75f) + sin(p.data[2]) * 0.25f) +
         k * 1.2f;
}
f32x2 rot(Context &__ctx, f32x2 r, float a) {
  return f32x2(cos(a) * r.data[0] - sin(a) * r.data[1],
               sin(a) * r.data[0] + cos(a) * r.data[1]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (gl_FragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) * 2.0f -
      1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 dir = normalize(f32x3(uv, 1.0f));
  dir.set_swizzle<2, 1>(rot(__ctx, dir.swizzle<2, 1>(), iTime * 0.2f));
  dir.set_swizzle<0, 2>(rot(__ctx, dir.swizzle<0, 2>(), iTime * 0.1f));
  dir = dir.swizzle<1, 2, 0>();
  f32x3 pos = f32x3(0, 0, iTime * 2.0f);
  float t = 0.0f;
  for (int i = 0; i < 200; i++) {
    float temp = map(__ctx, pos + dir * t) * 0.55f;
    if (temp < .1f)
      break;
    t += temp;
    dir.set_swizzle<0, 1>(rot(__ctx, dir.swizzle<0, 1>(), temp * 0.05f));
    dir.set_swizzle<1, 2>(rot(__ctx, dir.swizzle<1, 2>(), temp * 0.01f));
    dir.set_swizzle<2, 0>(rot(__ctx, dir.swizzle<2, 0>(), temp * 0.09f));
  }
  f32x3 ip = pos + dir * t;
  f32x3 baseColor = f32x3(0.1f, 0.2f, 0.9f);
  fragColor =
      f32x4(f32x3(max(0.01f, map(__ctx, ip + 0.2f)) + t * 0.12f) * baseColor +
                0.35f * (dir * spin(__ctx, ip)),
            1.0f);
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
