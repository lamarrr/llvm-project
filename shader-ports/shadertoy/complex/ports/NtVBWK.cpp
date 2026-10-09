// Port of https://www.shadertoy.com/view/NtVBWK; author: rubioh.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NtVBWK.json.
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
f32x3 mix3ColorGradient(Context &__ctx, float ratio, f32x3 a, f32x3 b,
                        f32x3 c) {
  return mix(mix(a, b, ratio / .5f), mix(b, c, (ratio - .5f) / .5f),
             step(.5f, ratio));
}
f32x3 palette(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
f32x2 hash(Context &__ctx, f32x2 p) {
  p = f32x2(dot(p, f32x2(175.1f, 311.7f)), dot(p, f32x2(4260.5f, 7052.3f)));
  return fract(sin(p + 4551.f) * 184.5453f);
}
f32x2 hash2(Context &__ctx, f32x2 p) {
  const f32x2 k = f32x2(0.3183099f, 0.3678794f);
  float n = 111.0f * p.data[0] + 113.0f * p.data[1];
  return fract(n * fract(k * n));
}
float noise_perso(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  float a = hash(__ctx, i).data[0];
  float b = hash(__ctx, i + f32x2(1.0f, 0.0f)).data[0];
  float c = hash(__ctx, i + f32x2(0.0f, 1.0f)).data[0];
  float d = hash(__ctx, i + f32x2(1.0f, 1.0f)).data[0];
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float nrand(Context &__ctx, f32x2 n) {
  return fract(sin(dot(n.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
f32x2 hash4(Context &__ctx, f32x2 p) {
  p = f32x2(nrand(__ctx, p), nrand(__ctx, p.swizzle<1, 0>()));
  return fract(sin(p + 455.f) * 18.5453f);
}
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  float a = hash4(__ctx, i).data[0];
  float b = hash4(__ctx, i + f32x2(1.0f, 0.0f)).data[0];
  float c = hash4(__ctx, i + f32x2(0.0f, 1.0f)).data[0];
  float d = hash4(__ctx, i + f32x2(1.0f, 1.0f)).data[0];
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float fbm(Context &__ctx, f32x2 x, int I, float v) {
  float t = 0.0f;
  float amp = 0.f;
  for (int i = 1; i < I; i++) {
    float f = pow(2.f, float(i));
    float a = pow(f, -.75f);
    amp = 10.f * step(3.f, float(i));
    t += a * noise_perso(__ctx, f * x + amp + f32x2(0.f, -iTime * v));
  }
  return t * 1.f - .4f;
}
float fbm_smoke(Context &__ctx, f32x2 x) {
  float t = 0.0f;
  float amp = 0.f;
  for (int i = 1; i < 8; i++) {
    float f = pow(2.f, float(i));
    float a = pow(f, -.75f);
    t += a * noise(__ctx, f * x - f32x2(0.f, iTime * 1.f));
  }
  return t * 1.f - .4f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (2.f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  uv.data[1] += .7f;
  uv *= 1.f;
  f32x2 st = uv / 2.f * (f32x2(2.f, 1.f));
  float smoke = fbm_smoke(__ctx, st);
  f32x3 a = f32x3(0.f, 0.f, 0.f);
  f32x3 b = f32x3(.2f, .2f, .2f);
  f32x3 c = f32x3(.9f, .75f, .6f);
  f32x3 smoke_col =
      mix3ColorGradient(__ctx, smoke * 2.f, a, b, c) * (1.f / uv.data[1]);
  st = uv * f32x2(2.f, .2f);
  float flamme = 3.f * fbm(__ctx, st + fbm(__ctx, st, 4, 8.f), 4, 8.f);
  st = uv * f32x2(1.f, 1.f);
  flamme = 2.f * fbm(__ctx, st + fbm(__ctx, st, 3, 8.f), 3, 8.f);
  st = uv * f32x2(.1f, .25f);
  flamme += (fbm(__ctx, st + fbm(__ctx, st, 5, 2.f), 5, 2.f) * .7f + .3f) *
            (1.f - uv.data[1]);
  flamme = pow(flamme, 2.f * (uv.data[1] + .05f));
  a = f32x3(0.f, 0.f, 0.f);
  b = f32x3(.7f, .1f, .1f);
  c = f32x3(0.9f, 0.9f, .5f);
  f32x3 flamme_col = mix3ColorGradient(__ctx, flamme * 2.f, a, b, c);
  float flamme_mask = smoothstep(0.f, 1.f, flamme_col.data[0]);
  flamme_col = clamp(flamme_col, 0.0f, 1.0f);
  smoke_col = clamp(smoke_col, 0.0f, 1.0f);
  fragColor = f32x4(mix(f32x3(0.f), flamme_col, flamme_mask), 1.0f);
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
