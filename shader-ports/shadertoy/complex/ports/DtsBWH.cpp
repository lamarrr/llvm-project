// Port of https://www.shadertoy.com/view/DtsBWH; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtsBWH.json.
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
f32x2x2 rotM(Context &__ctx, float r) {
  float c = cos(r), s = sin(r);
  return f32x2x2(c, s, -s, c);
}
float hash21(Context &__ctx, f32x2 p) {
  p = fract(p * f32x2(13.81f, 741.76f));
  p += dot(p, p + 42.23f);
  return fract(p.data[0] * p.data[1]);
}
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
float particle(Context &__ctx, f32x2 p) {
  return smoothstep(.1f, .0f, length(p)) *
         smoothstep(.1f, .06f, length(p - f32x2(0.f, .02f)));
}
float particleLayer(Context &__ctx, f32x2 p) {
  float id = hash21(__ctx, floor(p));
  return smoothstep(0.f, 1.f, id) *
         particle(__ctx,
                  (fract(p) - f32x2(.5f + .4f * cos(id * iTime),
                                    .5f + .4f * sin(.8f * id * iTime))) *
                      rotM(__ctx, (id - fft.data[0]) * 2.f * 3.14159265359f) /
                      f32x2(cos(.5f * id * iTime), 1));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  compressFft(__ctx);
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  f32x3 col = f32x3(0);
  f32x3 p, camPos = f32x3(0, 0, -1.3f + (.3f * sin(2.5f * iTime / 16.f)));
  float v1, v2, a = 11.f;
  for (float n = 1.f; n < a; n++) {
    v1 =
        2.5f * iTime + n / a * 3.14159265359f * 4.f - fft.data[0] * n / a * 1.f;
    v2 = iTime + n / a * 3.14159265359f +
         fft.data[1] * mod(1.f - n * 2.f / a, 2.f) * 1.f;
    p = f32x3(cos(v1) * cos(v2), sin(v1) * cos(v2), sin(v2)) * .5f *
        max(ffts.data[3], fft.data[0]);
    p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotM(__ctx, n)));
    col +=
        1.f /
        ((p.data[2] - camPos.data[2]) * (p.data[2] - camPos.data[2]) +
         dot(p.swizzle<0, 1>(), p.swizzle<0, 1>())) *
        .001f * (.8f + 1.f * fft.data[0] * fft.data[0]) /
        max(.001f, length(uv - camPos.swizzle<0, 1>() -
                          p.swizzle<0, 1>() / (p.data[2] - camPos.data[2])) -
                       .02f / (p.data[2] - camPos.data[2])) *
        (.5f +
         clamp(.01f / max(.001f, length(uv - camPos.swizzle<0, 1>() -
                                        p.swizzle<0, 1>() /
                                            (p.data[2] - camPos.data[2]) +
                                        .005f * normalize(p.swizzle<0, 1>()))),
               .0f, .9f)) *
        (f32x3(mod(n + .5f, 2.f), mod(n, 2.f), mod(n * 3.14159265359f, 2.f)) *
             ffts.swizzle<0, 1, 2>() * .5f +
         .5f * f32x3(ffts.data[0] <= ffts.data[1], ffts.data[1] <= ffts.data[2],
                     ffts.data[2] <= ffts.data[0]));
  }
  uv *= rotM(__ctx, iTime * .1f - .5f * length(uv));
  float aFrac, amp = 0.f;
  for (float n = 0.f; n < 4.f; n++) {
    aFrac = fract(-.05f * iTime + .25f * n) -
            .02f * fft.data[3] * fft.data[3] * fft.data[3];
    amp += 1.4f * (.2f + .8f * fft.data[2]) *
           particleLayer(__ctx, (uv * mix(1.f, length(uv), ffts.data[3]) +
                                 n * f32x2(.1f, .05f)) *
                                    25.f * aFrac) *
           smoothstep(1.f, .33f, aFrac) *
           (.1f + .9f * smoothstep(.33f, .66f, aFrac));
  }
  col *= (1.f + amp * 40.f *
                    (1.f + .5f * fft.data[0] * fft.data[0] * fft.data[0] /
                               abs(length(uv) - fract(2.5f * iTime) * 1.15f)));
  col += .05f * step(.95f, fft.data[0]) *
         hash21(__ctx, f32x2(2.5f * iTime, iFrame)) * mod(float(iFrame), 2.f) /
         abs(length(uv) - fract(2.5f * iTime + .1f) * 1.15f);
  col *= .3f * hash21(__ctx, uv * iTime) + .7f;
  col -= length(uv) * .005f;
  col = pow(col, f32x3(.4545f));
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
