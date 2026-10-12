// Port of https://www.shadertoy.com/view/DtfBRn; author: beans_please.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtfBRn.json.
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
float lerp(Context &__ctx, float a, float b, float t) {
  return a + t * (b - a);
}
f32x2 lerp(Context &__ctx, f32x2 a, f32x2 b, float t) {
  return a + t * (b - a);
}
f32x3 lerp(Context &__ctx, f32x3 a, f32x3 b, float t) {
  return a + t * (b - a);
}
f32x4 lerp(Context &__ctx, f32x4 a, f32x4 b, float t) {
  return a + t * (b - a);
}
float wrap(Context &__ctx, float inp, float start, float end) {
  return start + mod(inp - start, end - start);
}
f32x2 wrap(Context &__ctx, f32x2 inp, float start, float end) {
  return start + mod(inp - start, end - start);
}
f32x3 wrap(Context &__ctx, f32x3 inp, float start, float end) {
  return start + mod(inp - start, end - start);
}
f32x4 wrap(Context &__ctx, f32x4 inp, float start, float end) {
  return start + mod(inp - start, end - start);
}
float remap(Context &__ctx, float inp, float inp_start, float inp_end,
            float out_start, float out_end) {
  return out_start +
         ((out_end - out_start) / (inp_end - inp_start)) * (inp - inp_start);
}
f32x2 remap(Context &__ctx, f32x2 inp, float inp_start, float inp_end,
            float out_start, float out_end) {
  return out_start +
         ((out_end - out_start) / (inp_end - inp_start)) * (inp - inp_start);
}
f32x3 remap(Context &__ctx, f32x3 inp, float inp_start, float inp_end,
            float out_start, float out_end) {
  return out_start +
         ((out_end - out_start) / (inp_end - inp_start)) * (inp - inp_start);
}
f32x4 remap(Context &__ctx, f32x4 inp, float inp_start, float inp_end,
            float out_start, float out_end) {
  return out_start +
         ((out_end - out_start) / (inp_end - inp_start)) * (inp - inp_start);
}
float remap_clamp(Context &__ctx, float inp, float inp_start, float inp_end,
                  float out_start, float out_end) {
  float t = clamp((inp - inp_start) / (inp_end - inp_start), 0.0f, 1.0f);
  return out_start + t * (out_end - out_start);
}
f32x2 remap_clamp(Context &__ctx, f32x2 inp, float inp_start, float inp_end,
                  float out_start, float out_end) {
  f32x2 t = clamp((inp - inp_start) / (inp_end - inp_start), 0.0f, 1.0f);
  return out_start + t * (out_end - out_start);
}
f32x3 remap_clamp(Context &__ctx, f32x3 inp, float inp_start, float inp_end,
                  float out_start, float out_end) {
  f32x3 t = clamp((inp - inp_start) / (inp_end - inp_start), 0.0f, 1.0f);
  return out_start + t * (out_end - out_start);
}
f32x4 remap_clamp(Context &__ctx, f32x4 inp, float inp_start, float inp_end,
                  float out_start, float out_end) {
  f32x4 t = clamp((inp - inp_start) / (inp_end - inp_start), 0.0f, 1.0f);
  return out_start + t * (out_end - out_start);
}
float remap01(Context &__ctx, float inp, float inp_start, float inp_end) {
  return clamp((inp - inp_start) / (inp_end - inp_start), 0.0f, 1.0f);
}
f32x2 remap01(Context &__ctx, f32x2 inp, float inp_start, float inp_end) {
  return clamp((inp - inp_start) / (inp_end - inp_start), 0.0f, 1.0f);
}
f32x3 remap01(Context &__ctx, f32x3 inp, float inp_start, float inp_end) {
  return clamp((inp - inp_start) / (inp_end - inp_start), 0.0f, 1.0f);
}
f32x4 remap01(Context &__ctx, f32x4 inp, float inp_start, float inp_end) {
  return clamp((inp - inp_start) / (inp_end - inp_start), 0.0f, 1.0f);
}
f32x2x2 rotate_2d(Context &__ctx, float angle) {
  return f32x2x2(cos(angle), sin(angle),
                 cos(angle + 1.57079632679489661923132169163f),
                 sin(angle + 1.57079632679489661923132169163f));
}
unsigned prng_state[2];
unsigned prng_rot(Context &__ctx, unsigned x, int k) {
  return (x << k) | (x >> (32 - k));
}
unsigned randomui(Context &__ctx) {
  unsigned s0 = prng_state[0];
  unsigned s1 = prng_state[1];
  unsigned result = prng_rot(__ctx, s0 * 0x9e3779bbu, 5) * 5u;
  s1 ^= s0;
  prng_state[0] = prng_rot(__ctx, s0, 26) ^ s1 ^ (s1 << 9);
  prng_state[1] = prng_rot(__ctx, s1, 13);
  return result;
}
int randomi(Context &__ctx) { return int(randomui(__ctx) % 0x7FFFFFFFu); }
float random(Context &__ctx) {
  return float(randomui(__ctx)) / float(0xffffffffu);
}
f32x2 random_gauss(Context &__ctx) {
  float u1 = random(__ctx);
  float u2 = random(__ctx) * 6.283185307179586476925286766f;
  float temp = sqrt(-2.f * log(u1));
  return temp * f32x2(cos(u2), sin(u2));
}
f32x2 random_unit_vec2(Context &__ctx) {
  f32x2 v = f32x2(1);
  float lensqr;
  for (int i = 0; i < 20; i++) {
    v = f32x2(2.f * random(__ctx) - 1.f, 2.f * random(__ctx) - 1.f);
    lensqr = dot(v, v);
    if (lensqr == 0.f)
      i--;
    else if (lensqr <= 1.f)
      break;
  }
  return v / sqrt(lensqr);
}
f32x3 random_unit_vec3(Context &__ctx) {
  f32x3 v = f32x3(1);
  float lensqr;
  for (int i = 0; i < 20; i++) {
    v = f32x3(2.f * random(__ctx) - 1.f, 2.f * random(__ctx) - 1.f,
              2.f * random(__ctx) - 1.f);
    lensqr = dot(v, v);
    if (lensqr == 0.f)
      i--;
    else if (lensqr <= 1.f)
      break;
  }
  return v / sqrt(lensqr);
}
f32x2 random_in_circle(Context &__ctx) {
  f32x2 v = f32x2(1);
  for (int i = 0; i < 20; i++) {
    v = f32x2(2.f * random(__ctx) - 1.f, 2.f * random(__ctx) - 1.f);
    if (dot(v, v) <= 1.f)
      return v;
  }
  return v;
}
f32x3 random_in_sphere(Context &__ctx) {
  f32x3 v = f32x3(1);
  for (int i = 0; i < 20; i++) {
    v = f32x3(2.f * random(__ctx) - 1.f, 2.f * random(__ctx) - 1.f,
              2.f * random(__ctx) - 1.f);
    if (dot(v, v) <= 1.f)
      return v;
  }
  return v;
}
void prng_init(Context &__ctx, u32x2 seed) {
  seed += u32x2(1317, 944573125);
  seed *= 464973573u;
  prng_state[0] = seed.data[0];
  prng_state[1] = seed.data[1];
  randomi(__ctx);
}
void prng_init(Context &__ctx, unsigned seed) {
  prng_init(__ctx, u32x2(seed, 1));
}
void prng_init(Context &__ctx, f32x3 seed) {
  seed += 3.49276101561702f;
  seed.set_swizzle<0, 1>(seed.swizzle<0, 1>() * ((seed.data[2] + 10.258f)));
  prng_state[0] = float_bits_to_uint(seed.data[0]);
  prng_state[1] = float_bits_to_uint(seed.data[1]);
  randomui(__ctx);
}
void prng_init(Context &__ctx, f32x2 seed) { prng_init(__ctx, f32x3(seed, 1)); }
void prng_init(Context &__ctx, float seed) {
  prng_init(__ctx, f32x3(seed, 1, 1));
}
f32x2 screen_to_uv(Context &__ctx, f32x2 coord) {
  return (2.f * coord - iResolution.swizzle<0, 1>()) /
         min(iResolution.data[0], iResolution.data[1]);
}
void mainImage(Context &__ctx, f32x4 &frag_col, f32x2 frag_coord) {
  f32x2 uv = screen_to_uv(__ctx, frag_coord);
  f32x3 col = f32x3(uv, 0);
  col = pow(col, f32x3(1.f / 2.2f));
  frag_col = f32x4(col, 1.f);
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
