// Port of https://www.shadertoy.com/view/ctS3zV; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctS3zV.json.
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
struct Context {
  Texture2D channel3;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const int CHAR_A = 65;
const int CHAR_C = 67;
const int CHAR_X = 88;
const int CHAR_Y = 89;
const int CHAR_Z = 90;
const float PI = 3.141592654f;
f32x3 hsv2rgb(Context &__ctx, float h, float s, float v) {
  f32x3 rgb =
      clamp(abs(mod(h * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
            0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return v * mix(f32x3(1.0f), rgb, s);
}
unsigned ihash(Context &__ctx, unsigned x) {
  x += 0x12345678u;
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}
float circle(Context &__ctx, f32x2 p, f32x2 centre, float r) {
  p -= centre;
  return length(p) - r;
}
float xor (Context & __ctx, float d0, float d1) {
  return min(max(d0, d1), max(-d0, -d1));
} f32x2 dup(Context &__ctx, f32x2 p, float k) {
  return mod(p + k, 2.0f * k) - k;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float scale = 10.0f;
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_Z), 2)).data[0] != 0.0f))
    scale *= 2.0f;
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  p *= scale;
  float px = 0.5f * fwidth(p.data[0]);
  float d = 1e8f;
  float t = -log2(length(p));
  float loop = 64.0f;
  t += mod(0.5f * iTime, loop);
  t = round(t);
  int t0 = int(mod(t, loop));
  t -= mod(0.5f * iTime, loop);
  float r = exp2(-t);
  float k = 0.25f;
  k *= 2.0f * float(ihash(__ctx, unsigned(t0))) / exp2(32.0f);
  k = exp2(k);
  float e = circle(__ctx, p, f32x2(0), r / k);
  float f = circle(__ctx, p, f32x2(0), r * k);
  d = min(d, max(f, -e));
  f32x3 fg = f32x3(0.5f);
  f32x3 bg = f32x3(1);
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_C), 2)).data[0] != 0.0f) &&
      d < 0.0f) {
    float h = float(ihash(__ctx, unsigned(t0 ^ 1))) / exp2(32.0f);
    float inc =
        iMouse.data[0] <= 0.0f ? 0.5f : iMouse.data[0] / iResolution.data[0];
    float s =
        iMouse.data[0] <= 0.0f ? 1.0f : iMouse.data[1] / iResolution.data[1];
    fg = hsv2rgb(__ctx, h, s, 1.0f);
    bg = hsv2rgb(__ctx, h + inc, s, 1.0f);
  }
  f32x2 p1 = dup(__ctx, p, 1.0f);
  float d1 = circle(__ctx, p1, f32x2(0), 1.0f);
  d = xor(__ctx, d, d1);
  float s = max(abs(p.data[0]), abs(p.data[1]));
  s -= 2.0f;
  s *= 0.25f;
  s = round(s);
  s /= 0.25f;
  s += 2.0f;
  e = max(abs(p.data[1]) - s + 1.0f, abs(p.data[0]) - s + 1.0f);
  f = max(abs(p.data[1]) - s - 1.0f, abs(p.data[0]) - s - 1.0f);
  d = xor(__ctx, d, max(f, -e));
  f32x3 col = mix(fg, bg, smoothstep(-px, px, d));
  col *= smoothstep(0.0f, 2.0f * px, abs(d) - 0.01f);
  float dir =
      mod(0.5f * dot(round(p - p1), f32x2(1)), 2.0f) < 0.5f ? -1.0f : 1.0f;
  if (length(p1) < 1.0f)
    col *= 0.5f -
           0.5f * cos(0.3f * PI * d1 / px +
                      2.0f * dir * (PI * iTime + atan(p1.data[1], p1.data[0])));
  col = pow(col, f32x3(0.4545f));
  fragColor = f32x4(col, 1);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 3) texture_2d<f32x4> channel3,
               __binding(1, 3) sampler sampling3) {
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
  Context __ctx{{channel3, sampling3}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
