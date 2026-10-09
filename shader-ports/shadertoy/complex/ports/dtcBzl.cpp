// Port of https://www.shadertoy.com/view/dtcBzl; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtcBzl.json.
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
}
f32x2x2 rotM(Context &__ctx, float r) {
  float c = cos(r), s = sin(r);
  return f32x2x2(c, s, -s, c);
}
f32x3 tmUnreal(Context &__ctx, f32x3 c) { return c / (c + .155f) * 1.019f; }
float aaStep(Context &__ctx, float fun) {
  return smoothstep(fwidth(fun), .0f, fun);
}
float glow(Context &__ctx, float sd, float amp) {
  return clamp(amp / max(amp, sd), .0f, 1.f);
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, .0f)) + min(max(d.data[0], d.data[1]), .0f);
}
f32x3 getCol(Context &__ctx, float id) {
  f32x3 setCol = f32x3(0);
  id = fract(id / 8.f) * 8.f;
  if (id < 1.f)
    setCol = f32x3(23, 123, 250);
  else if (id < 2.f)
    setCol = f32x3(30, 29, 215);
  else if (id < 3.f)
    setCol = f32x3(244, 0, 204);
  else if (id < 4.f)
    setCol = f32x3(131, 58, 187);
  else if (id < 5.f)
    setCol = f32x3(0, 250, 253);
  else if (id < 6.f)
    setCol = f32x3(66, 120, 91);
  else if (id < 7.f)
    setCol = f32x3(252, 157, 0);
  else if (id < 8.f)
    setCol = f32x3(231, 15, 20);
  setCol *=
      abs(cos(.06f * 150.f / 60.f * iTime +
              3.14159265359f / f32x3(.5f, 2.f, 4.f) + ffts.swizzle<0, 1, 2>()));
  return setCol / 256.f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  compressFft(__ctx);
  f32x2 uv0 = uv;
  float size = 1.f / exp(cos(150.f / 60.f * iTime / 8.f) * 2.f + 3.f);
  uv *= mix(1.f,
            .5f + 1.f * texelFetch(__ctx.channel0,
                                   i32x2(mod(dot(floor((uv) / size),
                                                 f32x2(1, 1.f / size)),
                                             512.f),
                                         0))
                            .data[0],
            fft.data[3] * fft.data[3] * fft.data[3]);
  float mySd = sdBox(__ctx, uv * rotM(__ctx, 3.14159265359f / 4.f),
                     f32x2(.66f * (.97f + .03f * fft.data[2])));
  f32x3 col =
      f32x3(glow(__ctx, mySd, .01f) - aaStep(__ctx, mySd)) *
      (1.3f - fft.data[1]) *
      texelFetch(__ctx.channel0, i32x2((floor(abs(uv.data[1]) * 22.f) +
                                        floor(abs(uv.data[0]) * 22.f) / 22.f) /
                                           22.f * 512.f,
                                       0))
          .data[0] *
      2.f;
  if (fft.data[0] < .94f)
    uv = uv0;
  mySd =
      length(uv) - .3f * (.4f + .6f * fft.data[0] * fft.data[0] * fft.data[0]);
  col += (glow(__ctx, mySd, .01f) - aaStep(__ctx, mySd)) *
         (.2f + .8f * fft.data[0]);
  float colId = floor(150.f / 60.f * iTime / 32.f) * 2.f;
  float amp = (aaStep(__ctx, mySd) * .93f + .07f);
  col += clamp(uv.data[1] + (fft.data[1] * fft.data[1] - .5f) * .2f, .0f, 1.f) *
         getCol(__ctx, colId + 0.f) * amp * ffts.data[3];
  col +=
      clamp(sign(fract(150.f / 60.f * iTime / 64.f - .25f) - .5f) * uv.data[0] +
                sin(150.f / 60.f * iTime / 64.f * 3.14159265359f * 2.f) * .6f +
                .3f,
            .0f, 1.f) *
      getCol(__ctx, colId + 1.f) * amp * fft.data[3];
  mySd = abs(mySd + .001f) - (1.f - uv.data[1] * uv.data[1] * 111.f) * .001f;
  col += col * glow(__ctx, mySd, .01f) * 9.f;
  if (fft.data[0] > .97f)
    col += col * fract(float(iFrame) * .5f) * 9.f;
  col -= length(uv) * .01f;
  col = tmUnreal(__ctx, col);
  fragColor = f32x4(col, 1.f);
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
