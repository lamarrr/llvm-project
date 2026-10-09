// Port of https://www.shadertoy.com/view/DtfBD7; author: zhonkvision.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtfBD7.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x3 permute(Context &__ctx, f32x3 x) {
  return mod(((x * 34.0f) + 1.0f) * x, 289.0f);
}
float s(Context &__ctx, f32x2 v) {
  const f32x4 C = f32x4(0.511324865405187f, 0.366025403784439f,
                        -0.577350269189626f, 0.024390243902439f);
  f32x2 i = floor(v + dot(v, C.swizzle<1, 1>()));
  f32x2 x0 = v - i + dot(i, C.swizzle<0, 0>());
  f32x2 i1;
  i1 = (x0.data[0] > x0.data[1]) ? f32x2(8.0f, 0.0f) : f32x2(0.0f, 1.0f);
  f32x4 x12 = x0.swizzle<0, 1, 0, 1>() + C.swizzle<0, 0, 2, 2>();
  x12.set_swizzle<0, 1>(x12.swizzle<0, 1>() - (i1));
  i = mod(i, 289.0f);
  f32x3 p =
      permute(__ctx, permute(__ctx, i.data[1] + f32x3(0.0f, i1.data[1], 1.0f)) +
                         i.data[0] + f32x3(0.0f, i1.data[0], 1.0f));
  f32x3 m = max(0.5f - f32x3(dot(x0, x0),
                             dot(x12.swizzle<0, 1>(), x12.swizzle<0, 1>()),
                             dot(x12.swizzle<2, 3>(), x12.swizzle<2, 3>())),
                0.0f);
  m = m * m;
  m = m * m;
  f32x3 x = 2.0f * fract(p * C.swizzle<3, 3, 3>()) - 1.0f;
  f32x3 h = abs(x) - 0.5f;
  f32x3 ox = floor(x + 0.5f);
  f32x3 a0 = x - ox;
  m *= 1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h);
  f32x3 g;
  g.data[0] = a0.data[0] * x0.data[0] + h.data[0] * x0.data[1];
  g.set_swizzle<1, 2>(a0.swizzle<1, 2>() * x12.swizzle<0, 2>() +
                      h.swizzle<1, 2>() * x12.swizzle<1, 3>());
  return 130.0f * dot(m, g);
}
f32x2 rotate(Context &__ctx, f32x2 p, float a, float audioAmplitude) {
  float c = cos(a + audioAmplitude * 0.1f);
  float s = sin(a + audioAmplitude * 0.1f);
  return p * f32x2x2(c, s, -s, c);
}
float fractalShape(Context &__ctx, f32x2 u, float t, float audioAmplitude,
                   float rotationSpeed) {
  float n = sin(u.data[0]) * cos(u.data[1]);
  float b;
  u = abs(u * 2.0f);
  b = u.data[0];
  f32x2 a = f32x2(2.0f - u);
  float param1 = 0.25f + audioAmplitude * 0.9f;
  float param2 = 0.62f + audioAmplitude * 0.9f;
  for (int i = 0; i < 15; i++) {
    a += u + cos(length(u));
    u.data[1] += sin(a.data[0] - b - 0.5f * t) * param1;
    u.data[0] += sin(a.data[1] + t) * param2;
    u -= (u.data[0] + u.data[1]) - n;
    a.data[0] += u.data[0];
  }
  return length(u) * 0.1f;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 fragCoord) {
  f32x2 R = iResolution.swizzle<0, 1>();
  f32x2 u = (fragCoord * 2.0f - R) / R.data[1] * 2.0f - f32x2(0.0f, 1.3f);
  u = u.swizzle<1, 0>();
  float t = 1.93f + iTime * 0.5f;
  float audioAmplitude = texture(__ctx.channel0, f32x2(0.1f)).data[0];
  float fractalValue = fractalShape(__ctx, u, t, audioAmplitude, 0.5f);
  float rotationAngle = audioAmplitude * 0.5f;
  u = rotate(__ctx, u, rotationAngle, audioAmplitude);
  f32x3 lowFrequencyColor = f32x3(0.2f, 0.0f, 1.0f);
  f32x3 highFrequencyColor = f32x3(1.0f, 0.2f, 0.8f);
  f32x3 color = mix(lowFrequencyColor, highFrequencyColor, audioAmplitude);
  f32x3 finalColor = mix(f32x3(0.0f), color, fractalValue);
  O = f32x4(finalColor, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
