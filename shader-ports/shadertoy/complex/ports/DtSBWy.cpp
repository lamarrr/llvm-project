// Port of https://www.shadertoy.com/view/DtSBWy; author: gehtsiegarnixan.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtSBWy.json.
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
f32x3 baryWeights(Context &__ctx, f32x2 uv) {
  float weightA = min(uv.data[0], uv.data[1]);
  float weightB = min(1.f - uv.data[0], 1.f - uv.data[1]);
  float weightC = 1.f - weightA - weightB;
  return f32x3(weightA, weightB, weightC);
}
float straightContrast(Context &__ctx, float alpha, float contrast) {
  return clamp(contrast * (alpha - 0.5f) + 0.5f, 0.f, 1.f);
}
f32x3 HeightToNormalDerivative(Context &__ctx, sampler2D heightMap, f32x2 uv,
                               float strength) {
  float height = texture(heightMap, uv).data[0];
  f32x2 xy =
      -f32x2(ddx(height), ddy(height)) *
      (iResolution.swizzle<0, 1>() / iChannelResolution[0].swizzle<0, 1>());
  return normalize(f32x3(xy * strength, 1.f));
}
f32x3 HeightToNormal3Sample(Context &__ctx, sampler2D heightMap, f32x2 uv,
                            float strength) {
  f32x2 s = 1.0f / iChannelResolution[0].swizzle<0, 1>();
  float textureOffset = 1.0f;
  float p = texture(heightMap, uv).data[0];
  float h1 = texture(heightMap, uv + s * f32x2(textureOffset, 0)).data[0];
  float v1 = texture(heightMap, uv + s * f32x2(0, textureOffset)).data[0];
  f32x2 xy = (p - f32x2(h1, v1));
  return normalize(f32x3(xy * strength, 1.f));
}
f32x3 HeightToNormal3Sample2(Context &__ctx, sampler2D heightMap, f32x2 uv,
                             float strength) {
  f32x2 s = 1.f / iResolution.swizzle<0, 1>();
  float height = texture(heightMap, uv).data[0];
  f32x2 dxy =
      height - f32x2(texture(heightMap, uv + f32x2(s.data[0], 0.f)).data[0],
                     texture(heightMap, uv + f32x2(0.f, s.data[1])).data[0]);
  f32x2 xy = dxy / s;
  return normalize(f32x3(xy * strength, 1.f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 test;
  for (int i = 0; i < 1; i++) {
    float mut = (float(i) / float(1));
    test += HeightToNormalDerivative(__ctx, __ctx.channel0, uv + mut, 50.f);
  }
  fragColor = f32x4((test * .5f + .5f), 1.f);
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
