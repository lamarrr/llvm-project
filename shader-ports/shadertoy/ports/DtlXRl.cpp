// Port of https://www.shadertoy.com/view/DtlXRl; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtlXRl.json.
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
  fft.data[0] = step(.9f, fft.data[0]);
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
float circ(Context &__ctx, f32x2 p, float d) {
  return .05f / abs(length(p) - d);
}
float line(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 ab = b - a;
  return .05f /
         length(a + (ab)*clamp(dot(p - a, ab) / dot(ab, ab), 0.f, 1.f) - p);
}
float lines(Context &__ctx, f32x2 p, f32x2[4] coord, int maxCoord) {
  float amp = 0.f;
  for (int n = 0; n < maxCoord; n++) {
    amp += line(__ctx, p, coord[n], coord[(n + 1) % maxCoord]);
  }
  return amp;
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
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float maxRes = max(iResolution.data[0], iResolution.data[1]);
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) / maxRes;
  f32x3 col;
  float aFrac, amp = 0.f;
  animTime = 2.133333f * iTime;
  fft = f32x4(0), ffts = f32x4(0);
  compressFft(__ctx);
  for (float n = 0.f; n < 4.f; n++) {
    aFrac = fract(-.05f * animTime + .25f * n) -
            .02f * fft.data[3] * fft.data[3] * fft.data[3];
    amp += 1.4f * (.2f + .8f * fft.data[2]) *
           dustLayer(__ctx, (uv + n * f32x2(.1f, .05f)) * 25.f * aFrac) *
           smoothstep(1.f, .33f, aFrac);
  }
  amp *= (.7f + .5f * length(uv));
  f32x2[] myBox =
      f32x2[](f32x2(1, 1), f32x2(1, -1), f32x2(-1, -1), f32x2(-1, 1));
  f32x2[] myTri = f32x2[](
      f32x2(0, 1), f32x2(cos(3.14159265359f / 6.f), -sin(3.14159265359f / 6.f)),
      f32x2(-cos(3.14159265359f / 6.f), -sin(3.14159265359f / 6.f)), f32x2(0));
  aFrac = fract(.5f * animTime + 4.f * cos(.04f * animTime));
  if (aFrac < 1.f / 3.f) {
    amp += fft.data[1] *
           lines(__ctx, 5.f * uv / (.8f + .4f * ffts.data[3]), myBox, 4);
  } else if (aFrac < 2.f / 3.f) {
    amp += fft.data[1] * lines(__ctx,
                               5.f *
                                   sqrt((1.f + sin(3.14159265359f / 6.f)) *
                                        cos(3.14159265359f / 6.f) / 4.f) *
                                   uv * rotM(__ctx, -5.f * animTime) /
                                   (.8f + .4f * ffts.data[3]),
                               myTri, 3);
  } else {
    amp += fft.data[1] * circ(__ctx,
                              5.f * sqrt(3.14159265359f / 4.f) * uv /
                                  (.8f + .4f * ffts.data[3]),
                              1.f);
  }
  col = f32x3(amp * amp);
  col *= f32x3(ffts.data[0] <= ffts.data[1], ffts.data[1] <= ffts.data[2],
               ffts.data[2] <= ffts.data[0]);
  float maxY =
      (maxRes == iResolution.data[0]) ? 1.f : iResolution.data[1] / maxRes;
  amp =
      fft.data[0] * .03f / abs(abs(uv.data[1]) - 2.f * maxY * fract(animTime));
  col += amp * amp;
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
