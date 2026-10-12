// Port of https://www.shadertoy.com/view/DtS3zc; author: shaofun.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtS3zc.json.
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

float copysignf(Context &__ctx, float x, float s) {
  unsigned uabs = float_bits_to_uint(x) & 0x7fffffffU;
  unsigned usig = float_bits_to_uint(s) & 0x80000000U;
  return uint_bits_to_float(uabs | usig);
}
float mulsignf(Context &__ctx, float x, float s) {
  unsigned u = float_bits_to_uint(s) & 0x80000000U;
  u ^= float_bits_to_uint(x);
  return uint_bits_to_float(u);
}
float signed1(Context &__ctx, float x) { return copysignf(__ctx, 1.0f, x); }
float signed2(Context &__ctx, float x) { return x >= 0.0f ? 1.0f : -1.0f; }
float signed3(Context &__ctx, float x) { return sign(sign(x) + 0.5f); }
float signed4(Context &__ctx, float x) {
  return float(x >= 0.0f) - float(x < 0.0f);
}
float signed5(Context &__ctx, float x) {
  float t = float(x >= 0.0f);
  return float(t + t - 1.0f);
}
float signed6(Context &__ctx, float x) {
  unsigned ix = (float_bits_to_uint(x) & 0x80000000U);
  ix |= 0x3f800000U;
  return uint_bits_to_float(ix);
}
float signed7(Context &__ctx, float x) { return mulsignf(__ctx, 1.0f, x); }
float run(Context &__ctx, float x) {
  return float(signed7(__ctx, x - 0.5f) == sign(sin(iTime * 5.0f)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 col;
  uv -= 0.5f;
  uv *= 5.0f;
  col = f32x3(run(__ctx, max(floor(abs(uv.data[0])), floor(abs(uv.data[1])))));
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
