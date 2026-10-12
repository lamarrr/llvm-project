// Port of https://www.shadertoy.com/view/cl23zh; author: iq.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cl23zh.json.
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
const float kFreqBase = 11.0f;
const float kFreqAM = 1.0f;
const float kFreqSignal = kFreqBase + kFreqAM;
float kFreqSample;
const int kNumSincs = 900;
const float kPi = 3.1415927f;
const float kTau = 6.2831853f;
float sinc(Context &__ctx, float x) {
  return abs(x) > 0.0f ? sin(kPi * x) / (kPi * x) : 1.0f;
}
float signal(Context &__ctx, float t) {
  float f = 0.0f;
  f += 1.0f * (0.5f - 0.5f * cos(kTau * t)) * sin(kTau * kFreqBase * t);
  t -= 0.5f;
  f += 0.5f * (0.5f - 0.5f * cos(kTau * t)) * sin(kTau * kFreqBase * t);
  return f;
}
float reconstruct(Context &__ctx, float tc) {
  float y = 0.0f;
  int c = int(floor(tc * kFreqSample));
  for (int n = c - kNumSincs; n <= c + kNumSincs; n++) {
    float t = float(n) / kFreqSample;
    y += signal(__ctx, t) * sinc(__ctx, kFreqSample * (t - tc));
  }
  return y;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float time = mod(iTime, 15.0f);
  kFreqSample =
      kFreqSignal * mix(0.5f, 2.0f * 1.005f, smoothstep(0.0f, 10.0f, time));
  float u = fragCoord.data[0] / iResolution.data[0];
  f32x3 col = f32x3(0.15f) * (0.75f + u * (1.0f - u));
  f32x2 uv = fragCoord / iResolution.data[1];
  float px = 1.0f / iResolution.data[1];
  float sc = 1.1f;
  float am = 0.4f;
  float t = uv.data[0] * sc;
  {
    float x = signal(__ctx, t);
    float y = reconstruct(__ctx, t);
    float dx = am * ddx(x) / (sc * px);
    float dy = am * ddx(y) / (sc * px);
    x = abs(am * x - (uv.data[1] - 0.5f)) / sqrt(1.0f + dx * dx);
    y = abs(am * y - (uv.data[1] - 0.5f)) / sqrt(1.0f + dy * dy);
    x = smoothstep(px * 2.0f / sc, 0.0f, x);
    y = smoothstep(px * 2.0f / sc, 0.0f, y);
    col = mix(col, f32x3(1.0f, 0.7f, 0.0f), x);
    col = mix(col, f32x3(0.0f, 0.7f, 1.0f), y);
  }
  {
    float a = smoothstep(12.0f, 10.0f, time);
    float x = round(t * kFreqSample) / kFreqSample;
    float y = signal(__ctx, x);
    f32x2 p = f32x2(t - x, am * y - (uv.data[1] - 0.5f));
    float f = smoothstep(px * 1.5f, 0.0f, length(p) - 0.01f);
    col = mix(col, f32x3(1.0f, 1.0f, 1.0f), a * f);
  }
  col += fract(sin(fragCoord.data[0] * f32x3(13, 17, 11) +
                   fragCoord.data[1] * f32x3(1, 7, 5)) *
               158.391832f) /
         255.0f;
  fragColor = f32x4(col, 1.0f);
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
