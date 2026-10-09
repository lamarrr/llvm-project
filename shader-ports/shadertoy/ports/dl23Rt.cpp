// Port of https://www.shadertoy.com/view/dl23Rt; author: maxwellcoffee.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dl23Rt.json.
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
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  float c, o, e = 2.84f + iTime * .021111111f;
  f32x2 r = iResolution.swizzle<0, 1>();
  for (; o++ < 1e2f;) {
    f32x3 p = abs(.7f * c * normalize(f32x3((F + F - r) / r.data[1], 1)));
    p.set_swizzle<0, 1>(p.swizzle<0, 1>() *
                        (f32x2x2(tan(e * f32x4(1, 3, 1, 1)))));
    p.set_swizzle<2, 1>(p.swizzle<2, 1>() + (e + c * cos(6.f * e)));
    p = fract(p) - .15f;
    p.set_swizzle<0, 1>(p.swizzle<0, 1>() *
                        (f32x2x2(tan(c * f32x4(1, 3, 1, 1)))));
    p.set_swizzle<0, 2>(p.swizzle<0, 2>() *
                        (f32x2x2(tan(e * f32x4(1, 3, 1, 1)))));
    p.data[1] = max(abs(p.data[1]) - mix(0.f, .2f, 1.f * 1.f + 4.5f), 0.f);
    c += (length(f32x2(length(p.swizzle<0, 1>()) - .2f, p.data[2])) -
          mix(.14f, .31f, .25f * .25f + 4.5f) - c * .01f) *
         .5f;
  }
  O.set_swizzle<0, 1, 2>(
      1.2f * (tan(c * 110.f + .99f * f32x3(0, 1.f + c * .2f, 2)) + .2f) /
      exp(c * 0.0914f));
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
