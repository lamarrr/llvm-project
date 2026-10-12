// Port of https://www.shadertoy.com/view/7tyyWc; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tyyWc.json.
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
f32x4 blackbody(Context &__ctx, float x) {
  f32x4 O = f32x4(0.0f);
  float m = .01f + 5.f * 2.0f, T = x * 16000.f;
  O -= O;
  for (float i = 0.f; i < 3.f; i += .05f) {
    float f = 1.f + .5f * i;
    O[int(i)] += 10.f / m * (f * f * f) / (exp((19E3f * f / T)) - 1.f);
  }
  return O;
}
float erp(Context &__ctx, float x, float k) {
  return x < 0.5f ? pow(2.0f * x, k) * 0.5f
                  : 1.f - pow(2.0f * (1.0f - x), k) / 2.f;
}
float hash2(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float noise(Context &__ctx, f32x2 p) {
  float tl = hash2(__ctx, f32x2(floor(p.data[0]), ceil(p.data[1])));
  float bl = hash2(__ctx, f32x2(floor(p.data[0]), floor(p.data[1])));
  float tr = hash2(__ctx, f32x2(ceil(p.data[0]), ceil(p.data[1])));
  float br = hash2(__ctx, f32x2(ceil(p.data[0]), floor(p.data[1])));
  float a = mix(bl, tl, erp(__ctx, fract(p.data[1]), 1.5f));
  float b = mix(br, tr, erp(__ctx, fract(p.data[1]), 1.5f));
  return mix(a, b, erp(__ctx, fract(p.data[0]), 1.5f));
}
float join(Context &__ctx, float a, float b) { return min(a, b); }
float sdOrientedBox(Context &__ctx, f32x2 p, f32x2 a, f32x2 b, float th) {
  float l = length(b - a);
  f32x2 d = (b - a) / l;
  f32x2 q = (p - (a + b) * 0.5f);
  q = f32x2x2(d.data[0], -d.data[1], d.data[1], d.data[0]) * q;
  q = abs(q) - f32x2(l, th) * 0.5f;
  return length(max(q, 0.0f)) + min(max(q.data[0], q.data[1]), 0.0f);
}
float sdDisk(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float sdDiskOutline(Context &__ctx, f32x2 p, float r1, float r2) {
  return abs(length(p) - r1) - r2 * 0.5f;
}
float sdRotatedLine(Context &__ctx, f32x2 p, f32x2 start, float offset,
                    float len, float theta, float thick) {
  return sdOrientedBox(__ctx, p, start + f32x2(sin(theta), cos(theta)) * offset,
                       f32x2(sin(theta), cos(theta)) * (len + offset), thick);
}
f32x4 sd(Context &__ctx, f32x2 p) {
  float seconds = sdRotatedLine(
      __ctx, p, f32x2(0.0f, 0.0f), 0.0f, 0.15f,
      (floor((iDate * 1.0f).data[3]) / 60.0f) * 2.0f * 3.141592653f, -0.001f);
  float minutes = sdRotatedLine(
      __ctx, p, f32x2(0.0f, 0.0f), 0.0f, 0.11f,
      ((iDate * 1.0f).data[3] / 60.0f / 60.0f) * 2.0f * 3.141592653f, 0.0f);
  float hours = sdRotatedLine(__ctx, p, f32x2(0.0f, 0.0f), 0.0f, 0.1f,
                              ((iDate * 1.0f).data[3] / 60.0f / 60.0f / 12.0f) *
                                  2.0f * 3.141592653f,
                              0.0f);
  float centerDisk = sdDisk(__ctx, p, 0.0f);
  float outerCircle = sdDiskOutline(__ctx, p, 0.23f, 0.0f);
  float ticks = 1000000.0f;
  for (float i = 0.0f; i < 2.0f * 3.141592653f;
       i += 2.0f * 3.141592653f / 12.0f) {
    ticks =
        join(__ctx, ticks,
             sdRotatedLine(__ctx, p, f32x2(0.0f, 0.0f), 0.18f, 0.03f, i, 0.0f));
  }
  for (float i = 0.0f; i < 2.0f * 3.141592653f;
       i += 2.0f * 3.141592653f / 60.0f) {
    ticks = join(
        __ctx, ticks,
        sdRotatedLine(__ctx, p, f32x2(0.0f, 0.0f), 0.2f, 0.01f, i, -0.005f));
  }
  return f32x4(
      join(__ctx,
           join(__ctx,
                join(__ctx, join(__ctx, join(__ctx, seconds, minutes), hours),
                     centerDisk),
                outerCircle),
           ticks),
      0.0f, 0.0f, 0.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float ratio = iResolution.data[1] / iResolution.data[0];
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv -= f32x2(0.5f);
  uv.data[1] *= ratio;
  float px = 1.0f / ratio / max(iResolution.data[0], iResolution.data[1]);
  f32x4 col =
      blackbody(__ctx, .005f / max(0.0f, sd(__ctx, uv).data[0] + 0.01f));
  col += max(f32x4(0.0f),
             blackbody(
                 __ctx,
                 erp(__ctx,
                     abs(fragCoord.data[1] / iResolution.data[1] - 0.5f) * 2.0f,
                     0.2f) *
                     0.5f)) *
         1.0f;
  fragColor = col;
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
