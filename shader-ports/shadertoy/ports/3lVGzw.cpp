// Port of https://www.shadertoy.com/view/3lVGzw; author: ianwall.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/3lVGzw.json.
#include "../support.h"
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
f32x4 mod289(Context &__ctx, f32x4 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
f32x4 permute(Context &__ctx, f32x4 x) {
  return mod289(__ctx, ((x * 34.0f) + 1.0f) * x);
}
f32x4 taylorInvSqrt(Context &__ctx, f32x4 r) {
  return 1.79284291400159f - 0.85373472095314f * r;
}
f32x2 fade(Context &__ctx, f32x2 t) {
  return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}
float cnoise(Context &__ctx, f32x2 P) {
  f32x4 Pi = floor(P.swizzle<0, 1, 0, 1>()) + f32x4(0.0f, 0.0f, 1.0f, 1.0f);
  f32x4 Pf = fract(P.swizzle<0, 1, 0, 1>()) - f32x4(0.0f, 0.0f, 1.0f, 1.0f);
  Pi = mod289(__ctx, Pi);
  f32x4 ix = Pi.swizzle<0, 2, 0, 2>();
  f32x4 iy = Pi.swizzle<1, 1, 3, 3>();
  f32x4 fx = Pf.swizzle<0, 2, 0, 2>();
  f32x4 fy = Pf.swizzle<1, 1, 3, 3>();
  f32x4 i = permute(__ctx, permute(__ctx, ix) + iy);
  f32x4 gx = fract(i * (1.0f / 41.0f)) * 2.0f - 1.0f;
  f32x4 gy = abs(gx) - 0.5f;
  f32x4 tx = floor(gx + 0.5f);
  gx = gx - tx;
  f32x2 g00 = f32x2(gx.data[0], gy.data[0]);
  f32x2 g10 = f32x2(gx.data[1], gy.data[1]);
  f32x2 g01 = f32x2(gx.data[2], gy.data[2]);
  f32x2 g11 = f32x2(gx.data[3], gy.data[3]);
  f32x4 norm = taylorInvSqrt(
      __ctx, f32x4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
  g00 *= norm.data[0];
  g01 *= norm.data[1];
  g10 *= norm.data[2];
  g11 *= norm.data[3];
  float n00 = dot(g00, f32x2(fx.data[0], fy.data[0]));
  float n10 = dot(g10, f32x2(fx.data[1], fy.data[1]));
  float n01 = dot(g01, f32x2(fx.data[2], fy.data[2]));
  float n11 = dot(g11, f32x2(fx.data[3], fy.data[3]));
  f32x2 fade_xy = fade(__ctx, Pf.swizzle<0, 1>());
  f32x2 n_x = mix(f32x2(n00, n01), f32x2(n10, n11), fade_xy.data[0]);
  float n_xy = mix(n_x.data[0], n_x.data[1], fade_xy.data[1]);
  return 2.3f * n_xy;
}
float pnoise(Context &__ctx, f32x2 P, f32x2 rep) {
  f32x4 Pi = floor(P.swizzle<0, 1, 0, 1>()) + f32x4(0.0f, 0.0f, 1.0f, 1.0f);
  f32x4 Pf = fract(P.swizzle<0, 1, 0, 1>()) - f32x4(0.0f, 0.0f, 1.0f, 1.0f);
  Pi = mod(Pi, rep.swizzle<0, 1, 0, 1>());
  Pi = mod289(__ctx, Pi);
  f32x4 ix = Pi.swizzle<0, 2, 0, 2>();
  f32x4 iy = Pi.swizzle<1, 1, 3, 3>();
  f32x4 fx = Pf.swizzle<0, 2, 0, 2>();
  f32x4 fy = Pf.swizzle<1, 1, 3, 3>();
  f32x4 i = permute(__ctx, permute(__ctx, ix) + iy);
  f32x4 gx = fract(i * (1.0f / 41.0f)) * 2.0f - 1.0f;
  f32x4 gy = abs(gx) - 0.5f;
  f32x4 tx = floor(gx + 0.5f);
  gx = gx - tx;
  f32x2 g00 = f32x2(gx.data[0], gy.data[0]);
  f32x2 g10 = f32x2(gx.data[1], gy.data[1]);
  f32x2 g01 = f32x2(gx.data[2], gy.data[2]);
  f32x2 g11 = f32x2(gx.data[3], gy.data[3]);
  f32x4 norm = taylorInvSqrt(
      __ctx, f32x4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
  g00 *= norm.data[0];
  g01 *= norm.data[1];
  g10 *= norm.data[2];
  g11 *= norm.data[3];
  float n00 = dot(g00, f32x2(fx.data[0], fy.data[0]));
  float n10 = dot(g10, f32x2(fx.data[1], fy.data[1]));
  float n01 = dot(g01, f32x2(fx.data[2], fy.data[2]));
  float n11 = dot(g11, f32x2(fx.data[3], fy.data[3]));
  f32x2 fade_xy = fade(__ctx, Pf.swizzle<0, 1>());
  f32x2 n_x = mix(f32x2(n00, n01), f32x2(n10, n11), fade_xy.data[0]);
  float n_xy = mix(n_x.data[0], n_x.data[1], fade_xy.data[1]);
  return 2.3f * n_xy;
}
float radial = 1.00f;
float out_prop = 4.0f;
float strength = 1.7f;
float start = 0.0f;
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.0f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  uv *= 1.1f + start / 50.0f;
  float c = 0.0f;
  float r = 0.0f;
  float angle =
      mix(uv.data[0], 4.0f * atan(uv.data[1], uv.data[0]) / 3.1415926f, radial);
  for (float i = start; i < 50.0f + start; i += 1.0f) {
    float z = i / 50.0f;
    float str = mix(0.1f * strength, 0.05f * pow(1.f + z, strength), radial);
    f32x2 p = uv + z * str *
                       pnoise(__ctx, f32x2(angle, iTime + out_prop * z),
                              f32x2(2.0f, 100.0f));
    float d = mix(p.data[1] + 0.5f, length(p), radial);
    c += 1.0f - smoothstep(0.0f, 2.5f * fwidth(d - z), abs(d - z));
    r += c * angle;
  }
  f32x3 col = f32x3(f32x3(c));
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
