// Port of https://www.shadertoy.com/view/mtyXzV; author: LydianLights.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mtyXzV.json.
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
const float PI = 3.1415927f;
f32x3 mainPass(Context &__ctx, f32x2 fragCoord) {
  f32x2 uv = 2.0f * (fragCoord / iResolution.swizzle<0, 1>()) - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 rotUV = uv * f32x2x2(cos(PI / 5.0f), sin(PI / 5.0f), -sin(PI / 5.0f),
                             cos(PI / 5.0f));
  rotUV.data[1] += 0.1f * iTime;
  rotUV = fract(1.5f * rotUV);
  float d0 = abs(step(rotUV.data[0], 0.5f) - step(rotUV.data[1], 0.5f));
  f32x3 color = f32x3(d0);
  float d1 = length(uv);
  d1 = step(d1, 0.5f) * step(0.3f, d1);
  if (d1 > 0.0f)
    color = f32x3(1.0f, 0.25f, 0.25f);
  float d2 = length(uv - f32x2(1.1f, 0.0f));
  d2 = step(d2, 0.5f) * step(0.3f, d2);
  if (d2 > 0.0f)
    color = f32x3(0.25f, 1.0f, 0.25f);
  float d3 = length(uv + f32x2(1.1f, 0.0f));
  d3 = step(d3, 0.5f) * step(0.3f, d3);
  if (d3 > 0.0f)
    color = f32x3(0.25f, 0.25f, 1.0f);
  return color;
}
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
f32x3 antialias(Context &__ctx, f32x2 fragCoord) {
  const float AA_STAGES = 2.0f;
  const float AA_TOTAL_PASSES = AA_STAGES * AA_STAGES + 1.0f;
  const float AA_JITTER = 0.5f;
  f32x3 color = mainPass(__ctx, fragCoord);
  for (float x = 0.0f; x < AA_STAGES; x++) {
    for (float y = 0.0f; y < AA_STAGES; y++) {
      f32x2 offset = AA_JITTER * (2.0f * hash22(__ctx, f32x2(x, y)) - 1.0f);
      color += mainPass(__ctx, fragCoord + offset);
    }
  }
  return color / AA_TOTAL_PASSES;
}
f32x3 gamma(Context &__ctx, f32x3 color) {
  return pow(color, f32x3(1.0f / 2.2f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float sliderX =
      iMouse.data[2] > 0.0f ? iMouse.data[0] : iResolution.data[0] / 2.0f;
  f32x3 base = mainPass(__ctx, fragCoord);
  f32x3 antialiased = antialias(__ctx, fragCoord);
  f32x3 color = fragCoord.data[0] < sliderX ? base : antialiased;
  if (abs(fragCoord.data[0] - sliderX) < 4.0f) {
    color = f32x3(0.5f);
  }
  color = gamma(__ctx, color);
  fragColor = f32x4(color, 1.0f);
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
