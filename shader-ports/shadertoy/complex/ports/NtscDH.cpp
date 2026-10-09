// Port of https://www.shadertoy.com/view/NtscDH; author: elvi5tim.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NtscDH.json.
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
const float F3 = 0.3333333f;
const float G3 = 0.1666667f;
f32x3 random3(Context &__ctx, f32x3 c) {
  float j = 4096.0f * sin(dot(c, f32x3(17.0f, 59.4f, 15.0f)));
  f32x3 r;
  r.data[2] = fract(512.0f * j);
  j *= .125f;
  r.data[0] = fract(512.0f * j);
  j *= .125f;
  r.data[1] = fract(512.0f * j);
  return r - 0.5f;
}
float simplex3d(Context &__ctx, f32x3 p) {
  f32x3 s = floor(p + dot(p, f32x3(F3)));
  f32x3 x = p - s + dot(s, f32x3(G3));
  f32x3 e = step(f32x3(0.0f), x - x.swizzle<1, 2, 0>());
  f32x3 i1 = e * (1.0f - e.swizzle<2, 0, 1>());
  f32x3 i2 = 1.0f - e.swizzle<2, 0, 1>() * (1.0f - e);
  f32x3 x1 = x - i1 + G3;
  f32x3 x2 = x - i2 + 2.0f * G3;
  f32x3 x3 = x - 1.0f + 3.0f * G3;
  f32x4 w, d;
  w.data[0] = dot(x, x);
  w.data[1] = dot(x1, x1);
  w.data[2] = dot(x2, x2);
  w.data[3] = dot(x3, x3);
  w = max(0.6f - w, 0.0f);
  d.data[0] = dot(random3(__ctx, s), x);
  d.data[1] = dot(random3(__ctx, s + i1), x1);
  d.data[2] = dot(random3(__ctx, s + i2), x2);
  d.data[3] = dot(random3(__ctx, s + 1.0f), x3);
  w *= w;
  w *= w;
  d *= w;
  return dot(d, f32x4(52.0f));
}
float fbm(Context &__ctx, f32x3 p) {
  float f = 0.0f;
  float frequency = 1.0f;
  float amplitude = 0.5f;
  for (int i = 0; i < 4; i++) {
    f += simplex3d(__ctx, p * frequency) * amplitude;
    amplitude *= 0.5f;
    frequency *= 2.0f + float(i) / 100.0f;
  }
  return min(f, 1.0f);
}
float random(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(12.9798f, 78.323f))) *
               43858.5563313f);
}
f32x2 rectToPolar(Context &__ctx, f32x2 p, f32x2 ms) {
  p -= ms / 2.0f;
  const float PI = 3.1415926534f;
  float r = length(p);
  float a = ((atan(p.data[1], p.data[0]) / PI) * 0.5f + 0.5f) * ms.data[0];
  return f32x2(a, r);
}
float line(Context &__ctx, float v, float from, float to, float f) {
  float d = max(from - v, v - to);
  return 1.0f - smoothstep(0.0f, f, d);
}
float effect(Context &__ctx, f32x2 p, float o) {
  p *= 2.0f;
  float f1 =
      simplex3d(__ctx, f32x3(p * f32x2(1.0f, 5.0f), iTime * 0.05f)) * 0.5f +
      0.5f;
  float e = fbm(__ctx, f32x3(p * f32x2(15.0f, 1.0f) + f32x2(f1 * 0.85f, o),
                             iTime * .005f));
  e = abs(e) * sqrt(p.data[1] / 5.0f);
  float c2 = simplex3d(__ctx, f32x3(p * f32x2(6.0f, 2.0f), iTime * 0.05f));
  c2 = (c2 * 0.5f) + 0.5f;
  c2 *= 0.5f;
  e += c2;
  return e * 0.5f;
}
float sw(Context &__ctx, f32x2 p, f32x2 ms) {
  p = rectToPolar(__ctx, p, ms);
  p.data[0] = mod(p.data[0] + 0.5f, ms.data[0]);
  const float b = 0.5f;
  const float d = 0.04f;
  float seem = line(__ctx, p.data[0], -1.0f, d, b) +
               line(__ctx, p.data[0], ms.data[0] - d, ms.data[0] + 1.0f, b);
  seem = min(seem, 1.0f);
  float s1 = effect(__ctx, p, 0.0f);
  p.data[0] = mod(p.data[0] + 0.6f, ms.data[0]);
  float s2 = effect(__ctx, p, -1020.0f);
  float s = s1;
  s = mix(s1, s2, seem);
  float perc = min(max(abs(sin(iTime * 0.1f)), iTime * 0.1f), 1.0f);
  float f1 = perc * 0.25f;
  float f2 = perc * 1.f;
  float m = line(__ctx, p.data[1], -0.1f, f1 + s * f2, 0.2f);
  return smoothstep(0.31f, 0.6f, m);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = fragCoord.swizzle<0, 1>() / iResolution.swizzle<1, 1>();
  float m = iResolution.data[0] / iResolution.data[1];
  f32x2 ms = f32x2(m, 1.0f);
  float c = 0.0f;
  float s = sw(__ctx, p, ms);
  c += s;
  float t = random(__ctx, p * 4.0f);
  float shade = fbm(__ctx, f32x3(p * 3.0f, iTime * 0.1f)) * 0.5f + 0.5f;
  shade = sqrt(pow(shade * 0.8f, 5.5f));
  f32x3 pic = f32x3(shade);
  f32x3 col = mix(f32x3(0.95f, 0.96f, 0.8f), pic, c);
  col -= (1.0f - s) * t * 0.04f;
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
