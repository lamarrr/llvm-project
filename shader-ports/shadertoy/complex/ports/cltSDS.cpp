// Port of https://www.shadertoy.com/view/cltSDS; author: osnajanos.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cltSDS.json.
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
f32x2 get_uv(Context &__ctx, f32x2 position) {
  return (position * 2.0f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
}
float map(Context &__ctx, float value, float min1, float max1, float min2,
          float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}
float map_01(Context &__ctx, float value, float min, float max) {
  return map(__ctx, value, min, max, 0.0f, 1.0f);
}
float map_tri(Context &__ctx, float value, float min, float max) {
  return map(__ctx, value, -1.0f, 1.0f, min, max);
}
float map_tri_01(Context &__ctx, float value) {
  return map_tri(__ctx, value, 0.0f, 1.0f);
}
float sdHexagon(Context &__ctx, f32x2 p, float r) {
  const f32x3 k = f32x3(-0.866025404f, 0.5f, 0.577350269f);
  p = abs(p);
  p -= 2.0f * min(dot(k.swizzle<0, 1>(), p), 0.0f) * k.swizzle<0, 1>();
  p -= f32x2(clamp(p.data[0], -k.data[2] * r, k.data[2] * r), r);
  return length(p) * sign(p.data[1]);
}
float beat(Context &__ctx, float bpm) {
  return smoothstep(0.5f, 1.0f, sqrt(1.0f - mod(iTime, 60.0f / bpm)));
}
float beat_osc(Context &__ctx, float bpm, float min, float max, float speed,
               float offset) {
  const float two_pi = 2.0f * 3.14159265f;
  return map_tri(__ctx,
                 cos(offset * two_pi + speed * iTime * bpm / 60.0f * two_pi),
                 min, max);
}
f32x3 palette(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
f32x3 palette_neon(Context &__ctx, float t) {
  return palette(__ctx, t, f32x3(0.5f), f32x3(0.5f), f32x3(1.0f),
                 f32x3(0.263f, 0.416f, 0.557f));
}
f32x3 palette_spectrum(Context &__ctx, float t) {
  return palette(__ctx, t, f32x3(0.5f), f32x3(0.5f), f32x3(2.0f, 1.0f, 0.0f),
                 f32x3(0.5f, 0.2f, 0.25f));
}
f32x3 mix_palettes(Context &__ctx, float t, float p) {
  float p1 = p;
  float p2 = 1.0f - p;
  return p1 * palette_neon(__ctx, t) + p2 * palette_spectrum(__ctx, t);
}
f32x3 hexagons(Context &__ctx, f32x2 position, float scale, float density,
               float lineWidth, float iterations, float offset,
               float colorMix) {
  f32x3 color = f32x3(0.0f);
  f32x2 uv = get_uv(__ctx, position);
  f32x2 uv0 = uv;
  for (float i = 0.0f; i < iterations; i++) {
    uv = fract(uv * scale) - 0.5f;
    float d0 = sdHexagon(__ctx, uv0, 1.0f);
    float d = sdHexagon(__ctx, uv, 1.0f);
    d = sin(d * density + offset) / density;
    d = abs(d);
    d = (0.0025f * lineWidth) / d;
    d = smoothstep(0.0f, 0.3f, d);
    f32x3 c = mix_palettes(__ctx, d0 + offset, colorMix);
    color += c * d;
  }
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  const float bpm = 174.f / 2.f;
  f32x3 color = f32x3(0.0f);
  f32x3 layerBackground = hexagons(
      __ctx, fragCoord, beat_osc(__ctx, bpm, 2.95f, 3.05f, 1.0f / 16.0f, 0.0f),
      beat_osc(__ctx, bpm, 12.0f, 15.0f, 1.0f / 16.0f, 0.1f), 3.0f, 2.0f,
      -iTime * 0.125f, beat_osc(__ctx, bpm, 0.5f, 1.0f, 1.0f / 8.0f, 0.0f));
  f32x3 layerBackgroundMask =
      hexagons(__ctx, fragCoord, beat_osc(__ctx, bpm, 1.0f, 2.0f, 0.0f, 0.0f),
               5.0f, 4.0f, 1.25f, -iTime * 0.25f,
               beat_osc(__ctx, bpm, 0.0f, 1.0f, 0.5f, 0.0f));
  f32x3 layerMid =
      hexagons(__ctx, fragCoord, 1.5f,
               beat_osc(__ctx, bpm, 17.5f, 18.0f, 0.25f, 0.0f), 0.5f, 1.0f,
               -iTime * 0.5f, beat_osc(__ctx, bpm, 0.0f, 1.0f, 0.125f, 0.0f));
  f32x3 LayerForeground = hexagons(
      __ctx, fragCoord, beat_osc(__ctx, bpm, 0.6f, 0.7f, 1.0f / 32.0f, 0.0f),
      beat_osc(__ctx, bpm, 12.0f, 16.0f, 1.0f / 64.f, 0.0f), 2.0f, 2.0f,
      -iTime * 0.25f, beat_osc(__ctx, bpm, 0.0f, 1.0f, 1.0f / 32.f, 0.0f));
  color += layerBackground * layerBackgroundMask;
  color += layerMid * beat(__ctx, bpm) * 0.5f;
  color += LayerForeground * beat(__ctx, bpm);
  fragColor.set_swizzle<0, 1, 2>(color);
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
