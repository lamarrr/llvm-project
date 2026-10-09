// Port of https://www.shadertoy.com/view/ctKBDW; author: workingclasshacker.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctKBDW.json.
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
f32x2x2 rotate2d(Context &__ctx, float _angle) {
  return f32x2x2(cos(_angle), -sin(_angle), sin(_angle), cos(_angle));
}
float noise(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float perlinNoise(Context &__ctx, f32x2 uv) {
  f32x2 i = floor(uv);
  f32x2 f = fract(uv);
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(mix(noise(__ctx, i + f32x2(0.0f, 0.0f)),
                 noise(__ctx, i + f32x2(1.0f, 0.0f)), u.data[0]),
             mix(noise(__ctx, i + f32x2(0.0f, 1.0f)),
                 noise(__ctx, i + f32x2(1.0f, 1.0f)), u.data[0]),
             u.data[1]);
}
f32x2 perlinNoise22(Context &__ctx, f32x2 uv) {
  f32x2 i = floor(uv);
  f32x2 f = fract(uv);
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(mix(i + f32x2(0.0f, 0.0f), i + f32x2(1.0f, 0.0f), u.data[0]),
             mix(i + f32x2(0.0f, 1.0f), i + f32x2(1.0f, 1.0f), u.data[0]),
             u.data[1]);
}
float ffbm(Context &__ctx, f32x2 uv, float scale, float offset) {
  float n = 0.0f;
  float amp = 1.0f;
  for (int i = 0; i < 4; i++) {
    n += perlinNoise(__ctx, uv * scale) * amp;
    uv *= 2.0f;
    amp *= 0.5f;
  }
  return n + offset;
}
float circleGradient(Context &__ctx, f32x2 uv, float blur) {
  float dist = length(uv);
  return smoothstep(0.5f + blur, 0.5f - blur, dist);
}
f32x3 applyBlur(Context &__ctx, f32x3 color, float blurAmount, f32x2 uv) {
  f32x3 accumulatedColor = f32x3(0.0f);
  float totalWeight = 0.0f;
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      f32x2 sampleUv = uv + f32x2(x, y) * blurAmount;
      accumulatedColor += texture(__ctx.channel0, sampleUv).swizzle<0, 1, 2>();
      totalWeight += 1.0f;
    }
  }
  return accumulatedColor / totalWeight;
}
float gaussian(Context &__ctx, float x, float sigma) {
  return exp(-(x * x) / (2.0f * sigma * sigma));
}
f32x2 lissajous(Context &__ctx, float t, float a, float b, float delta) {
  return f32x2(sin(a * t + delta), sin(b * t));
}
f32x3 applyGlow(Context &__ctx, f32x3 color, float distance, float glowStrength,
                float glowRadius) {
  float glow = gaussian(__ctx, distance, glowRadius) * glowStrength;
  return color + glow;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float scale = (sin(iTime) * .5f + .5f) + 2.f;
  f32x2 uv =
      scale *
      (2.0f * gl_FragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
      min(iResolution.data[0], iResolution.data[1]);
  float t = mod(iTime * 0.1f, 3.1415926535897932384626433832795f * 2.0f);
  float bass = texture(__ctx.channel0, f32x2(0.00f, 0.0f)).data[0];
  float b = pow(bass, 3.0f);
  uv += lissajous(__ctx, t, 1.0f, 2.0f,
                  3.1415926535897932384626433832795f / 2.0f);
  uv *=
      rotate2d(__ctx, mod(t * 2.f, 3.1415926535897932384626433832795f * 2.0f));
  f32x2 ouv = uv;
  float ilength = 1.0f / length(uv);
  uv = uv * ilength - f32x2(ilength + t, 0.5f);
  f32x3 finalColor = f32x3(0.0f);
  float noise = 0.0f;
  for (int i = 0; i < 5; i++) {
    float scale = 5.5f + float(i) * 0.5f;
    float offset = float(i) * 0.5f;
    uv *= rotate2d(__ctx, mod(t * offset * .2f,
                              3.1415926535897932384626433832795f * 2.0f));
    noise =
        ffbm(__ctx,
             uv - t * 0.7f * float(i) - f32x2(float(i * i), float(i) / 2.0f),
             scale, offset * 2.f) *
        ilength * 0.2f;
    finalColor += f32x3(noise / 5.0f);
  }
  finalColor.set_swizzle<0, 1>(finalColor.swizzle<0, 1>() * (b));
  finalColor.data[2] +=
      0.4f /
      (ffbm(__ctx, uv * sin(mod(t, 3.0f) * 3.0f) * 0.5f, 5.0f, 0.0f) * 0.5f +
       0.5f);
  finalColor.data[0] +=
      0.2f /
      (ffbm(__ctx, uv * cos(mod(t, 4.0f) * 3.0f) * 0.5f, 5.0f, 0.0f) * 0.5f +
       0.5f);
  finalColor = applyGlow(__ctx, finalColor, length(ouv - uv), .3f, 3.1f);
  float grad = circleGradient(__ctx, uv, 0.5f);
  finalColor -=
      (0.0001f / length(uv) * 0.5f + 0.5f) * sin(b * 4.5f) * 0.1f * grad;
  finalColor -= (length(ouv)) * .1f;
  fragColor = f32x4(finalColor, 1.0f);
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
