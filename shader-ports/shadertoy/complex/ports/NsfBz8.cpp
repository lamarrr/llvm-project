// Port of https://www.shadertoy.com/view/NsfBz8; author: Yusef28.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NsfBz8.json.
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
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

float noise(Context &__ctx, f32x2 st) {
  return fract(sin(dot(f32x2(12.23f, 74.343f), st)) * 43254.f);
}
float rnd(Context &__ctx, float a) {
  return fract(sin(a * 234.15f) * 41240.91f);
}
float noise2D(Context &__ctx, f32x2 st) {
  f32x2 id = floor(st);
  f32x2 f = fract(st);
  float a = noise(__ctx, id);
  float b = noise(__ctx, id + f32x2(1.f, 0.f));
  float c = noise(__ctx, id + f32x2(0.f, 1.f));
  float d = noise(__ctx, id + f32x2(1.f));
  f = smoothstep(0.f, 1.f, f);
  float ab = mix(a, b, f.data[0]);
  float cd = mix(c, d, f.data[0]);
  return mix(ab, cd, f.data[1]);
}
f32x2x2 rot45 = f32x2x2(0.707f, -0.707f, 0.707f, 0.707f);
f32x2x2 rot(Context &__ctx, float a) {
  float s = sin(a);
  float c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float fbm(Context &__ctx, f32x2 st, float N, float rt) {
  st *= 3.f;
  float s = .5f;
  float ret = 0.f;
  for (float i = 0.f; i < N; i++) {
    ret += noise2D(__ctx, st) * s;
    st *= 2.9f;
    s /= 2.f;
    st *= rot(__ctx, (acos(-1.f) * (i + 1.f) / N) + rt * 8.f);
  }
  return ret;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * 0.5f) /
             iResolution.data[1];
  f32x3 col = f32x3(0.f);
  col += pow(
      mix(texture(__ctx.channel0,
                  uv * 2.5f +
                      f32x2(floor(iTime * 0.2f + uv.data[0]), 0.f) * 0.5f)
              .data[0],
          texture(__ctx.channel0,
                  uv * 2.5f +
                      f32x2(floor(iTime * 0.2f + 1.f + uv.data[0]), 0.f) * 0.5f)
              .data[0],
          smoothstep(0.f, 1.f, fract(iTime * 0.2f + uv.data[0]))),
      9.f);
  f32x3 theColor;
  for (float i = 1.f; i < 14.f; i++) {
    f32x2 altUV = uv;
    float j = pow(rnd(__ctx, i) * 0.6f, 0.5f);
    theColor = f32x3(j, j * j * j, j * j * 0.27f) +
               texture(__ctx.channel1, uv).swizzle<0, 0, 0>() * 0.1f;
    uv = uv + f32x2(iTime / (80.f - i * 2.f), 0.f);
    float faa = fbm(__ctx, uv + i * 100.f, 5.f, 6.f);
    faa = abs(faa);
    float shadow = smoothstep(0.5f, 0.3f, faa);
    col = mix(col, theColor * 0.1f, shadow * 0.5f);
    float faa2 = fbm(__ctx, uv + i * 100.f + .03f, 5.f, 6.f);
    float shadow2 = clamp(smoothstep(0.4f, 0.36f, faa2), 0.f, 1.f);
    col = mix(col, f32x3(0.f), shadow2 * 0.7f);
    float stripes = smoothstep(
        0.3f, 0.2f,
        abs(fract((uv.data[1] - uv.data[0]) * (15.f + i * 1.3f)) - 0.5f));
    float f = max(0.f, smoothstep(0.4f, 0.39f, faa));
    float g = max(0.f, smoothstep(0.4f, 0.39f, faa));
    float h = clamp(g - max(0.f, smoothstep(0.36f, 0.34f, faa)), 0.f, 1.f);
    if (mod(i, 3.f) == 0.f) {
      stripes = 1.f;
      theColor = theColor.swizzle<2, 1, 1>();
      theColor += pow(
          mix(texture(__ctx.channel0,
                      altUV * 2.5f +
                          f32x2(floor(iTime * 0.3f + uv.data[0]), 0.f) * 0.5f)
                  .data[0],
              texture(__ctx.channel0,
                      altUV * 2.5f +
                          f32x2(floor(iTime * 0.3f + 1.f + uv.data[0]), 0.f) *
                              0.5f)
                  .data[0],
              smoothstep(0.f, 1.f, fract(iTime * 0.3f + uv.data[0]))),
          6.f);
      theColor =
          mix(theColor, texture(__ctx.channel1, uv * 2.f).data[0] * theColor,
              0.5f - h * 0.5f);
    }
    col = mix(col,
              theColor -
                  (stripes + texture(__ctx.channel0, uv).swizzle<0, 0, 0>()) *
                      0.04f,
              f);
    col = mix(col,
              theColor * 0.9f -
                  texture(__ctx.channel1, uv * 8.f).swizzle<0, 0, 0>() * 0.13f,
              h);
  }
  col *= 1.3f;
  uv = fragCoord / iResolution.swizzle<0, 1>();
  uv *= 1.0f - uv.swizzle<1, 0>();
  float vig = uv.data[0] * uv.data[1] * 25.0f;
  vig = pow(vig, 0.3f);
  fragColor = f32x4(col * vig * 1.4f, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_2d<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
