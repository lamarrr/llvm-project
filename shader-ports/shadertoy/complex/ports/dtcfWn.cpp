// Port of https://www.shadertoy.com/view/dtcfWn; author: DrieStone.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtcfWn.json.
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
float baseVerticalPosition = 185.0f;
float flameHeightFactor = 3.5f;
float flameIntensityFactor = 2.2f;
float rand(Context &__ctx, f32x2 n) {
  return fract(sin(cos(dot(n, f32x2(12.9898f, 12.1414f)))) * 83758.5453f);
}
float noise(Context &__ctx, f32x2 n) {
  const f32x2 d = f32x2(0.0f, 1.0f);
  f32x2 b = floor(n), f = smoothstep(f32x2(0.0f), f32x2(1.0f), fract(n));
  return mix(mix(rand(__ctx, b), rand(__ctx, b + d.swizzle<1, 0>()), f.data[0]),
             mix(rand(__ctx, b + d.swizzle<0, 1>()),
                 rand(__ctx, b + d.swizzle<1, 1>()), f.data[0]),
             f.data[1]);
}
float fbm(Context &__ctx, f32x2 n) {
  float total = 0.0f, amplitude = 1.0f;
  for (int i = 0; i < 5; i++) {
    total += noise(__ctx, n) * amplitude;
    n += n * 1.7f;
    amplitude *= 0.47f;
  }
  return total;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  const f32x3 c1 = f32x3(0.5f, 0.0f, 0.1f);
  const f32x3 c2 = f32x3(0.9f, 0.1f, 0.0f);
  const f32x3 c3 = f32x3(0.2f, 0.1f, 0.7f);
  const f32x3 c4 = f32x3(1.0f, 0.9f, 0.1f);
  const f32x3 c5 = f32x3(0.1f);
  const f32x3 c6 = f32x3(0.9f);
  f32x2 speed = f32x2(0.1f, 0.9f);
  float shift = 1.327f + sin(iTime * 2.0f) / 2.4f;
  float alpha = 1.0f;
  float dist = 3.5f - sin(iTime * 0.4f) / 1.89f;
  float flameVerticalPosition =
      baseVerticalPosition * iResolution.data[1] / 720.0f;
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 p = (fragCoord.swizzle<0, 1>() + f32x2(0.0f, flameVerticalPosition)) *
            dist / iResolution.swizzle<0, 0>();
  p += sin(p.swizzle<1, 0>() * 4.0f + f32x2(.2f, -.3f) * iTime) * 0.04f;
  p += sin(p.swizzle<1, 0>() * 8.0f + f32x2(.6f, +.1f) * iTime) * 0.01f;
  p.data[0] -= iTime / 1.1f;
  float q = fbm(__ctx, p - iTime * 0.3f + 1.0f * sin(iTime + 0.5f) / 2.0f);
  float qb = fbm(__ctx, p - iTime * 0.4f + 0.1f * cos(iTime) / 2.0f);
  float q2 = fbm(__ctx, p - iTime * 0.44f - 5.0f * cos(iTime) / 2.0f) - 6.0f;
  float q3 = fbm(__ctx, p - iTime * 0.9f - 10.0f * cos(iTime) / 15.0f) - 4.0f;
  float q4 = fbm(__ctx, p - iTime * 1.4f - 20.0f * sin(iTime) / 14.0f) + 2.0f;
  q = (q + qb - .4f * q2 - 2.0f * q3 + .6f * q4) / 3.8f;
  f32x2 r = f32x2(
      fbm(__ctx, p + q / 2.0f + iTime * speed.data[0] - p.data[0] - p.data[1]),
      fbm(__ctx, p + q - iTime * speed.data[1]));
  f32x3 c = mix(c1, c2, fbm(__ctx, p + r)) + mix(c3, c4, r.data[0]) -
            mix(c5, c6, r.data[1]);
  f32x3 color = f32x3(1.0f / (pow(c + 1.61f, f32x3(4.0f))) *
                      cos(shift * fragCoord.data[1] / iResolution.data[1]));
  color = f32x3(1.0f, .2f, .05f) /
          (pow((r.data[1] + r.data[1]) * max(.0f, p.data[1]) + 0.1f, 4.0f));
  ;
  color +=
      (texture(__ctx.channel0, uv * 0.6f + f32x2(.5f, .1f)).swizzle<0, 1, 2>() *
           0.01f * pow((r.data[1] + r.data[1]) * .65f, 5.0f) +
       0.055f) *
      mix(f32x3(.9f, .4f, .3f), f32x3(.7f, .5f, .2f), uv.data[1]);
  color = color / (1.0f + max(f32x3(0), color));
  f32x3 fireColor =
      f32x3(color.data[0], color.data[1], color.data[2]) * flameIntensityFactor;
  f32x4 webcamColor = texture(__ctx.channel0, uv);
  float fireLuminance = dot(fireColor, f32x3(0.299f, 0.587f, 0.114f));
  f32x3 blendedColor =
      mix(webcamColor.swizzle<0, 1, 2>(), fireColor,
          clamp(fireLuminance * flameHeightFactor, 0.0f, 1.0f));
  fragColor = f32x4(blendedColor, 1.0f);
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
