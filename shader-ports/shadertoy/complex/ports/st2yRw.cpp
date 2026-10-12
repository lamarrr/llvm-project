// Port of https://www.shadertoy.com/view/st2yRw; author: jarble.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/st2yRw.json.
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
f32x2 fract1(Context &__ctx, f32x2 a) { return fract(a); }
f32x2 rotate(Context &__ctx, f32x2 v, float a) {
  float s = sin(a);
  float c = cos(a);
  return f32x2x2(c, -s, s, c) * v;
}
f32x2 triangle_wave(Context &__ctx, f32x2 a, float num) {
  a = rotate(__ctx, a, num * radians(180.f));
  return abs(fract1(__ctx, (a + f32x2(1.f, 0.5f)) * 1.5f) - .5f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = f32x4(0.0f);
  f32x3 col = f32x3(0.f);
  float t1 = 8.f;
  f32x2 uv = (fragCoord) / iResolution.data[1] / t1 / 2.0f;
  float time1 = iTime / 64.f;
  uv += f32x2(time1 / 2.0f, time1 / 3.0f) / t1 / 4.0f +
        (f32x2(cos(time1), sin(time1))) * 8.f / t1;
  float scale = 1.5f;
  float p1 = 1.f;
  f32x2 t2 = f32x2(0.f);
  f32x2 t3 = f32x2(0.f);
  f32x3 col1 = col;
  for (int k = 0; k < 9; k++) {
    float rotation_number = 0.f;
    uv += t2;
    uv /= scale;
    t2 = p1 * triangle_wave(__ctx, uv - .5f + floor(t2.data[1] + t2.data[0]),
                            rotation_number);
    t3 = -p1 * triangle_wave(
                   __ctx, uv.swizzle<1, 0>() + floor(t2.data[1] + t2.data[0]),
                   rotation_number);
    uv = t2 - t3;
    p1 *= -1.f;
    float multiplier = 1.25f;
    col.data[0] = max(uv.data[1] + uv.data[0] - col.data[0] * 2.f,
                      col.data[0] * (multiplier));
    col = abs(col.swizzle<1, 2, 0>() - f32x3(1.f - col.data[0])) / multiplier;
    col1 = abs(col - col1);
  }
  fragColor = f32x4(col * 2.f, 1.0f);
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
