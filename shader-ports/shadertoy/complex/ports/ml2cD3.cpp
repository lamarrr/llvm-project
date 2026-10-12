// Port of https://www.shadertoy.com/view/ml2cD3; author: leon.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ml2cD3.json.
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
float hash13(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * .1031f);
  p3 += dot(p3, p3.swizzle<2, 1, 0>() + 31.32f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
f32x3 hash33(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 0, 2>() + 33.33f);
  return fract((p3.swizzle<0, 0, 1>() + p3.swizzle<1, 0, 0>()) *
               p3.swizzle<2, 1, 0>());
}
f32x2 moda(Context &__ctx, f32x2 p, float count) {
  float an = 6.283f / count;
  float a = atan(p.data[1], p.data[0]) + an * .5f;
  a = mod(a, an) - an * .5f;
  return f32x2(cos(a), sin(a)) * length(p);
}
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), -sin(a), sin(a), cos(a));
}
float gyroid(Context &__ctx, f32x3 seed) {
  return dot(sin(seed), cos(seed.swizzle<1, 2, 0>()));
}
float fbm(Context &__ctx, f32x3 seed) {
  float result = 0.f, a = .5f;
  for (int i = 0; i < 4; ++i, a /= 2.f) {
    seed.data[2] += result * 2.5f;
    result += gyroid(__ctx, seed / a) * a;
  }
  return result;
}
float fbm2(Context &__ctx, f32x3 seed) {
  float result = 0.f, a = .5f;
  for (int i = 0; i < 4; ++i, a /= 2.f) {
    seed.data[2] += result * .5f;
    result += (gyroid(__ctx, seed / a) * a);
  }
  return result;
}
float height(Context &__ctx, f32x3 pos, float d) {
  float f = fbm(__ctx, pos) * .5f + .5f;
  d = max(0.f, d - .2f);
  float thin1 = .5f + sin(d * 12.f - iTime) * .1f;
  float thin2 = d * .1f;
  f = abs(abs(abs(f) - thin1) - thin2) - .1f;
  return smoothstep(.01f, -.1f, f - d * .05f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
            iResolution.swizzle<0, 1>().data[1];
  float a = atan(p.data[1], p.data[0]) + iTime * .1f;
  float d = length(p);
  f32x3 pos = f32x3(p, d * .5f);
  pos.data[2] -= iTime * .01f;
  float h = height(__ctx, pos, d);
  f32x3 e = f32x3(1.f / iResolution.swizzle<0, 1>(), 0);
  f32x3 n =
      normalize(f32x3(height(__ctx, pos + f32x3(-e.swizzle<0, 2>(), 0), d) -
                          height(__ctx, pos + f32x3(e.swizzle<0, 2>(), 0), d),
                      height(__ctx, pos + f32x3(-e.swizzle<2, 1>(), 0), d) -
                          height(__ctx, pos + f32x3(e.swizzle<2, 1>(), 0), d),
                      0.2f / max(.001f, d - .2f)));
  f32x3 tint = .5f + .5f * cos(f32x3(1, 2, 3) * 2.f + a);
  f32x3 color = f32x3(.05f);
  tint = mix(f32x3(1), tint, smoothstep(.0f, .5f, d));
  float l = max(0.f, dot(n, -normalize(f32x3(p, -1))));
  l *= smoothstep(-.5f, .2f, d);
  color = f32x3(pow(l, 1.9f) * pow(1.f - h, 4.f)) * tint;
  color += tint * pow(l, 20.f);
  color *= smoothstep(0.f, .1f, h);
  e = f32x3(.01f, 0, 0);
  pos = f32x3(p, 0.f);
  pos += h * .02f;
  pos.data[0] =
      abs(pos.data[0]) - iTime * .1f + 0.05f / max(.0f, abs(p.data[0]) + .1f);
  float x = (fbm2(__ctx, pos + e.swizzle<1, 0, 1>()) -
             fbm2(__ctx, pos - e.swizzle<1, 0, 1>())) /
            (2.f * e.data[0]);
  float y = (fbm2(__ctx, pos + e.swizzle<0, 1, 1>()) -
             fbm2(__ctx, pos - e.swizzle<0, 1, 1>())) /
            (2.f * e.data[0]);
  f32x2 curl = f32x2(-x, y);
  p += curl * .1f * smoothstep(.9f, .0f, abs(p.data[0]) - .7f);
  float shape = abs(p.data[1]) - .2f;
  float strips = mix(.8f + .2f * sin(uv.data[1] * 1000.f + iTime * 10.f +
                                     h * 5.f + uv.data[0] * 200.f),
                     1.f, d);
  color += strips * tint * min(1.f, (0.00f + h * .1f) / max(0.f, shape));
  color *= mix(1.f, .2f, h * d);
  color = mix(
      color, f32x3(0),
      smoothstep(mix(0.01f, .1f, h), .0f, abs(p.data[1]) - mix(0.1f, .01f, h)));
  color = mix(color, f32x3(1),
              smoothstep(mix(0.01f, .1f, h), .0f,
                         abs(p.data[1] - .05f) - mix(0.05f, .01f, h)));
  fragColor = f32x4(color, 1.0f);
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
