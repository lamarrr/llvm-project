// Port of https://www.shadertoy.com/view/md3GDl; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/md3GDl.json.
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
  fft.data[0] = smoothstep(.8f, .9f, fft.data[0]);
}
float fitPoly2(Context &__ctx, float x, f32x3 y) {
  return y.data[0] + (4.f * y.data[1] - 3.f * y.data[0] - y.data[2]) * x +
         (2.f * y.data[0] - 4.f * y.data[1] + 2.f * y.data[2]) * x * x;
}
f32x3 getCol(Context &__ctx, float id, float s) {
  f32x3 setCol = f32x3(255);
  if (id == 0.f)
    setCol = f32x3(fitPoly2(__ctx, s, f32x3(9, 30, 0)),
                   fitPoly2(__ctx, s, f32x3(0, 29, 250)),
                   fitPoly2(__ctx, s, f32x3(66, 215, 253)));
  else if (id == 1.f)
    setCol = f32x3(fitPoly2(__ctx, s, f32x3(132, 244, 255)),
                   fitPoly2(__ctx, s, f32x3(0, 0, 148)),
                   fitPoly2(__ctx, s, f32x3(92, 204, 255)));
  else if (id == 2.f)
    setCol = f32x3(fitPoly2(__ctx, s, f32x3(39, 26, 173)),
                   fitPoly2(__ctx, s, f32x3(153, 246, 252)),
                   fitPoly2(__ctx, s, f32x3(106, 138, 248)));
  else if (id == 3.f)
    setCol = f32x3(fitPoly2(__ctx, s, f32x3(250, 252, 236)),
                   fitPoly2(__ctx, s, f32x3(47, 157, 212)),
                   fitPoly2(__ctx, s, f32x3(11, 0, 80)));
  else if (id == 4.f)
    setCol = f32x3(fitPoly2(__ctx, s, f32x3(53, 231, 237)),
                   fitPoly2(__ctx, s, f32x3(17, 15, 114)),
                   fitPoly2(__ctx, s, f32x3(18, 20, 41)));
  else if (id == 5.f)
    setCol = f32x3(fitPoly2(__ctx, s, f32x3(35, 103, 178)),
                   fitPoly2(__ctx, s, f32x3(87, 211, 225)),
                   fitPoly2(__ctx, s, f32x3(97, 225, 245)));
  return setCol / 255.f;
}
f32x2x2 rotM(Context &__ctx, float rad) {
  return f32x2x2(cos(rad), -sin(rad), sin(rad), cos(rad));
}
float sdCircle(Context &__ctx, f32x2 pos, float r) { return length(pos) - r; }
float ring(Context &__ctx, f32x2 pos, float r) {
  return step(abs(sdCircle(__ctx, pos, r)), .005f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float aTime = 2.133333f * iTime;
  compressFft(__ctx);
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  f32x3 background = .5f * f32x3(1.f + sign(uv.data[1] + .2f));
  float r = .33f * ffts.data[3] + .03f;
  float maxRing = ceil(10.f * clamp(ffts.data[3], .2f, 1.f));
  uv =
      (fract(aTime / 4.f) > .5f)
          ? fract(2.5f *
                      (.27f + mod(aTime / 32.f, 2.f) * mod(aTime / 32.f, 2.f)) *
                      uv +
                  .5f) -
                f32x2(.5f)
          : .9f * uv;
  uv *= rotM(__ctx, aTime / 8.f);
  f32x3 myCol;
  f32x3 col = f32x3(0);
  for (float n = -maxRing + 1.f; n < maxRing; n++) {
    myCol = getCol(__ctx,
                   mod(n + floor((float(ffts.data[0] < ffts.data[1]) +
                                  float(ffts.data[1] < ffts.data[2]) +
                                  float(ffts.data[2] < ffts.data[0]))),
                       6.f),
                   fft[1 + int(mod(n, 3.f))]);
    col += myCol * ring(__ctx,
                        (uv - f32x2((1.f + .66f * fft.data[0]) * r * n /
                                        maxRing * sin(aTime / 16.f),
                                    .0f)) /
                            f32x2(cos(aTime / 16.f), 1.f),
                        r * sqrt(1.f - n / maxRing * n / maxRing));
  }
  col += background * (1.f - max(col.data[0], max(col.data[1], col.data[2])));
  col = pow(col, f32x3(.4545f));
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
