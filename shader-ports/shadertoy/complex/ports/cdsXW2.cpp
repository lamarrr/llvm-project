// Port of https://www.shadertoy.com/view/cdsXW2; author: elloskelling.
// Dataset license metadata: mit; original retained in upstream/cdsXW2.json.
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
float field(Context &__ctx, f32x3 p, float s) {
  float strength = 7.f + .03f * log(1.e-6f + fract(sin(iTime) * 4373.11f));
  float accum = s / 4.f;
  float prev = 0.f;
  float tw = 0.f;
  for (int i = 0; i < 26; ++i) {
    float mag = dot(p, p);
    p = abs(p) / mag + f32x3(-.5f, -.4f, -1.5f);
    float w = exp(-float(i) / 7.f);
    accum += w * exp(-strength * pow(abs(mag - prev), 2.2f));
    tw += w;
    prev = mag;
  }
  return max(0.f, 5.f * accum / tw - .7f);
}
float field2(Context &__ctx, f32x3 p, float s) {
  float strength = 7.f + .03f * log(1.e-6f + fract(sin(iTime) * 4373.11f));
  float accum = s / 4.f;
  float prev = 0.f;
  float tw = 0.f;
  for (int i = 0; i < 18; ++i) {
    float mag = dot(p, p);
    p = abs(p) / mag + f32x3(-.5f, -.4f, -1.5f);
    float w = exp(-float(i) / 7.f);
    accum += w * exp(-strength * pow(abs(mag - prev), 2.2f));
    tw += w;
    prev = mag;
  }
  return max(0.f, 5.f * accum / tw - .7f);
}
f32x3 nrand3(Context &__ctx, f32x2 co) {
  f32x3 a = fract(cos(co.data[0] * 8.3e-3f + co.data[1]) *
                  f32x3(1.3e5f, 4.7e5f, 2.9e5f));
  f32x3 b = fract(sin(co.data[0] * 0.3e-3f + co.data[1]) *
                  f32x3(8.1e5f, 1.0e5f, 0.1e5f));
  f32x3 c = mix(a, b, 0.5f);
  return c;
}
float sdTriangle(Context &__ctx, f32x2 p, f32x2 p0, f32x2 p1, f32x2 p2) {
  f32x2 e0 = p1 - p0;
  f32x2 e1 = p2 - p1;
  f32x2 e2 = p0 - p2;
  f32x2 v0 = p - p0;
  f32x2 v1 = p - p1;
  f32x2 v2 = p - p2;
  f32x2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0f, 1.0f);
  f32x2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0f, 1.0f);
  f32x2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0f, 1.0f);
  float s = e0.data[0] * e2.data[1] - e0.data[1] * e2.data[0];
  f32x2 d =
      min(min(f32x2(dot(pq0, pq0),
                    s * (v0.data[0] * e0.data[1] - v0.data[1] * e0.data[0])),
              f32x2(dot(pq1, pq1),
                    s * (v1.data[0] * e1.data[1] - v1.data[1] * e1.data[0]))),
          f32x2(dot(pq2, pq2),
                s * (v2.data[0] * e2.data[1] - v2.data[1] * e2.data[0])));
  return -sqrt(d.data[0]) * sign(d.data[1]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 pp = (2.0f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  pp *= 1.5f;
  f32x2 uv =
      2.f * fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 1.f;
  f32x2 uvs = uv * iResolution.swizzle<0, 1>() /
              max(iResolution.data[0], iResolution.data[1]);
  f32x3 p = f32x3(uvs / 4.f, 0) + f32x3(1.f, -1.3f, 0.f);
  p += .2f * f32x3(sin(iTime / 16.f), sin(iTime / 12.f), sin(iTime / 128.f));
  float freqs[6];
  freqs[0] = texture(__ctx.channel0, f32x2(0.01f, 0.25f)).data[0];
  freqs[1] = texture(__ctx.channel0, f32x2(0.03f, 0.25f)).data[0];
  freqs[2] = texture(__ctx.channel0, f32x2(0.05f, 0.25f)).data[0];
  freqs[3] = texture(__ctx.channel0, f32x2(0.07f, 0.25f)).data[0];
  freqs[4] = texture(__ctx.channel0, f32x2(0.15f, 0.25f)).data[0];
  freqs[5] = texture(__ctx.channel0, f32x2(0.30f, 0.25f)).data[0];
  float t = field(__ctx, p, freqs[4]);
  float v = (1.f - exp((abs(uv.data[0]) - 1.f) * 6.f)) *
            (1.f - exp((abs(uv.data[1]) - 1.f) * 6.f));
  f32x3 p2 = f32x3(uvs / (4.f + sin(iTime * 0.11f) * 0.2f + 0.2f +
                          sin(iTime * 0.15f) * 0.3f + 0.4f),
                   1.5f) +
             f32x3(2.f, -1.3f, -1.f);
  p2 += 0.25f * f32x3(sin(iTime / 16.f), sin(iTime / 12.f), sin(iTime / 128.f));
  float t2 = field2(__ctx, p2, freqs[5]);
  f32x4 c2 = mix(.4f, 1.f, v) *
             f32x4(1.3f * t2 * t2 * t2, 1.8f * t2 * t2, t2 * freqs[0], t2);
  f32x2 seed = p.swizzle<0, 1>() * 2.0f;
  seed = floor(seed * iResolution.data[0]);
  f32x3 rnd = nrand3(__ctx, seed);
  f32x4 starcolor = f32x4(pow(rnd.data[1], 40.0f));
  f32x2 seed2 = p2.swizzle<0, 1>() * 2.0f;
  seed2 = floor(seed2 * iResolution.data[0]);
  f32x3 rnd2 = nrand3(__ctx, seed2);
  starcolor += f32x4(pow(rnd2.data[1], 40.0f));
  f32x4 fC = 0.2f +
             1.3f * (mix(freqs[5] - .3f, 1.f, v) *
                         f32x4(1.5f * freqs[4] * t * t * t,
                               1.2f * freqs[3] * t * t, freqs[5] * t, 1.0f) +
                     c2) +
             1.5f * starcolor;
  float beat = 0.25f * freqs[0] * freqs[0] + 0.25f * freqs[1] * freqs[1] +
               0.25f * freqs[2] * freqs[2] + 0.25f * freqs[3] * freqs[3];
  beat = sqrt(beat) * 3.7f + 0.f;
  float s = 2.3f;
  f32x2 v1 = f32x2(0.0f, 0.577f - 0.1f) * (s);
  f32x2 v2 = f32x2(-0.5f, -0.289f - 0.1f) * (s);
  f32x2 v3 = f32x2(0.5f, -0.289f - 0.1f) * (s);
  float d = sdTriangle(__ctx, pp, v1, v2, v3);
  f32x3 col = beat - sign(d) * f32x3(0.1f, 0.4f, 0.7f);
  if (d > 0.f) {
    col += fC.swizzle<0, 1, 2>() - beat;
  } else {
    col *= 0.8f + 0.2f * cos(120.0f * d);
    col *= f32x3(1.f, 0.f, 0.f);
  }
  col *= 1.0f - exp(-2.0f * abs(d));
  col = mix(col, f32x3(1.0f), 1.0f - smoothstep(0.0f, 0.02f, abs(d)));
  fragColor = f32x4(col, 1.0f);
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
