// Port of https://www.shadertoy.com/view/cdcBWs; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cdcBWs.json.
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
f32x4 fft, ffts;
void compressFft(Context &__ctx) {
  fft = f32x4(0), ffts = f32x4(0);
  for (int n = 0; n < 3; n++)
    fft.data[0] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  for (int n = 6; n < 8; n++)
    ffts.data[0] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  for (int n = 8; n < 14; n += 2)
    ffts.data[1] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  for (int n = 14; n < 24; n += 4)
    ffts.data[2] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  for (int n = 24; n < 95; n += 10)
    fft.data[2] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  for (int n = 95; n < 512; n += 100)
    fft.data[3] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  fft.data[1] = dot(ffts.swizzle<0, 1, 2>(), f32x3(1));
  ffts.data[3] = dot(fft.swizzle<0, 1, 2, 3>(), f32x4(1));
  fft /= f32x4(3, 8, 8, 5);
  ffts /= f32x4(2, 3, 3, 23);
  if (false)
    for (int n = 0; n++ < 4;)
      fft[n] *= 1.f + .3f * pow(fft[n], 5.f);
  fft = clamp(fft, .0f, 1.f);
}
f32x3 getCol(Context &__ctx, float id) {
  f32x3 setCol = f32x3(0);
  id = mod(id, 15.f);
  if (id < 1.f)
    setCol = f32x3(244, 0, 204);
  else if (id < 2.f)
    setCol = f32x3(0, 250, 253);
  else if (id < 3.f)
    setCol = f32x3(30, 29, 215);
  else if (id < 4.f)
    setCol = f32x3(252, 157, 0);
  else if (id < 5.f)
    setCol = f32x3(26, 246, 138);
  else if (id < 6.f)
    setCol = f32x3(131, 58, 187);
  else if (id < 7.f)
    setCol = f32x3(231, 15, 20);
  else if (id < 8.f)
    setCol = f32x3(35, 87, 97);
  else if (id < 9.f)
    setCol = f32x3(103, 211, 225);
  else if (id < 10.f)
    setCol = f32x3(241, 204, 9);
  else if (id < 11.f)
    setCol = f32x3(22, 242, 124);
  else if (id < 12.f)
    setCol = f32x3(30, 248, 236);
  else if (id < 13.f)
    setCol = f32x3(173, 0, 27);
  else if (id < 14.f)
    setCol = f32x3(28, 142, 77);
  else if (id < 15.f)
    setCol = f32x3(66, 120, 91);
  return setCol / 256.f;
}
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, .0f)) + min(max(d.data[0], d.data[1]), .0f);
}
float sdEquilateralTriangle(Context &__ctx, f32x2 p) {
  const float k = sqrt(3.f);
  p.data[0] = abs(p.data[0]) - 1.f;
  p.data[1] = p.data[1] + 1.f / k;
  if (p.data[0] + k * p.data[1] > 0.f)
    p = f32x2(p.data[0] - k * p.data[1], -k * p.data[0] - p.data[1]) / 2.f;
  p.data[0] -= clamp(p.data[0], -2.f, 0.f);
  return -length(p) * sign(p.data[1]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  compressFft(__ctx);
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  uv *= .6f +
        .5f * texelFetch(
                  __ctx.channel0,
                  i32x2(dot(floor((uv + 1.f) / .1f), f32x2(1, 10)) + 24.f, 0))
                  .data[0];
  float widthF = (false) ? 9.f : 1.f;
  f32x3 r3 = f32x3(texelFetch(__ctx.channel0, i32x2(0, 0)).data[0],
                   texelFetch(__ctx.channel0, i32x2(1, 0)).data[0],
                   texelFetch(__ctx.channel0, i32x2(2, 0)).data[0]);
  f32x3 col =
      (.3f + .5f * pow(fft.data[0], 9.f)) *
      (r3 * r3 * r3 * r3 * r3 * f32x3(3, 1, 5) * .005f + .01f) * widthF /
      abs(f32x3(sdCircle(__ctx, uv, .27f * r3.data[0]),
                sdEquilateralTriangle(__ctx, uv / (.1f / r3.data[1] + .001f)) *
                    .1f / r3.data[1],
                sdBox(__ctx, uv, f32x2(.35f * r3.data[2]))));
  uv /= (.3f + .7f * pow(fft.data[3], .5f));
  col =
      (col * 4.f + .6f) * f32x3(.5f * fft.data[2] + .2f) *
      texelFetch(__ctx.channel0, i32x2((floor(abs(uv.data[1]) * 22.f) +
                                        floor(abs(uv.data[0]) * 22.f) / 22.f) /
                                           22.f * 512.f,
                                       0))
          .data[0];
  float colId = 3.f * floor(mod(2.5f * iTime / 16.f, 5.f));
  col = f32x3x3(getCol(__ctx, colId + 0.f), getCol(__ctx, colId + 1.f),
                getCol(__ctx, colId + 2.f)) *
        col;
  col *= abs(cos(.06f * iTime + 3.14159265359f / f32x3(.5f, 2.f, 4.f) +
                 ffts.swizzle<0, 1, 2>() * 3.14159265359f));
  fragColor = f32x4(col, 1.f);
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
