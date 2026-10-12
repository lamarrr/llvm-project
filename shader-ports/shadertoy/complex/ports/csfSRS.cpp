// Port of https://www.shadertoy.com/view/csfSRS; author: fenix.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/csfSRS.json.
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
void mainImage(Context &__ctx, f32x4 &O, f32x2 U) {
  f32x2 R = iResolution.swizzle<0, 1>(),
        u = (3.f * iMouse.data[0] / R.data[0] + 1.f) * 9.f * (U - R / 2.f) /
            R.data[1],
        v = abs(u), V = mod(u += .5f, 2.f) - 1.f, B = min(v, v.data[1]) - .5f,
        t = iTime * .2f +
            f32x2(u.data[1] > 1.f && u.data[0] > 1.f ||
                  u.data[1] < 0.f && u.data[0] < 0.f) +
            (B.data[0] < 0.f ? u.data[1] > 1.f   ? 1.f
                               : u.data[0] > 1.f ? 2.f
                               : u.data[1] < 0.f ? 3.f
                                                 : 0.f
                             : 0.f),
        a = .13f - abs(.6f * smoothstep(1.25f, .75f,
                                        abs(mod(t, 4.f) - f32x2(2, 1) - .25f)) +
                       .2f - fract(u));
  t += B.data[0] < 0.f ? 1.f : 2.f;
  t = V * V.data[1] *
      max(min(a, a.data[1]),
          min(.13f -
                  abs(.6f * smoothstep(1.25f, .75f,
                                       abs(mod(t, 4.f) - f32x2(2, 1) - .25f)) +
                      .2f - fract(u)),
              .13f -
                  abs(.6f * smoothstep(1.25f, .75f,
                                       abs(mod(t, 4.f) - f32x2(2, 1) - .25f)) +
                      .2f - fract(u))
                      .data[1]));
  O = f32x4(max(v.data[0], v.data[1]) > .5f
                ? smoothstep(-.8f, .8f, t / fwidth(t)).data[0]
                : 0.f);
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
