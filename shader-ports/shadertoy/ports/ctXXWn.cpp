// Port of https://www.shadertoy.com/view/ctXXWn; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctXXWn.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float pulse(Context &__ctx, float x, float k) {
  return 1.f - min(1.f, abs(fract(x) - 0.5f) / k);
}
f32x2x2 r(Context &__ctx, float t) {
  return f32x2x2(cos(t), -sin(t), sin(t), cos(t));
}
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(p.swizzle<0, 1, 0>() * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
float perlinNoise(Context &__ctx, f32x2 p, float t) {
  f32x2x2 r = r(__ctx, t);
  f32x2 tlVal =
      r * (hash22(__ctx, f32x2(floor(p.data[0]), ceil(p.data[1]))) - 0.5f) *
      2.0f;
  f32x2 blVal =
      r * (hash22(__ctx, f32x2(floor(p.data[0]), floor(p.data[1]))) - 0.5f) *
      2.0f;
  f32x2 trVal =
      r * (hash22(__ctx, f32x2(ceil(p.data[0]), ceil(p.data[1]))) - 0.5f) *
      2.0f;
  f32x2 brVal =
      r * (hash22(__ctx, f32x2(ceil(p.data[0]), floor(p.data[1]))) - 0.5f) *
      2.0f;
  float tl = dot(p - f32x2(floor(p.data[0]), ceil(p.data[1])), tlVal);
  float bl = dot(p - f32x2(floor(p.data[0]), floor(p.data[1])), blVal);
  float tr = dot(p - f32x2(ceil(p.data[0]), ceil(p.data[1])), trVal);
  float br = dot(p - f32x2(ceil(p.data[0]), floor(p.data[1])), brVal);
  return mix(mix(bl, tl, smoothstep(0.f, 1.f, fract(p.data[1]))),
             mix(br, tr, smoothstep(0.f, 1.f, fract(p.data[1]))),
             smoothstep(0.f, 1.f, fract(p.data[0]))) *
             0.5f +
         0.5f;
}
float vignette(Context &__ctx, f32x2 p, float s) {
  p = 1.f - (4.f * p * p - 4.f * p + 1.f) * s;
  return p.data[0] * p.data[1];
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float v = perlinNoise(__ctx, uv * 2.f + iDate.data[3] * 0.05f,
                        iDate.data[3] * 0.2f);
  float fw = fwidth(v);
  float t = 0.8f;
  f32x3 col =
      (smoothstep(t, t + fw * 10.f * 2.f, pulse(__ctx, v * 10.f, 1.f)) * 0.8f +
       0.2f) *
      f32x3(1.000f, 0.812f, 0.643f);
  col = mix(col.swizzle<0, 1, 2>(), col.swizzle<0, 1, 1>(),
            smoothstep(0.f, .25f, sin(v * 6.2831f)));
  fragColor = f32x4(
      col * vignette(__ctx, fragCoord / iResolution.swizzle<0, 1>(), 0.3f),
      1.0f);
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
