// Port of https://www.shadertoy.com/view/7tXfzB; author: MrNissenDK.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tXfzB.json.
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

float seed = 437585.453123f;
float PI = 3.141592653589793f;
float res = 10.f;
float hash21(Context &__ctx, f32x2 p) {
  p = mod(p, PI * 20.f);
  float a = (mod(43758.5453123f + seed, 37562.f) + 13548.f);
  float f = dot(p, f32x2(127.1f, 351.7f));
  return fract(sin(f) * a);
}
f32x2 hash2(Context &__ctx, f32x2 p) {
  p = mod(p, PI * 20.f);
  float a = (mod(43758.5453123f + seed, 37562.f) + 13548.f);
  p = f32x2(dot(p, f32x2(127.1f, 351.7f)), dot(p, f32x2(245.5f, 304.4f)));
  return fract(sin(p) * a);
}
f32x3 hash23(Context &__ctx, f32x2 p) {
  p = mod(p, PI * 20.f);
  float a = (mod(43758.5453123f + seed, 37562.f) + 13548.f);
  f32x3 p3 = f32x3(dot(p, f32x2(127.1f, 351.7f)), dot(p, f32x2(261.5f, 183.3f)),
                   dot(p, f32x2(245.5f, 304.4f)));
  return fract(sin(p3) * a);
}
f32x2 hash32(Context &__ctx, f32x3 p) {
  p = mod(p, PI * 20.f);
  float a = (mod(43758.5453123f + seed, 37562.f) + 13548.f);
  f32x2 p2 = f32x2(dot(p, f32x3(127.1f, 351.7f, 251.45f)),
                   dot(p, f32x3(261.5f, 183.3f, 376.89f)));
  return fract(sin(p2) * a);
}
float hash31(Context &__ctx, f32x3 p) {
  p = mod(p, PI * 20.f);
  float a = (mod(43758.5453123f + seed, 37562.f) + 13548.f);
  float f = dot(p, f32x3(127.1f, 351.7f, 251.45f));
  return fract(sin(f) * a);
}
float ease(Context &__ctx, float t) {
  return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
}
f32x2 ease(Context &__ctx, f32x2 t) {
  return f32x2(ease(__ctx, t.data[0]), ease(__ctx, t.data[1]));
}
f32x3 ease(Context &__ctx, f32x3 t) {
  return f32x3(ease(__ctx, t.data[0]), ease(__ctx, t.data[1]),
               ease(__ctx, t.data[2]));
}
f32x4 ease(Context &__ctx, f32x4 t) {
  return f32x4(ease(__ctx, t.data[0]), ease(__ctx, t.data[1]),
               ease(__ctx, t.data[2]), ease(__ctx, t.data[3]));
}
float noise(Context &__ctx, f32x2 point) {
  f32x2 fr = ease(__ctx, fract(point));
  f32x2 hi = ceil(point);
  f32x2 lo = floor(point);
  return mix(mix(hash21(__ctx, lo),
                 hash21(__ctx, f32x2(hi.data[0], lo.data[1])), fr.data[0]),
             mix(hash21(__ctx, f32x2(lo.data[0], hi.data[1])),
                 hash21(__ctx, hi), fr.data[0]),
             fr.data[1]);
}
float noise(Context &__ctx, f32x3 point) {
  f32x3 fr = ease(__ctx, fract(point));
  f32x3 hi = ceil(point);
  f32x3 lo = floor(point);
  return mix(
      mix(mix(hash31(__ctx, lo),
              hash31(__ctx, f32x3(hi.data[0], lo.swizzle<1, 2>())), fr.data[0]),
          mix(hash31(__ctx, f32x3(lo.data[0], hi.data[1], lo.data[2])),
              hash31(__ctx, f32x3(hi.swizzle<0, 1>(), lo.data[2])), fr.data[0]),
          fr.data[1]),
      mix(mix(hash31(__ctx, f32x3(lo.swizzle<0, 1>(), hi.data[2])),
              hash31(__ctx, f32x3(hi.data[0], lo.data[1], hi.data[2])),
              fr.data[0]),
          mix(hash31(__ctx, f32x3(lo.data[0], hi.swizzle<1, 2>())),
              hash31(__ctx, hi), fr.data[0]),
          fr.data[1]),
      fr.data[2]);
}
float octal(Context &__ctx, f32x3 uv, int octaves, float persistence) {
  float total = 0.f;
  float frequency = 1.f;
  float amplitude = 1.f;
  float maxValue = 0.f;
  for (int i = 0; i < octaves; i++) {
    total += noise(__ctx, uv * frequency) * amplitude;
    maxValue += amplitude;
    amplitude *= persistence;
    frequency *= 2.f;
  }
  return total / maxValue;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = ((fragCoord - .5f * iResolution.swizzle<0, 1>()) /
              (iResolution.data[0] / res));
  float r =
      1.f -
      abs((-octal(__ctx,
                  f32x3(uv + f32x2(sin(iTime / 10.f), cos(iTime / 10.f)) * 10.f,
                        500.f + iTime / 5.0f),
                  8, .5f) +
           .5f) *
          2.f);
  float g = 1.f - abs((-octal(__ctx,
                              f32x3(uv + f32x2(sin(iTime / 10.f),
                                               cos(500.f + iTime / 10.f)) *
                                             10.f,
                                    2.f * iTime / 5.0f),
                              8, .5f) +
                       .5f) *
                      2.f);
  float b = 1.f - abs((-octal(__ctx,
                              f32x3(uv + f32x2(sin(500.f + iTime / 10.f),
                                               cos(iTime / 10.f)) *
                                             10.f,
                                    2.f * iTime / 5.0f),
                              8, .5f) +
                       .5f) *
                      2.f);
  f32x3 col = f32x3(r, g, b);
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output) {
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
