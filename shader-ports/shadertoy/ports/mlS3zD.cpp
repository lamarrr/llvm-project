// Port of https://www.shadertoy.com/view/mlS3zD; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlS3zD.json.
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
  Texture2D channel0;
  Texture2D channel3;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const int CHAR_A = 65;
const int CHAR_B = 66;
const int CHAR_C = 67;
const int CHAR_X = 88;
const int CHAR_Y = 89;
const int CHAR_Z = 90;
const float PI = 3.14159f;
void swap(Context &__ctx, float &x, float &y) {
  float t = x;
  x = y;
  y = t;
}
void sort(Context &__ctx, float &x, float &y) {
  if (x > y)
    swap(__ctx, x, y);
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
float rand(Context &__ctx, i32x2 p) {
  unsigned h =
      ihash(__ctx, unsigned(p.data[0]) ^ ihash(__ctx, unsigned(p.data[1])));
  return float(h) / exp2(32.0f);
}
f32x3 h2rgb(Context &__ctx, float h) {
  f32x3 rgb =
      clamp(abs(mod(h * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
            0.0f, 1.0f);
  return rgb * rgb * (3.0f - 2.0f * rgb);
}
float regiondist(Context &__ctx, f32x2 p) {
  f32x2 repeat = f32x2(8, 5);
  p += repeat;
  p = mod(p, 2.0f * repeat);
  p -= repeat;
  p = abs(p);
  float d0 = max(p.data[0] - 5.0f, p.data[1] - 3.0f);
  float d1 = p.data[0] - 2.0f;
  return min(-d0, d1);
}
float offset(Context &__ctx, f32x2 p, bool flip, bool xoffset) {
  f32x2 delta = xoffset ? f32x2(0, 1) : f32x2(1, 0);
  f32x2 q0 = p - delta;
  f32x2 q1 = p + delta;
  if (flip) {
    p = p.swizzle<1, 0>();
    q0 = q0.swizzle<1, 0>();
    q1 = q1.swizzle<1, 0>();
  }
  if (regiondist(__ctx, p) > 0.0f)
    return 0.0f;
  if (regiondist(__ctx, p) == 0.0f &&
      (regiondist(__ctx, q0) == 0.0f || regiondist(__ctx, q1) == 0.0f))
    return 0.0f;
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_Y), 2)).data[0] != 0.0f))
    return 0.48f *
           (sin(p.data[0] + 0.5f * iTime) * sin(p.data[1] + 0.382f * iTime));
  float t = rand(__ctx, i32x2(p) ^ int(iTime));
  return 0.95f * (t - 0.5f);
}
float rect(Context &__ctx, f32x2 p, float x0, float y0, float x1, float y1) {
  return min(min(abs(p.data[0] - x0), abs(p.data[0] - x1)),
             min(abs(p.data[1] - y0), abs(p.data[1] - y1)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float lwidth = 0.01f;
  float scale = 6.0f;
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_Z), 2)).data[0] != 0.0f))
    scale *= 2.0f;
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (2.0f * iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  if (iMouse.data[0] > 0.0f)
    p -= m;
  p *= scale;
  f32x3 col = f32x3(0.1f);
  float px = fwidth(p.data[0]);
  float rdist = regiondist(__ctx, p);
  if (rdist > 0.0f) {
    if (!(texelFetch(__ctx.channel3, i32x2((CHAR_B), 2)).data[0] != 0.0f))
      col *= smoothstep(0.0f, px, rdist - lwidth);
  } else {
    f32x2 ix = round(p);
    p -= ix;
    bool flip = mod(ix.data[0] + ix.data[1], 2.0f) != 0.0f;
    if (flip) {
      p = p.swizzle<1, 0>();
      ix = ix.swizzle<1, 0>();
    }
    float x0 = offset(__ctx, ix, flip, true);
    float d = p.data[0] - x0;
    int xdelta = d < 0.0f ? -1 : 1;
    float y0 = offset(__ctx, ix + f32x2(xdelta, 0), flip, false);
    float d1 = p.data[1] - y0;
    int ydelta = d1 < 0.0f ? -1 : 1;
    float x1 =
        float(xdelta) + offset(__ctx, ix + f32x2(xdelta, ydelta), flip, true);
    float y1 =
        float(ydelta) + offset(__ctx, ix + f32x2(0, ydelta), flip, false);
    sort(__ctx, x0, x1);
    sort(__ctx, y0, y1);
    f32x2 uv = (p - f32x2(x0, y0)) / (f32x2(x1, y1) - f32x2(x0, y0));
    if (flip)
      uv = uv.swizzle<1, 0>();
    if (d < 0.0f)
      ix.data[0]--;
    if (d1 < 0.0f)
      ix.data[1]--;
    if (flip)
      ix = ix.swizzle<1, 0>();
    col = 0.25f + 0.75f * h2rgb(__ctx, rand(__ctx, i32x2(ix)));
    col *= length(texture(__ctx.channel0, uv).swizzle<0, 1, 2>());
    if ((texelFetch(__ctx.channel3, i32x2((CHAR_C), 2)).data[0] != 0.0f) &&
        flip)
      col *= 0.5f;
    if ((texelFetch(__ctx.channel3, i32x2((CHAR_X), 2)).data[0] != 0.0f))
      col *= 0.8f - 0.2f * cos(0.6f * PI / px * rect(__ctx, p, x0, y0, x1, y1));
    d = min(abs(d), abs(d1));
    if (!(texelFetch(__ctx.channel3, i32x2((CHAR_B), 2)).data[0] != 0.0f))
      col *= smoothstep(0.0f, px, d - lwidth);
  }
  col = pow(col, f32x3(0.4545f));
  fragColor = f32x4(col, 1);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
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
  Context __ctx{{channel0, sampling0}, {channel3, sampling3}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
