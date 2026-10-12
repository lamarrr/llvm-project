// Port of https://www.shadertoy.com/view/dllSW4; author: zero_ou.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dllSW4.json.
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
f32x3 circ(Context &__ctx, f32x2 uv, f32x2 pos, float r, float blend, f32x3 c) {
  uv -= pos;
  float d = length(uv);
  float mx = smoothstep(r, r - blend, d);
  return mx * c;
}
float band(Context &__ctx, float t, float start, float end, float blur) {
  float step1 = smoothstep(start - blur, start + blur, t);
  float step2 = smoothstep(end + blur, end - blur, t);
  return step1 * step2;
}
float rect(Context &__ctx, f32x2 uv, f32x2 p, float w, float h, float blur) {
  uv -= p;
  float left = 0.0f;
  float right = w;
  float bottom = 0.0f;
  float top = h;
  float lr_band = band(__ctx, uv.data[0], left, right, blur);
  float bt_band = band(__ctx, uv.data[1], bottom, top, blur);
  return lr_band * bt_band;
}
float rect(Context &__ctx, f32x2 uv, float left, float right, float bottom,
           float top, float blur) {
  float lr_band = band(__ctx, uv.data[0], left, right, blur);
  float bt_band = band(__ctx, uv.data[1], bottom, top, blur);
  return lr_band * bt_band;
}
float remap01(Context &__ctx, f32x2 r, float t) {
  return (t - r.data[0]) / (r.data[1] - r.data[0]);
}
float remap(Context &__ctx, f32x2 r1, f32x2 r2, float t) {
  return remap01(__ctx, r1, t) * (r2.data[1] - r2.data[0]) + r2.data[0];
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv -= 0.5f;
  uv *= 2.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 red = f32x3(1.0f, 0.0f, 0.0f);
  f32x3 green = f32x3(0.0f, 1.0f, 0.0f);
  f32x3 blue = f32x3(0.0f, 0.0f, 1.0f);
  f32x3 col = f32x3(0.0f);
  float x = uv.data[0];
  float m = -0.5f * sin(2.0f * 3.14159f * (x + iTime));
  float y = uv.data[1] + m;
  f32x2 rect_x = f32x2(-2.0f, 2.0f);
  float blur = remap(__ctx, rect_x, f32x2(0.01f, 0.25f), x);
  blur = pow(blur * 4.6f, 3.0f);
  col += blue * rect(__ctx, f32x2(x, y), f32x2(rect_x.data[0], -0.5f),
                     rect_x.data[1] - rect_x.data[0], 1.0f, blur);
  col -= blue * rect(__ctx, f32x2(x, y), -0.5f, 0.5f, -0.1f, 0.1f, 0.01f);
  float val = band(__ctx, uv.data[0], -0.2f, 0.2f, 0.01f) *
              band(__ctx, uv.data[1], -0.2f, 0.2f, 0.01f);
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
