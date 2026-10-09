// Port of https://www.shadertoy.com/view/dsyyDm; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dsyyDm.json.
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
float aaStep(Context &__ctx, float fun) {
  return smoothstep(min(fwidth(fun), .001f), .0f, fun);
}
f32x2x2 rotM(Context &__ctx, float r) {
  float c = cos(r), s = sin(r);
  return f32x2x2(c, s, -s, c);
}
float hash21(Context &__ctx, f32x2 p) {
  p = fract(p * f32x2(13.81f, 741.76f));
  p += dot(p, p + 42.23f);
  return fract(p.data[0] * p.data[1]);
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
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  float amp = 0.f;
  float delY = .1f;
  float r, ox, fun;
  compressFft(__ctx);
  for (float m = -1.f; m <= 1.f; m += .05f) {
    ox = m;
    fun = uv.data[0] - ox;
    for (float n = 0.f; n < 2.f / delY; n++) {
      r = floor(
          hash21(__ctx, f32x2(n + ceil(150.f / 60.f * iTime / 16.f),
                              ox + max(.98f * step(fft.data[0], .96f),
                                       fract(150.f / 60.f * iTime / 16.f)))) *
              3.f -
          1.f);
      fun += (r * (uv.data[1] - (delY * n - 1.f)) + ox + m) *
             step(delY * n - 1.f, uv.data[1] + .001f) *
             step(uv.data[1] + .001f, delY * (n + 1.f) - 1.f);
      ox = r * (delY * (n + 1.f) - 1.f) - (r * (delY * n - 1.f) - ox - m) - m;
    }
    amp +=
        (.8f + .5f * ffts[int(m * m * 3.f)]) *
        (aaStep(__ctx,
                abs(fun) - .009f * iResolution.data[1] / iResolution.data[0]) *
             (.1f +
              .9f * aaStep(__ctx,
                           abs(fragCoord.data[1] / iResolution.data[1] -
                               fract(150.f / 60.f * iTime / 8.f +
                                     m * (.6f + .4f * ffts.data[3]))) -
                               .005f * (1.f + 17.f * step(.95f, fft.data[0])) *
                                   iResolution.data[1] / iResolution.data[0])) +
         1e-3f / length(f32x2(fun, fragCoord.data[1] / iResolution.data[1] -
                                       fract(150.f / 60.f * iTime / 8.f +
                                             m * (.6f + .4f * ffts.data[3])))));
  }
  f32x3 col = f32x3(clamp(amp, .0f, 1.f));
  col += col * max(.0f,
                   1.f - 5.f * fragCoord.data[1] / iResolution.data[1] *
                             (1.f - fft.data[0] * fft.data[0] * fft.data[0])) +
         smoothstep(.0f, 1.f, .2f - amp) * fft.data[3];
  col *=
      abs(cos(.06f * 150.f / 60.f * iTime +
              3.14159265359f / f32x3(.5f, 2.f, 4.f) + ffts.swizzle<0, 1, 2>()));
  col *= mix(.2f + .8f * (1.f - length(uv)), 1.f, fft.data[2]);
  uv *= .8f + .5f * (1.f - fft.data[0] * fft.data[0]);
  fun =
      mix(abs(length(uv) - .23f) - .07f,
          (sin(150.f / 60.f * iTime / 4.f + fft.data[1] - .5f) < .0f)
              ? abs(dot(abs(uv * rotM(__ctx, 3.14159265359f / 4.f)), f32x2(1)) -
                    .25f) -
                    .1f
              : abs(sdEquilateralTriangle(__ctx, uv * 4.f)) - .27f,
          clamp(sin(150.f / 60.f * iTime / 2.f + fft.data[1] - .5f), .0f, 1.f));
  col = f32x3(ffts.data[0] <= ffts.data[1], ffts.data[1] <= ffts.data[2],
              ffts.data[2] <= ffts.data[0]) *
            fft.data[2] * .08f +
        abs(1.f - col) * aaStep(__ctx, fun) * fft.data[1] * .13f +
        col * (1.f - step(length(col), .9f) * aaStep(__ctx, fun)) * .8f;
  fragColor = f32x4(col, 1);
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
