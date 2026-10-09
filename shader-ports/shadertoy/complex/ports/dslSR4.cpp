// Port of https://www.shadertoy.com/view/dslSR4; author: rubioh.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dslSR4.json.
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
float hash11(Context &__ctx, float x) {
  return fract(sin(float(x) * 11.45067f) * 41.234f);
}
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x2 a = f32x2(110.487f, 9.4567f);
  f32x2 b = f32x2(4506.775f, 50.2467f);
  return fract(sin(f32x2(dot(a, p), dot(b, p))) * 41.234f);
}
float hash21(Context &__ctx, f32x2 p) {
  f32x2 a = f32x2(110.487f, 9.4567f);
  return fract(sin(dot(a, p)) * 41.234f);
}
f32x2 hash12(Context &__ctx, float x) {
  f32x3 p = f32x3(x, x, x);
  f32x3 a = f32x3(10.487f, 9.4567f, 41.28753f);
  f32x3 b = f32x3(456.775f, 150.2467f, 234.4565f);
  return fract(sin(f32x2(dot(a, p), dot(b, p))) * 41.234f);
}
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  float a = hash21(__ctx, i);
  float b = hash21(__ctx, i + f32x2(1.0f, 0.0f));
  float c = hash21(__ctx, i + f32x2(0.0f, 1.0f));
  float d = hash21(__ctx, i + f32x2(1.0f, 1.0f));
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float GaborNoise(Context &__ctx, f32x2 uv, f32x2 omega, float freq, f32x2 orig,
                 float bandwidth) {
  float f = 2.f * 3.14159f * freq;
  float a = exp(-length(orig - uv) * bandwidth * 3.14159f);
  return a * cos(f * dot(uv, omega));
}
f32x2 GaborAmplitude(Context &__ctx, f32x2 uv, f32x2 omega, float freq,
                     f32x2 orig, float bandwidth) {
  float f = 2.f * 3.14159f * freq;
  float a = exp(-length(orig - uv) * bandwidth * 3.14159f);
  return a * f32x2(cos(f * dot(uv, omega)), sin(f * dot(uv, omega)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  uv.data[0] /= 1.f;
  float gn;
  int N = 100;
  f32x2 L = f32x2(0.f);
  float my = iMouse.data[1] / iResolution.data[1];
  if (my == 0.f) {
    my = .5f;
  }
  float mx = iMouse.data[0] / iResolution.data[0];
  if (mx == 0.f) {
    mx = .5f;
  }
  float phase_velocity = 2.f * mx;
  float freq = 5.f;
  float bandwidth = .01f;
  for (int i = 0; i < N; i++) {
    f32x2 pos = f32x2(noise(__ctx, f32x2(float(i), 0.f)),
                      noise(__ctx, f32x2(float(i), 1.f)));
    float omega_ = atan(noise(__ctx, f32x2(float(i), 1.f)),
                        noise(__ctx, f32x2(float(i), 2.f))) *
                       phase_velocity +
                   iTime / 20.f;
    f32x2 omega = f32x2(cos(omega_), sin(omega_));
    freq = 40.f * my;
    bandwidth = .01f;
    L += GaborAmplitude(__ctx, (uv - pos), omega, freq, uv, bandwidth);
    gn += GaborNoise(__ctx, (uv - pos), omega, freq, uv, bandwidth);
  }
  float amplitude = length(L);
  float pn = gn / amplitude;
  gn /= sqrt(float(N));
  float an = amplitude / sqrt(float(N));
  pn = mix(pn, an * .5f, step(0.f, uv.data[1]));
  f32x3 col = f32x3(mix(gn, pn, step(0.f, uv.data[0]))) +
              100.f * f32x3(1.f, 0.f, 0.f) *
                  smoothstep(0.f, .005f, .005f - abs(uv.data[0])) +
              100.f * f32x3(1.f, 0.f, 0.f) *
                  smoothstep(0.f, .005f, .005f - abs(uv.data[1])) *
                  step(0.f, uv.data[0]);
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
