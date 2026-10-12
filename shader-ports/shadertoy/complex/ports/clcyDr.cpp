// Port of https://www.shadertoy.com/view/clcyDr; author: cedric_h.
// Dataset license metadata: mit; original retained in upstream/clcyDr.json.
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
f32x2 hash2(Context &__ctx, f32x2 p) {
  return textureLod(__ctx.channel0, (p + 0.5f) / 256.0f, 0.0f).swizzle<0, 1>();
}
f32x3 voronoi(Context &__ctx, f32x2 x) {
  f32x2 n = floor(x);
  f32x2 f = fract(x);
  f32x2 mg, mr;
  float md = 8.0f;
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
      f32x2 g = f32x2(float(i), float(j));
      f32x2 o = hash2(__ctx, n + g);
      o = 0.5f + 0.5f * sin(0.1f * iTime + 6.2831f * o);
      f32x2 r = g + o - f;
      float d = dot(r, r);
      if (d < md) {
        md = d;
        mr = r;
        mg = g;
      }
    }
  md = 8.0f;
  for (int j = -2; j <= 2; j++)
    for (int i = -2; i <= 2; i++) {
      f32x2 g = mg + f32x2(float(i), float(j));
      f32x2 o = hash2(__ctx, n + g);
      o = 0.5f + 0.5f * sin(0.1f * iTime + 6.2831f * o);
      f32x2 r = g + o - f;
      if (dot(mr - r, mr - r) > 0.00001f)
        md = min(md, dot(0.5f * (mr + r), normalize(r - mr)));
    }
  return f32x3(md, mr);
}
const f32x2x2 m = f32x2x2(0.80f, 0.60f, -0.60f, 0.80f);
float noise(Context &__ctx, f32x2 p) { return sin(p.data[0]) * sin(p.data[1]); }
float fbm6(Context &__ctx, f32x2 p) {
  float f = 0.0f;
  f += 0.500000f * (0.5f + 0.5f * noise(__ctx, p));
  p = m * p * 2.02f;
  f += 0.250000f * (0.5f + 0.5f * noise(__ctx, p));
  p = m * p * 2.03f;
  f += 0.125000f * (0.5f + 0.5f * noise(__ctx, p));
  p = m * p * 2.01f;
  f += 0.062500f * (0.5f + 0.5f * noise(__ctx, p));
  p = m * p * 2.04f;
  f += 0.031250f * (0.5f + 0.5f * noise(__ctx, p));
  p = m * p * 2.01f;
  f += 0.015625f * (0.5f + 0.5f * noise(__ctx, p));
  return f / 0.96875f;
}
f32x2 fbm6_2(Context &__ctx, f32x2 p) {
  return f32x2(fbm6(__ctx, p + f32x2(16.8f)), fbm6(__ctx, p + f32x2(11.5f)));
}
float aastep(Context &__ctx, float threshold, float value) {
  float afwidth = length(f32x2(ddx(value), ddy(value))) * 0.70710678118654757f;
  return smoothstep(threshold - afwidth, threshold + afwidth, value);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = 1.4f * fragCoord / iResolution.data[0];
  p += 0.24f * fbm6(__ctx, (p * 2.0f + iTime * 0.15f));
  f32x3 c = voronoi(__ctx, 8.0f * p);
  f32x3 col = f32x3(0.0f, 0.6f, 0.9f);
  col = mix(f32x3(0.6f, 0.8f, 0.99f), col,
            smoothstep(0.0075f, 0.025f, c.data[0]));
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
