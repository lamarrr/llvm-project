// Port of https://www.shadertoy.com/view/DllXzn; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DllXzn.json.
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
bool alert = false;
void assert(Context &__ctx, bool b) {
  if (!b)
    alert = true;
}
const float PI = 3.141592654f;
const float TWOPI = 2.0f * PI;
const int CHAR_A = 65;
const int CHAR_B = 66;
const int CHAR_C = 67;
const int CHAR_D = 68;
const int CHAR_W = 87;
const int CHAR_X = 88;
const int CHAR_Y = 89;
const int CHAR_Z = 90;
unsigned ihash(Context &__ctx, unsigned x) {
  x += 0x12345678u;
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}
float rand(Context &__ctx, i32x2 p) {
  unsigned h =
      ihash(__ctx, unsigned(p.data[0]) ^ ihash(__ctx, unsigned(p.data[1])));
  return float(h) / exp2(32.0f);
}
float rand(Context &__ctx, f32x2 p) { return rand(__ctx, i32x2(p)); }
f32x3 h2rgb(Context &__ctx, float h) {
  f32x3 rgb =
      clamp(abs(mod(h * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
            0.0f, 1.0f);
  return rgb * rgb * (3.0f - 2.0f * rgb);
}
bool coaxal(Context &__ctx, float &d, f32x2 p, float c, float lam) {
  if (abs(lam) > 1e4f) {
    d = abs(p.data[0]);
  } else {
    f32x2 centre = f32x2(-lam, 0);
    float radius = lam * lam - c;
    if (radius < 0.0f)
      return false;
    radius = sqrt(radius);
    d = length(p - centre) - radius;
  }
  return true;
}
float map(Context &__ctx, float lam, float t, float N) {
  float x = atan(lam);
  x /= 0.5f * PI;
  x += t;
  x *= N;
  return x;
}
float unmap(Context &__ctx, float x, float t, float N) {
  x /= N;
  x -= t;
  x *= 0.5f * PI;
  x = tan(x);
  return x;
}
float pencil(Context &__ctx, f32x2 p, float c, float t, float N, float &ix) {
  float d = 1e8f, d1;
  float lam = -(dot(p, p) + c) / (2.0f * p.data[0]);
  lam = map(__ctx, lam, t, N);
  float lam0 = round(lam);
  if (coaxal(__ctx, d1, p, c, unmap(__ctx, lam0, t, N)))
    d = min(d, abs(d1));
  float lam1 = lam0 + ((lam - lam0) >= 0.0f ? 1.0f : -1.0f);
  if (coaxal(__ctx, d1, p, c, unmap(__ctx, lam1, t, N)))
    d = min(d, abs(d1));
  ix = min(lam0, lam1);
  if (mod(ix, 2.0f) == 0.0f)
    d = -d;
  return d;
}
float xor (Context & __ctx, float d0, float d1) {
  return max(min(d0, d1), -max(d0, d1));
} float ssqr(Context &__ctx, float x) {
  return sign(x) * x * x;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 fg = f32x3(0.5f);
  float scale = 3.0f;
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  p *= scale;
  float px = fwidth(p.data[0]);
  float c = 1.0f, N = 10.0f;
  if (iMouse.data[0] > 0.0f) {
    c = ssqr(__ctx, 8.0f * (iMouse.data[0] / iResolution.data[0] - 0.5f));
    N = 1.0f + floor(20.0f * iMouse.data[1] / iResolution.data[1]);
  }
  float t = mod(iTime / N, 2.0f);
  float ix0, ix1;
  float d = pencil(__ctx, p.swizzle<1, 0>(), c, t, N, ix0);
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_X), 2)).data[0] != 0.0f))
    d = xor(__ctx, d, pencil(__ctx, p.swizzle<0, 1>(), -c, 0.0f, N, ix1));
  f32x3 col = f32x3(1.0f);
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_C), 2)).data[0] != 0.0f))
    fg = h2rgb(__ctx, rand(__ctx, mod(f32x2(ix0, ix1), 2.0f * N)));
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_A), 2)).data[0] != 0.0f))
    col = mix(fg, col, smoothstep(-0.5f * px, 0.5f * px, d));
  float k = 0.5f;
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_D), 2)).data[0] != 0.0f))
    col *= 0.75f - 0.25f * cos(k * PI * d / px);
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_B), 2)).data[0] != 0.0f))
    col *= smoothstep(0.0f, px, abs(d) - 0.01f);
  col = pow(col, f32x3(0.4545f));
  if (alert)
    col.data[0] = 1.0f;
  fragColor = f32x4(col, 1);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 3) texture_2d<f32x4> channel3,
               __binding__(1, 3) sampler sampling3) {
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
