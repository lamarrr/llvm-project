// Port of https://www.shadertoy.com/view/3tXfWN; author: gchipunov.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/3tXfWN.json.
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
float lerp(Context &__ctx, float a, float b, float t) {
  return a + t * (b - a);
}
f32x4 lerp(Context &__ctx, f32x4 a, f32x4 b, float t) {
  return a + t * (b - a);
}
float atan2(Context &__ctx, float y, float x) {
  float t0, t1, t2, t3, t4;
  t3 = abs(x);
  t1 = abs(y);
  t0 = max(t3, t1);
  t1 = min(t3, t1);
  t3 = float(1) / t0;
  t3 = t1 * t3;
  t4 = t3 * t3;
  t0 = -float(0.013480470f);
  t0 = t0 * t4 + float(0.057477314f);
  t0 = t0 * t4 - float(0.121239071f);
  t0 = t0 * t4 + float(0.195635925f);
  t0 = t0 * t4 - float(0.332994597f);
  t0 = t0 * t4 + float(0.999995630f);
  t3 = t0 * t3;
  t3 = (abs(y) > abs(x)) ? float(1.570796327f) - t3 : t3;
  t3 = (x < 0.0f) ? float(3.141592654f) - t3 : t3;
  t3 = (y < 0.0f) ? -t3 : t3;
  return t3;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() /
             f32x2(iResolution.data[1], iResolution.data[1]);
  f32x2 center =
      f32x2((0.5f * (iResolution.data[0] / iResolution.data[1])), 0.5f);
  float deltaYFromCenter = center.data[1] - uv.data[1];
  float deltaXFromCenter = center.data[0] - uv.data[0];
  f32x4 musicMap;
  float angleFromCenter = atan2(__ctx, deltaYFromCenter, deltaXFromCenter);
  musicMap.data[0] =
      (angleFromCenter + 3.14159265359f) / (3.14159265359f * 2.0f);
  float distFromCenter = sqrt((deltaYFromCenter * deltaYFromCenter) +
                              (deltaXFromCenter * deltaXFromCenter));
  float circleFromCenter = (distFromCenter * 3.0f);
  circleFromCenter = circleFromCenter - 0.4f;
  ;
  musicMap.data[2] = circleFromCenter;
  if (musicMap.data[2] < 0.0f) {
    fragColor = f32x4(1.00f, 1.00f, 1.00f, 1.0f);
  } else {
    float musicChannelnput =
        texture(__ctx.channel0, f32x2(musicMap.data[0], 0.0f)).data[0];
    float barStart = floor(musicMap.data[0] * 70.0f) / 70.0f;
    float intensity = 0.0f;
    for (float s = 0.0f; s < 1.0f / 70.0f; s += 1.0f / 70.0f * 2.0f) {
      intensity += texture(__ctx.channel0, f32x2(barStart + s, 0.0f)).data[0];
    }
    intensity *= 2.0f;
    if (musicMap.data[0] - barStart < 0.1f * 1.0f / 70.0f ||
        musicMap.data[0] > barStart + 1.0f / 70.0f - 0.1f * 1.0f / 70.0f) {
      intensity = 0.0f;
    }
    float onOff = musicMap.data[2] - intensity;
    onOff = clamp(onOff, -0.5f, 0.5f);
    onOff = floor(onOff + 1.0f);
    fragColor = lerp(__ctx, f32x4(.71f, .11f, .11f, 1.0f),
                     f32x4(.21f, .40f, .91f, 1.0f), onOff);
  }
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
