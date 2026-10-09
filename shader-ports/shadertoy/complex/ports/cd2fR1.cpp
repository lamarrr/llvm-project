// Port of https://www.shadertoy.com/view/cd2fR1; author: piyushslayer.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cd2fR1.json.
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
float Hash12(Context &__ctx, f32x2 p, float f) {
  p = abs(mod(p, f));
  u32x2 q = u32x2(i32x2(p)) * u32x2(1597334673U, 3812015801U);
  unsigned n = (q.data[0] ^ q.data[1]) * 1597334673U;
  return float(n) * (1.0f / float(0xffffffffU));
}
float PerlinNoise(Context &__ctx, f32x2 x, float freq) {
  x *= freq;
  f32x2 f = fract(x);
  x = floor(x);
  float a = Hash12(__ctx, x, freq);
  float b = Hash12(__ctx, x + f32x2(1.0f, 0.0f), freq);
  float c = Hash12(__ctx, x + f32x2(0.0f, 1.0f), freq);
  float d = Hash12(__ctx, x + f32x2(1.0f, 1.0f), freq);
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float PerlinFbm(Context &__ctx, f32x2 uv, float freq, int octaves) {
  float amp = .5f;
  float noise = 0.f;
  for (int i = 0; i < octaves; ++i) {
    noise += amp * PerlinNoise(__ctx, uv, freq);
    amp *= .5f;
    freq *= 2.f;
  }
  return noise;
}
f32x2 GenerateCarbonFiberMap(Context &__ctx, f32x2 uv) {
  f32x2 gridUv =
      uv * 50.0f * f32x2(1.0f, iResolution.data[1] / iResolution.data[0]);
  f32x2 gridId = floor(gridUv);
  float maskScaleInverse = 0.5f;
  f32x2 maskOffset = (gridUv + gridId.swizzle<1, 0>()) * maskScaleInverse;
  float patternMask = mod(floor(maskOffset.data[0]), 2.0f);
  f32x2 gradients = 1.0f - abs(fract(maskOffset) - 0.5f);
  gradients -= 0.5f;
  gradients = clamp(pow(gradients, f32x2(0.25f)), 0.0f, 1.0f);
  float heightMap = mix(gradients.data[1], gradients.data[0], patternMask);
  float anisotropicBrush =
      mix(PerlinFbm(__ctx,
                    uv * f32x2(50.0f, 0.1f) + f32x2(0.0f, uv.data[0] * 8.0f) +
                        iTime * 0.02f,
                    32.0f, 8),
          PerlinFbm(__ctx,
                    uv * f32x2(0.1f, 50.0f) + f32x2(uv.data[1] * 8.0f, 0.0f) +
                        iTime * 0.02f,
                    32.0f, 8),
          patternMask);
  heightMap = mix(anisotropicBrush * 0.3f, heightMap, heightMap);
  return f32x2(patternMask, heightMap);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 carbonFiber = GenerateCarbonFiberMap(__ctx, uv);
  f32x3 outColor = f32x3(1.0f);
  f32x3 normal =
      normalize(f32x3(dFdx(carbonFiber.data[1]) * 2.0f,
                      dFdy(carbonFiber.data[1]) * 2.0f, carbonFiber.data[1]));
  f32x3 lightDirection = normalize(
      f32x3(iMouse.data[1] < 0.5f ? f32x2(iResolution.swizzle<0, 1>() * 0.5f)
                                  : iMouse.swizzle<0, 1>(),
            500.0f) -
      f32x3(fragCoord, 0.0f));
  outColor = f32x3(max(0.0f, dot(normal, lightDirection))) * 2.0f;
  f32x3 r = reflect(-lightDirection, normal);
  outColor +=
      pow(max(0.0f,
              dot(normalize(f32x3(f32x2(0.5f), 1000.0f) - f32x3(uv, 0.0f)), r)),
          8.0f) *
      0.5f;
  outColor *= carbonFiber.data[1] * mix(f32x3(0.329f, 0.329f, 0.329f),
                                        f32x3(0.165f, 0.165f, 0.165f),
                                        carbonFiber.data[0]);
  fragColor = f32x4(outColor, 1.0f);
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
