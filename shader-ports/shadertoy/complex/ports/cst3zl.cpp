// Port of https://www.shadertoy.com/view/cst3zl; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cst3zl.json.
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
float animTime;
f32x4 fft, ffts;
void compressFft(Context &__ctx) {
  fft = f32x4(0), ffts = f32x4(0);
  for (int n = 1; n < 3; n++)
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
  fft /= f32x4(2, 8, 7, 4);
  ffts /= f32x4(2, 3, 3, 21);
  fft.data[0] = step(.91f, fft.data[0]);
}
float hash21(Context &__ctx, f32x2 p) {
  p = fract(p * f32x2(13.81f, 741.76f));
  p += dot(p, p + 42.23f);
  return fract(p.data[0] * p.data[1]);
}
f32x2x2 rotM(Context &__ctx, float deg) {
  deg /= 180.f / 3.14159265359f;
  return f32x2x2(cos(deg), -sin(deg), sin(deg), cos(deg));
}
float particle(Context &__ctx, f32x2 p) {
  return smoothstep(.1f, .0f, length(p)) *
         smoothstep(.1f, .06f, length(p - f32x2(0.f, .02f)));
}
float dustLayer(Context &__ctx, f32x2 p) {
  float id = hash21(__ctx, floor(p));
  return smoothstep(0.f, 1.f, id) *
         particle(__ctx,
                  (fract(p) - f32x2(.5f + .4f * cos(id * animTime),
                                    .5f + .4f * sin(.8f * id * animTime))) *
                      rotM(__ctx, id * 360.f) /
                      f32x2(cos(.5f * id * animTime), 1));
}
f32x3 Field(Context &__ctx, f32x3 Pos) {
  Pos *= 0.1f;
  float f = 0.1f;
  for (int i = 0; i < 5; ++i) {
    Pos = (Pos.swizzle<1, 2, 0>() *
           f32x3x3(0.8f, 0.6f, 0, -0.6f, 0.8f, 0.0f, 0.0f, 0.0f, 1.0f)) +
          f32x3(0.123f, 0.456f, 0.789f) * float(i);
    Pos = (abs(fract(Pos) - 0.5f)) * 2.0f;
    f *= 2.0f;
  }
  Pos *= Pos;
  return sqrt(Pos + Pos.swizzle<1, 2, 0>()) / f + 0.0001f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  float d2 = uv.data[0] * uv.data[0] + uv.data[1] * uv.data[1];
  animTime = 2.133333f * iTime;
  f32x3 col;
  float aFrac, amp = 0.f;
  compressFft(__ctx);
  f32x3 Position =
      f32x3(0.5f, 0.8f,
            .5f * animTime + .4f * fft.data[3] * fft.data[3] * fft.data[3]);
  f32x3 Direction = f32x3((1.f - .5f * cos(animTime / 16.f)) * uv, 1.f);
  for (float n = 0.f; n < 4.f; n++) {
    aFrac = fract(-.05f * animTime + .25f * n) -
            .03f * fft.data[3] * fft.data[3] * fft.data[3];
    amp += 1.4f * (.2f + .8f * fft.data[2]) *
           dustLayer(__ctx, (uv + n * f32x2(.1f, .05f)) * 25.f * aFrac) *
           smoothstep(1.f, .33f, aFrac);
  }
  amp *= (.7f + .5f * length(uv));
  for (int i = 0; i < 50; ++i) {
    f32x3 f2 = Field(__ctx, Position);
    Position += Direction * min(min(f2.data[0], f2.data[1]), f2.data[2]);
    col += float(50 - i) / (f2 + 0.005f);
  }
  col = f32x3(1.0f - 1.0f / (1.0f + col * (-0.06f / 2500.0f)));
  col *= col;
  col = (.5f * amp + 3.f * col.data[0]) * f32x3(ffts.data[0] <= ffts.data[1],
                                                ffts.data[1] <= ffts.data[2],
                                                ffts.data[2] <= ffts.data[0]);
  col += .1f *
         f32x3(clamp(.015f / abs(d2 - .04f * (.5f + ffts.data[3])) *
                         (.1f + .9f * fft.data[0]),
                     0.f, 1.f)) /
         (length(col));
  fragColor = f32x4(col, 1.0f);
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
