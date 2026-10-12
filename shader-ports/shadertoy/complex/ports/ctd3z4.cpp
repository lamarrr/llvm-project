// Port of https://www.shadertoy.com/view/ctd3z4; author: SentientCymatic.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctd3z4.json.
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

f32x2 hash2(Context &__ctx, float n) {
  return fract(sin(f32x2(n, n + 1.0f)) * f32x2(432.14159f, 528.14159f));
}
const f32x2 randConst = f32x2(432.14159f, 528.14159f);
const float randMultiplier = 3.14159f;
float rand(Context &__ctx, const f32x2 co) {
  return fract(
      sin(co.data[0] * randConst.data[0] + co.data[1] * randConst.data[1]) *
      randMultiplier);
}
float custom_smoothstep(Context &__ctx, float edge0, float edge1, float x) {
  float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}
float noise(Context &__ctx, f32x2 x) {
  f32x2 p = floor(x);
  f32x2 f = fract(x);
  f = f * f * (3.0f - 2.0f * f);
  f32x2 n = p + f32x2(2.5f, -2.5f);
  float a = mix(rand(__ctx, n), rand(__ctx, n + f32x2(1.0f, 0.0f)), f.data[0]);
  float b = mix(rand(__ctx, n + f32x2(0.0f, 1.0f)),
                rand(__ctx, n + f32x2(1.0f, 1.0f)), f.data[0]);
  return mix(a, b, f.data[1]);
}
f32x2 turbulence(Context &__ctx, f32x2 p, float t, float scale) {
  float sum = 0.1f;
  float freq = scale;
  float smoothness;
  f32x2 noise_coord;
  for (int i = 0; i < 5; i++) {
    smoothness = custom_smoothstep(__ctx, 0.0f, 10.0f, float(i));
    noise_coord =
        f32x2(p + t * 0.25f) +
        f32x2(cos(float(i) * 0.5f), sin(float(i) * 0.5f)) * smoothness;
    sum += abs(noise(__ctx, noise_coord)) / freq;
    freq *= 0.25f;
  }
  return f32x2(sum, sum) * 0.1f;
}
f32x2x2 mtx = f32x2x2(0.87f, 0.5f, -0.5f, 0.87f);
float fbm(Context &__ctx, f32x2 p) {
  float f = 0.05f;
  f += 0.950000f * noise(__ctx, p);
  p = mtx * p * 3.0f;
  f += 0.200000f * noise(__ctx, p);
  p = mtx * p * 2.0f;
  f += 0.100000f * noise(__ctx, p);
  p = mtx * p * 2.0f;
  f += 0.050000f * noise(__ctx, p);
  p = mtx * p * 2.0f;
  f += 0.025000f * noise(__ctx, p);
  p = mtx * p * 1.0f;
  f += 0.005000f * noise(__ctx, p);
  p = mtx * p * 2.0f;
  f += 0.004f * noise(__ctx, p);
  p = mtx * p * 2.0f;
  f += 0.002f * noise(__ctx, p);
  return f / 0.95000f;
}
float pattern(Context &__ctx, f32x2 p, float t, f32x2 uv, f32x2 &q, f32x2 &r,
              f32x2 &g) {
  float s = dot(uv + 0.5f, uv + 0.5f);
  float iMelty = 50.0f, iFlowing = 10.0f;
  float l = custom_smoothstep(__ctx, 0.0f, iMelty, sin(t * iFlowing));
  q = mix(f32x2(fbm(__ctx, p + f32x2(t * 1.f + sin(t), t * 0.2f + cos(t))),
                fbm(__ctx, p + f32x2(t * 0.5f + sin(t + 0.5f),
                                     t * 0.5f + cos(t + 1.5f)))),
          f32x2(fbm(__ctx, p), fbm(__ctx, p + f32x2(10.5f, 1.5f))), l);
  r = mix(f32x2(fbm(__ctx, p + 3.14159f * q +
                               f32x2(t * 0.25f + sin(t * 0.25f),
                                     t * 0.25f + cos(t * 0.50f)) +
                               f32x2(1.5f, 10.5f)),
                fbm(__ctx, p + 2.0f * q +
                               f32x2(t * 0.5f + sin(t * 0.3f),
                                     t * 0.4f + cos(t * 0.9f)) +
                               f32x2(8.5f, 4.8f))),
          f32x2(fbm(__ctx, p + 5.0f * q + f32x2(t) + f32x2(33.66f, 66.33f)),
                fbm(__ctx, p + 4.f * q + f32x2(t) + f32x2(8.5f, 2.5f))),
          l);
  g = mix(f32x2(fbm(__ctx, p + 2.0f * r +
                               f32x2(t * 0.5f + sin(t * 0.5f),
                                     t * 0.5f + cos(t * 0.75f)) +
                               f32x2(2.5f, 5)),
                fbm(__ctx, p + 1.5f * r +
                               f32x2(t * 0.75f + sin(t * 0.25f),
                                     t * 0.5f + cos(t * 0.5f)) +
                               f32x2(5, 2.5f))),
          f32x2(fbm(__ctx, p + 2.5f * r + f32x2(t * 5.0f) + f32x2(2, 5)),
                fbm(__ctx, p + 2.f * r + f32x2(t * 11.0f) + f32x2(5, 2.5f))),
          l);
  f32x2 v = turbulence(__ctx, p * 0.1f, t * 0.1f, 20.f);
  f32x2 m = f32x2(fbm(__ctx, p * 0.5f + f32x2(t * 0.9f, t * 0.9f) + v * 0.5f),
                  fbm(__ctx, p * 0.5f + f32x2(t * 0.9f, t * 0.9f) + v * 0.5f));
  return mix(fbm(__ctx, p + 2.5f * g +
                            f32x2(-t * 0.75f + sin(t * 0.5f),
                                  -t * 0.5f + cos(t * 0.25f)) +
                            v * 2.5f + m * 0.25f),
             fbm(__ctx, p + 5.0f * g + f32x2(-t * 5.0f) + v * 2.5f), l);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float iSmoke = 0.01f;
  float iSpeed = 0.25f;
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 q, r, g;
  float noise =
      pattern(__ctx, fragCoord * f32x2(iSmoke), iTime * iSpeed, uv, q, r, g);
  f32x3 col = mix(f32x3(0.2f, 0.4f, 0.2f), f32x3(0.0f, 0.25f, 0.5f),
                  custom_smoothstep(__ctx, 0.1f, 1.0f, noise));
  col = mix(col, f32x3(0.4f, 0.2f, 0.2f), dot(q, q) * 1.5f);
  col = mix(col, f32x3(0.2f, 0.4f, 0.0f), 0.25f * g.data[1] * g.data[1]);
  col = mix(col, f32x3(0.4f, 0.2f, 0.2f),
            custom_smoothstep(__ctx, 0.2f, 0.5f, 1.0f * r.data[1] * r.data[1]));
  col = mix(col, f32x3(0.2f, 0.4f, 0.6f), 0.5f * g.data[0]);
  float timeScale = .25f;
  float xDrift = sin(uv.data[0] * 3.14159f + iTime * timeScale);
  float yDrift = cos(uv.data[1] * 3.14159f + iTime * timeScale);
  f32x3 drift = f32x3(xDrift, yDrift, -xDrift - yDrift) * .1f;
  col += drift;
  col = mix(col, f32x3(1),
            custom_smoothstep(__ctx, 0.f, 1.f, noise) *
                custom_smoothstep(__ctx, 0.f, 10.f, noise));
  col *= noise * 2.f;
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
