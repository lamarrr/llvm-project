// Port of https://www.shadertoy.com/view/DsG3Dt; author: uada.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DsG3Dt.json.
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
float random(Context &__ctx, f32x2 st) {
  return fract(sin(dot(st.swizzle<0, 1>(), f32x2(16.9898f, 98.233f))) *
               43758.5453123f);
}
f32x2 random2(Context &__ctx, f32x2 p) {
  return fract(
      sin(f32x2(dot(p, f32x2(127.1f, 311.7f)), dot(p, f32x2(269.5f, 183.3f)))) *
      43758.5453f);
}
float noise(Context &__ctx, f32x2 st) {
  f32x2 i = floor(st);
  f32x2 f = fract(st);
  float a = random(__ctx, i);
  float b = random(__ctx, i + f32x2(1.0f, 0.0f));
  float c = random(__ctx, i + f32x2(0.0f, 1.0f));
  float d = random(__ctx, i + f32x2(1.0f, 1.0f));
  f32x2 u = f * f * (3.0f - 2.0f * f);
  return mix(a, b, u.data[0]) + (c - a) * u.data[1] * (1.0f - u.data[0]) +
         (d - b) * u.data[0] * u.data[1];
}
float o_noise(Context &__ctx, f32x2 st, int octave, float s) {
  float r = 0.0f;
  while (octave > 0) {
    r += noise(__ctx, st) * s;
    s *= 0.5f;
    octave -= 1;
  }
  return r;
}
float so_noise(Context &__ctx, f32x2 st, int octave, float s) {
  float r = 0.0f;
  while (octave > 0) {
    r += noise(__ctx, st * s) * s;
    s *= 0.5f;
    octave -= 1;
  }
  return r;
}
float cell(Context &__ctx, f32x2 p, float time) {
  float min_dist = 1000.0f;
  f32x2 i_st = floor(p);
  f32x2 f_st = fract(p);
  for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
      f32x2 n = f32x2(float(x), float(y));
      f32x2 point = random2(__ctx, i_st + n);
      point = 0.5f + 0.5f * sin(time + 6.285f * point);
      float dist = length(n + point - f_st);
      min_dist = min(min_dist, dist);
    }
  }
  return min_dist;
}
f32x2 voronoi(Context &__ctx, f32x2 x, float time) {
  i32x2 p = i32x2(floor(x));
  f32x2 f = fract(x);
  f32x2 res = f32x2(8.0f);
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
      i32x2 b = i32x2(i, j);
      f32x2 point = random2(__ctx, f32x2(p + b));
      point = 0.5f + 0.5f * sin(time + 6.285f * point);
      f32x2 r = f32x2(b) - f + point;
      float d = dot(r, r);
      if (d < res.data[0]) {
        res.data[1] = res.data[0];
        res.data[0] = d;
      } else if (d < res.data[1]) {
        res.data[1] = d;
      }
    }
  return sqrt(res);
}
f32x2 voronoi(Context &__ctx, f32x2 x, float time, f32x2 &oA, f32x2 &oB) {
  i32x2 p = i32x2(floor(x));
  f32x2 f = fract(x);
  f32x2 res = f32x2(8.0f);
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
      i32x2 b = i32x2(i, j);
      f32x2 point = random2(__ctx, f32x2(p + b));
      point = 0.5f + 0.5f * sin(time + 6.285f * point);
      f32x2 r = f32x2(b) - f + point;
      float d = dot(r, r);
      if (d < res.data[0]) {
        res.data[1] = res.data[0];
        res.data[0] = d;
        oA = r;
      } else if (d < res.data[1]) {
        res.data[1] = d;
        oB = r;
      }
    }
  return sqrt(res);
}
float voronoi2_edge(Context &__ctx, f32x2 p, float time, float edge) {
  f32x2 a, b;
  f32x2 c = voronoi(__ctx, p, time, a, b);
  float d = dot(0.5f * (a + b), normalize(b - a));
  return 1.0f - smoothstep(0.0f, edge, d);
}
float voronoi_edge(Context &__ctx, f32x2 p, float time, float edge) {
  f32x2 c = voronoi(__ctx, p, time);
  float dis = c.data[1] - c.data[0];
  return 1.0f - smoothstep(0.0f, edge, dis);
}
float fbm(Context &__ctx, f32x2 x, float H) {
  float t = 0.0f;
  for (int i = 0; i < 8; i++) {
    float f = pow(2.0f, float(i));
    float a = pow(f, -H);
    t += a * noise(__ctx, f * x);
  }
  return t;
}
float saturate(Context &__ctx, float a) { return clamp(a, 0.0f, 1.0f); }
f32x3 saturate(Context &__ctx, f32x3 a) {
  return clamp(a, f32x3(0.0f), f32x3(1.0f));
}
float lava_shape(Context &__ctx, f32x3 pos, f32x2 uv, float time) {
  float d = distance(uv, f32x2(0.5f));
  float edge = 10000.0f;
  float falloff = 0.0125f;
  float s_d = 1.0f - smoothstep(edge - falloff, edge + falloff, d);
  f32x2 w_p = pos.swizzle<0, 2>() / 60.0f;
  w_p += o_noise(__ctx, w_p * 10.0f, 3, 0.995f) * 0.1f;
  f32x2 v = voronoi(__ctx, w_p, time);
  float rocks = v.data[1];
  rocks *= voronoi_edge(__ctx, w_p, time, 0.532f);
  float rock_slope = 0.125f;
  float rock_edge = 0.49f;
  rocks =
      smoothstep(rock_edge - rock_slope, rock_edge + rock_slope, 1.0f - rocks);
  rocks +=
      so_noise(__ctx,
               pos.swizzle<0, 2>() / 2.0f +
                   f32x2(so_noise(__ctx, pos.swizzle<0, 2>() / 4.0f, 3, 1.40f)),
               8, 0.50f) *
      0.84f * rocks;
  s_d = s_d * rocks;
  return s_d;
}
float cracked_lava_shape(Context &__ctx, f32x3 pos, float time) {
  f32x2 ep = f32x2(.8f);
  f32x2 xep = f32x2(-ep.data[1], ep.data[0]);
  f32x2 cell_p = pos.swizzle<0, 2>() / 90.0f;
  time *= 0.2f;
  cell_p += o_noise(__ctx, cell_p / 1.0f, 3, 4.0f) * 0.01f;
  cell_p += fbm(__ctx, cell_p / 10.0f, 0.5f);
  float rocks = voronoi(__ctx, cell_p, time).data[1];
  rocks = 1.0f - voronoi_edge(__ctx, cell_p, time, 0.28f);
  rocks = rocks * 2.50f;
  float rock_slope = .9125f;
  float rock_edge = 0.5f;
  rocks = saturate(
      __ctx, smoothstep(rock_edge - rock_slope, rock_edge + rock_slope, rocks));
  rocks +=
      o_noise(__ctx,
              cell_p * 5.0f + f32x2(o_noise(__ctx, cell_p * 4.0f, 3, 1.40f)), 3,
              0.50f) *
      0.84f * rocks;
  rocks += fbm(__ctx, cell_p / 3.0f, 0.75f) * rocks;
  return rocks;
}
float shape(Context &__ctx, f32x3 pos, float time) {
  return cracked_lava_shape(__ctx, pos, time);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 pos = f32x3(uv.data[0], 0.0f, uv.data[1]) * 500.0f;
  f32x3 col = f32x3(0.0f);
  float s = shape(__ctx, pos, iTime) / 1.0f;
  s = clamp(s, 0.0f, 1.0f);
  f32x4 lava_color = f32x4(0.9059f, 0.9333f, 0.0667f, 1.0f);
  f32x4 lava_mix = f32x4(0.788f, 0.055f, 0.055f, 1.0f);
  f32x4 dirt_color = f32x4(0.094f, 0.067f, 0.067f, 1.0f);
  lava_color =
      6.0f *
      mix(lava_color, lava_mix,
          saturate(__ctx, o_noise(__ctx,
                                  pos.swizzle<0, 2>() / 14.0f +
                                      o_noise(__ctx,
                                              pos.swizzle<0, 2>() / 40.0f +
                                                  f32x2(iTime),
                                              3, 1.0f),
                                  2, 1.0f)));
  col = mix(lava_color.swizzle<0, 1, 2>(), dirt_color.swizzle<0, 1, 2>(), s);
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
