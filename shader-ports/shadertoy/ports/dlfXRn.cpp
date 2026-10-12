// Port of https://www.shadertoy.com/view/dlfXRn; author: KoharuXenon.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlfXRn.json.
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
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

float warp = 0.75f;
float zoom_factor;
float abberationoffset = 0.1f;
void mainImage(Context &__ctx, f32x4 &O, f32x2 funni) {
  warp = sin(iTime);
  zoom_factor = 1.f + ((warp / 15.f) * -1.f);
  f32x2 p = funni.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x4 toUse = f32x4(texture(__ctx.channel0, p));
  f32x2 uv = p;
  f32x2 redoffsetfunniredamgus = uv;
  f32x2 blueoffsetlikedeeznuts = uv;
  f32x2 dc = abs(0.5f - uv);
  dc *= dc;
  uv.data[0] -= 0.5f;
  uv.data[0] *= 1.0f + (dc.data[1] * (0.3f * warp));
  uv.data[0] += 0.5f;
  uv.data[1] -= 0.5f;
  uv.data[1] *= 1.0f + (dc.data[0] * (0.2f * warp));
  uv.data[1] += 0.5f;
  redoffsetfunniredamgus.data[0] -= 0.5f;
  redoffsetfunniredamgus.data[0] *=
      1.0f + (dc.data[1] * ((0.3f + abberationoffset) * warp));
  redoffsetfunniredamgus.data[0] += 0.5f;
  redoffsetfunniredamgus.data[1] -= 0.5f;
  redoffsetfunniredamgus.data[1] *=
      1.0f + (dc.data[0] * ((0.2f + abberationoffset) * warp));
  redoffsetfunniredamgus.data[1] += 0.5f;
  blueoffsetlikedeeznuts.data[0] -= 0.5f;
  blueoffsetlikedeeznuts.data[0] *=
      1.0f + (dc.data[1] * ((0.3f - abberationoffset) * warp));
  blueoffsetlikedeeznuts.data[0] += 0.5f;
  blueoffsetlikedeeznuts.data[1] -= 0.5f;
  blueoffsetlikedeeznuts.data[1] *=
      1.0f + (dc.data[0] * ((0.2f - abberationoffset) * warp));
  blueoffsetlikedeeznuts.data[1] += 0.5f;
  uv = uv + (uv - f32x2(0.5f, 0.5f)) * (zoom_factor - 1.0f);
  redoffsetfunniredamgus =
      redoffsetfunniredamgus +
      (redoffsetfunniredamgus - f32x2(0.5f, 0.5f)) * (zoom_factor - 1.0f);
  blueoffsetlikedeeznuts =
      blueoffsetlikedeeznuts +
      (blueoffsetlikedeeznuts - f32x2(0.5f, 0.5f)) * (zoom_factor - 1.0f);
  if (uv.data[1] > 1.0f || uv.data[0] < 0.0f || uv.data[0] > 1.0f ||
      uv.data[1] < 0.0f)
    O = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
  else {
    f32x4 col1 = f32x4(texture(__ctx.channel0, uv));
    f32x4 col2 = f32x4(texture(__ctx.channel0, redoffsetfunniredamgus));
    f32x4 col3 = f32x4(texture(__ctx.channel0, blueoffsetlikedeeznuts));
    O = f32x4(col2.data[0], col1.data[1], col3.data[2], col1.data[3]);
  }
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
