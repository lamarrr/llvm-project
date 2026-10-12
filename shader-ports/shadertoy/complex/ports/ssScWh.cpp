// Port of https://www.shadertoy.com/view/ssScWh; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssScWh.json.
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
float thc(Context &__ctx, float a, float b) {
  return tanh(a * cos(b)) / tanh(a);
}
float ths(Context &__ctx, float a, float b) {
  return tanh(a * sin(b)) / tanh(a);
}
f32x2 thc(Context &__ctx, float a, f32x2 b) {
  return tanh(a * cos(b)) / tanh(a);
}
f32x2 ths(Context &__ctx, float a, f32x2 b) {
  return tanh(a * sin(b)) / tanh(a);
}
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float h21(Context &__ctx, f32x2 a, float sc) {
  a = mod(a, sc);
  return fract(sin(dot(a, f32x2(12.9898f, 78.233f))) * 43758.5453123f);
}
float mlength(Context &__ctx, f32x2 uv) {
  return max(abs(uv.data[0]), abs(uv.data[1]));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
f32x3 pModSpherical(Context &__ctx, f32x3 p, f32x2 r) {
  f32x2 a = 3.14159f / r;
  float sc = 2.5f + thc(__ctx, 4.f, 8.f * length(p) + iTime);
  float axz =
      mod(atan(p.data[2], p.data[0]) + a.data[0], 2.f * a.data[0]) - a.data[0];
  p.set_swizzle<0, 2>(length(p.swizzle<0, 2>()) *
                      f32x2(thc(__ctx, sc, axz), ths(__ctx, sc, axz)));
  float axy =
      mod(atan(p.data[1], p.data[0]) + a.data[1], 2.f * a.data[1]) - a.data[1];
  p.set_swizzle<0, 1>(length(p.swizzle<0, 1>()) *
                      f32x2(thc(__ctx, sc, axy), ths(__ctx, sc, axy)));
  return p;
}
float box(Context &__ctx, f32x3 pos, f32x3 size) {
  f32x3 q = abs(pos) - size;
  return min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f) +
         length(max(q, 0.0f));
}
float scene(Context &__ctx, f32x3 p) {
  float time = p.data[1] + 0.5f * iTime;
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() *
                      (f32x2x2(cos(time), -sin(time), sin(time), cos(time))));
  p = pModSpherical(__ctx, p, f32x2(20.f + 10.f * thc(__ctx, 2.f, -time)));
  float d = length(p - f32x3(1.f, 0, 0)) - 0.32f;
  return 0.4f * d;
}
float scene2(Context &__ctx, f32x3 p) { return length(p) - 1.f; }
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(0.01f, 0);
  float d = scene(__ctx, p);
  return normalize(d - f32x3(scene(__ctx, p - e.swizzle<0, 1, 1>()),
                             scene(__ctx, p - e.swizzle<1, 0, 1>()),
                             scene(__ctx, p - e.swizzle<1, 1, 0>())));
}
float march(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float d = 0.f;
  f32x3 p;
  for (int i = 0; i < 400; i++) {
    p = ro + rd * d;
    float s = scene(__ctx, p);
    s *= z;
    d += s;
    if (d > 10.f || s < 0.01f)
      break;
  }
  return d;
}
f32x3 normal2(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(0.01f, 0);
  float d = scene2(__ctx, p);
  return normalize(d - f32x3(scene2(__ctx, p - e.swizzle<0, 1, 1>()),
                             scene2(__ctx, p - e.swizzle<1, 0, 1>()),
                             scene2(__ctx, p - e.swizzle<1, 1, 0>())));
}
float march2(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float d = 0.f;
  f32x3 p;
  for (int i = 0; i < 400; i++) {
    p = ro + rd * d;
    float s = scene2(__ctx, p);
    s *= z;
    d += s;
    if (d > 10.f || s < 0.01f)
      break;
  }
  return d;
}
f32x3 ray(Context &__ctx, f32x3 p, f32x3 l, f32x2 uv, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
f32x3 pixel(Context &__ctx, f32x3 p, f32x3 rd) {
  f32x3 n = normal(__ctx, p);
  float diffs = dot(n, -rd);
  float diff = max(diffs, 0.f);
  float fres = pow(1.f - abs(diffs), 4.f);
  float spec = pow(max(dot(reflect(-rd, n), rd), 0.f), 40.f);
  return f32x3(1) * (diff + fres) + spec;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0);
  f32x3 ro = f32x3(0, 0, -5);
  f32x3 rd = ray(__ctx, ro, f32x3(0), uv, 1.7f);
  float d0 = march(__ctx, ro, rd, 1.f);
  f32x3 p = ro + rd * d0;
  f32x3 n = normal(__ctx, p);
  float d1 = march2(__ctx, 0.2f * p + n * 0.01f, rd, -1.f);
  float dens = 1.5f;
  float foff = 0.5f;
  float thick = clamp(d1 / dens, 0.f, 1.f);
  thick = smoothstep(0.f, 1.f, thick);
  float blend = pow(thick, foff);
  f32x3 background = f32x3(1.f) + dot(rd, rd) * 2.8f;
  col = f32x3(0.4f);
  f32x3 e = f32x3(1.f);
  col = pal(__ctx, 0.32f, e, e, e, f32x3(0.f, 0.33f, 0.66f));
  col = mix(background, col, blend);
  fragColor = f32x4(col, 1.0f);
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
