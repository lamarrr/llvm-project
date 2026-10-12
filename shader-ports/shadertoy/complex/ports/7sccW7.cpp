// Port of https://www.shadertoy.com/view/7sccW7; author: fpiaggio.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7sccW7.json.
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
float getSound(Context &__ctx) {
  float s = 0.f;
  for (float i = 0.f; i < 20.f; i++) {
    s += texture(__ctx.channel0, f32x2(0.f, i / 20.f)).data[0];
  }
  for (float i = 0.f; i < 20.f; i++) {
    s += texture(__ctx.channel0, f32x2(i / 20.f, 0.f)).data[0];
  }
  return s / 20.f;
}
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, s, -s, c);
}
float hash(Context &__ctx, float x) { return fract(sin(x) * 43758.5453f); }
float noise(Context &__ctx, f32x2 uv) {
  f32x3 x = f32x3(uv.swizzle<0, 1>(), 90.0f);
  f32x3 p = floor(x);
  f32x3 f = fract(x);
  f = f * f * (3.0f - 2.0f * f);
  float offset = 57.0f;
  float n = dot(p, f32x3(1.0f, offset, offset * 2.0f));
  return mix(mix(mix(hash(__ctx, n + 0.0f), hash(__ctx, n + 1.0f), f.data[0]),
                 mix(hash(__ctx, n + offset), hash(__ctx, n + offset + 1.0f),
                     f.data[0]),
                 f.data[1]),
             mix(mix(hash(__ctx, n + offset * 2.0f),
                     hash(__ctx, n + offset * 2.0f + 1.0f), f.data[0]),
                 mix(hash(__ctx, n + offset * 3.0f),
                     hash(__ctx, n + offset * 3.0f + 1.0f), f.data[0]),
                 f.data[1]),
             f.data[2]);
}
float snoise(Context &__ctx, f32x2 uv) {
  return noise(__ctx, uv) * 2.0f - 1.0f;
}
float perlinNoise(Context &__ctx, f32x2 uv) {
  float n = noise(__ctx, uv * 1.0f) * 128.0f + noise(__ctx, uv * 2.0f) * 64.0f +
            noise(__ctx, uv * 4.0f) * 32.0f + noise(__ctx, uv * 8.0f) * 16.0f +
            noise(__ctx, uv * 16.0f) * 8.0f + noise(__ctx, uv * 32.0f) * 4.0f +
            noise(__ctx, uv * 64.0f) * 2.0f + noise(__ctx, uv * 128.0f) * 1.0f;
  float noiseVal =
      n / (1.0f + 2.0f + 4.0f + 8.0f + 16.0f + 32.0f + 64.0f + 128.0f);
  noiseVal = abs(noiseVal * 2.0f - 1.0f);
  return noiseVal;
}
float fBm(Context &__ctx, f32x2 uv, float lacunarity, float gain) {
  float sum = 0.0f;
  float amp = 7.0f;
  for (int i = 0; i < 10; ++i) {
    sum += (perlinNoise(__ctx, uv)) * amp;
    amp *= gain;
    uv *= lacunarity;
  }
  return sum;
}
f32x3 particles(Context &__ctx, f32x2 pos) {
  f32x3 c = f32x3(0, 0, 0);
  float noiseFactor = fBm(__ctx, pos, 0.01f, 0.1f);
  for (float i = 1.0f; i < 2.0f + 1.0f; ++i) {
    float cs =
        cos(iTime + getSound(__ctx) * .2f * 0.10f * (i / 2.0f) + noiseFactor) *
        10.010f;
    float ss =
        sin(iTime + getSound(__ctx) * .2f * 10.50f * (i / 2.0f) + noiseFactor) *
        0.010f;
    f32x2 origin = f32x2(cs, ss);
    float t = sin(iTime + getSound(__ctx) * .2f * 0.30f * i) * 0.5f + 0.5f;
    float particleSize = mix(1.76f, 10.61f, t);
    float d =
        clamp(sin(length(pos - origin) + particleSize), 0.0f, particleSize);
    float t2 = sin(iTime + getSound(__ctx) * .2f * 0.70f * i) * 0.5f + 0.5f;
    f32x3 color = mix(f32x3(.0f, 2.0f, 2.0f), f32x3(5.0f, 1.0f, 1.0f), t2);
    c += color * pow(d, 5.0f);
  }
  return c;
}
float line(Context &__ctx, f32x2 a, f32x2 b, f32x2 p) {
  f32x2 aTob = b - a;
  f32x2 aTop = p - a;
  float t = dot(aTop, aTob) / dot(aTob, aTob) * fBm(__ctx, a, 0.f, 0.f);
  t = clamp(t, 0.0f, 1.0f);
  float d = length(p - (a + aTob * t));
  d = 1.0f / d;
  return clamp(d, 0.0f, 1.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float scale = 200.0f;
  float v = 70.0f + sin(iTime + getSound(__ctx) * .2f) * 30.f;
  uv *= r2d(__ctx, iTime + getSound(__ctx) * .2f * 0.25f);
  float freqA =
      mix(0.4f, 1.2f, sin(iTime + getSound(__ctx) * .2f + 30.0f) * 0.5f + 0.5f);
  float freqB =
      mix(0.4f, 1.2f, sin(iTime + getSound(__ctx) * .2f + 20.0f) * 0.5f + 0.5f);
  float freqC =
      mix(0.4f, 1.2f, sin(iTime + getSound(__ctx) * .2f + 10.0f) * 0.5f + 0.5f);
  f32x3 finalColor = f32x3(0.0f);
  finalColor =
      (particles(
           __ctx,
           sin(abs(uv * (sin(iTime + getSound(__ctx) * .2f * 0.2f) + 0.6f) *
                   5.f))) *
       length(uv)) *
      0.20f;
  uv *= r2d(__ctx, sin(-iTime + getSound(__ctx) * .2f * 2.25f));
  float t = line(__ctx, f32x2(-v, -v), f32x2(0.0f, v), uv * scale);
  uv *= r2d(__ctx, cos(-iTime + getSound(__ctx) * .2f * 1.25f));
  finalColor += f32x3(8.0f * t, 2.0f * t, 4.0f * t) * freqA;
  t = line(__ctx, f32x2(0.0f, v), f32x2(v, -v), uv * scale);
  uv *= r2d(__ctx, -iTime + getSound(__ctx) * .2f * .45f);
  finalColor += f32x3(2.0f * t, 2.0f * t, 4.0f * t) * freqB;
  t = line(__ctx, f32x2(-v, -v), f32x2(v, -v), uv * scale);
  uv *= r2d(__ctx, sin(iTime + getSound(__ctx) * .2f * 100.15f));
  finalColor += f32x3(2.0f * t, 6.0f * t, 4.0f * t) * freqB;
  fragColor = f32x4(finalColor, 1.0f);
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
