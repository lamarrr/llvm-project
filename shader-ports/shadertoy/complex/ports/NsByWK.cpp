// Port of https://www.shadertoy.com/view/NsByWK; author: MarcoF.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NsByWK.json.
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
f32x3 mod289(Context &__ctx, f32x3 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x2 mod289(Context &__ctx, f32x2 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x3 permute(Context &__ctx, f32x3 x) {
  return mod289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
float snoise(Context &__ctx, f32x2 v) {
  f32x4 C = f32x4(0.211324865405187f, 0.366025403784439f, -0.577350269189626f,
                  0.024390243902439f);
  f32x2 i = floor(v + dot(v, C.swizzle<1, 1>()));
  f32x2 x0 = v - i + dot(i, C.swizzle<0, 0>());
  f32x2 i1 = f32x2(0.0f);
  i1 = (x0.data[0] > x0.data[1]) ? f32x2(1.0f, 0.0f) : f32x2(0.0f, 1.0f);
  f32x2 x1 = x0.swizzle<0, 1>() + C.swizzle<0, 0>() - i1;
  f32x2 x2 = x0.swizzle<0, 1>() + C.swizzle<2, 2>();
  i = mod289(__ctx, i);
  f32x3 p =
      permute(__ctx, permute(__ctx, i.data[1] + f32x3(0.0f, i1.data[1], 1.0f)) +
                         i.data[0] + f32x3(0.0f, i1.data[0], 1.0f));
  f32x3 m = max(0.5f - f32x3(dot(x0, x0), dot(x1, x1), dot(x2, x2)), 0.0f);
  m = m * m;
  m = m * m;
  f32x3 x = 2.0f * fract(p * C.swizzle<3, 3, 3>()) - 1.0f;
  f32x3 h = abs(x) - 0.5f;
  f32x3 ox = floor(x + 0.5f);
  f32x3 a0 = x - ox;
  float a = 1.79284291400159f;
  float b = 0.85373472095314f;
  m *= a - b * (a0 * a0 + h * h);
  f32x3 g = f32x3(0.0f);
  g.data[0] = a0.data[0] * x0.data[0] + h.data[0] * x0.data[1];
  g.set_swizzle<1, 2>(a0.swizzle<1, 2>() * f32x2(x1.data[0], x2.data[0]) +
                      h.swizzle<1, 2>() * f32x2(x1.data[1], x2.data[1]));
  return 130.0f * dot(m, g);
}
float fbm(Context &__ctx, f32x2 uv, int octaves, float amplitude,
          float frequency, f32x2 shift, float lacunarity, float gain) {
  float value = 0.0f;
  for (int i = 0; i < octaves; ++i) {
    float sn = snoise(__ctx, uv * frequency);
    value += amplitude * sn;
    uv += shift;
    frequency *= lacunarity;
    amplitude *= gain;
  }
  return value;
}
float easySmoothstep(Context &__ctx, float edge, float smoothness, float x) {
  return smoothstep(edge - smoothness, edge + smoothness, x);
}
float line(Context &__ctx, float uv, float pos, float size) {
  return 1.0f - easySmoothstep(__ctx, size * 0.5f, 0.05f, abs(uv - pos));
}
float changeRange(Context &__ctx, float iValue, float iMin, float iMax,
                  float fMin, float fMax) {
  float fValue = (iValue - iMin) / (iMax - iMin);
  fValue *= fMax - fMin;
  fValue += fMin;
  return fValue;
}
f32x2 scale(Context &__ctx, f32x2 original, f32x2 scale, f32x2 pivot) {
  scale = 1.0f / scale;
  f32x2x2 s = f32x2x2(scale.data[0], 0.0f, 0.0f, scale.data[1]);
  f32x2 final = original;
  final -= pivot;
  final *= s;
  final += pivot;
  return final;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.data[1];
  f32x3 col = f32x3(0.f);
  float n = fbm(__ctx, uv * 0.3f + f32x2(iTime * 0.1f, 0.f), 1, 1.0f, 4.0f,
                f32x2(0.f), 2.0f, 0.5f);
  f32x2 uvR = scale(__ctx, uv, f32x2(1.0f) + n * 0.05f, f32x2(0.5f));
  f32x2 uvGrid = fract(uvR * 10.f);
  float lx = 1.0f - line(__ctx, uvGrid.data[0], 0.025f, 0.05f);
  float ly = 1.0f - line(__ctx, uvGrid.data[1], 0.025f, 0.05f);
  col = f32x3(1.0f) * min(lx, ly);
  float shade = changeRange(__ctx, n, -1.0f, 1.0f, 0.0f, 1.0f);
  col *= f32x3(1.0f) * shade;
  fragColor = f32x4(col, 1.f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
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
