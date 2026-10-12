// Port of https://www.shadertoy.com/view/mtS3DR; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mtS3DR.json.
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
const int CHAR_A = 65;
const int CHAR_B = 66;
const int CHAR_C = 67;
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
int N = 63;
int maxdepth = 1024;
int bbit(Context &__ctx, int n) { return ~n & (n + 1); }
int check(Context &__ctx, i32x2 p) {
  int x = p.data[0], y = p.data[1];
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_B), 2)).data[0] != 0.0f)) {
    int bb = bbit(__ctx, x);
    assert(__ctx, bb == bbit(__ctx, y));
    int res = 1;
    while (bb != 0) {
      res = 1 - res;
      bb >>= 1;
    }
    return res;
  }
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_A), 2)).data[0] != 0.0f))
    return 0;
  unsigned h =
      ihash(__ctx, unsigned(x) ^ ihash(__ctx, unsigned(y) ^ unsigned(iTime)));
  return int(h & 1u);
}
int ttype(Context &__ctx, i32x2 ix) {
  int x = ix.data[0], y = ix.data[1];
  if (x < 0 || y < 0)
    return 0;
  if (x >= N || y >= N)
    return 0;
  int bbx = bbit(__ctx, x), bby = bbit(__ctx, y);
  if (bbx == bby && (bbx == 1 || bbx < maxdepth))
    return 1;
  int blocktype = -1;
  i32x2 block;
  if (bby > 1 && bby < maxdepth) {
    int bbx0 = bbit(__ctx, x - 1), bbx1 = bbit(__ctx, x + 1);
    if (bbx0 == bby) {
      blocktype = 0;
      block = i32x2(x - 1, y);
    }
    if (bbx1 == bby) {
      blocktype = 0;
      block = i32x2(x + 1, y);
    }
  }
  if (bbx > 1 && bbx < maxdepth) {
    int bby0 = bbit(__ctx, y - 1), bby1 = bbit(__ctx, y + 1);
    if (bbx == bby0) {
      blocktype = 1;
      block = i32x2(x, y - 1);
    }
    if (bbx == bby1) {
      blocktype = 1;
      block = i32x2(x, y + 1);
    }
  }
  if (blocktype < 0)
    return 0;
  return int(check(__ctx, block) == blocktype);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_X), 2)).data[0] != 0.0f))
    N = (1 << (int(mod(iTime, 8.0f) + 1.0f))) - 1;
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_Y), 2)).data[0] != 0.0f))
    maxdepth = 1 << int(mod(iTime, 8.0f));
  if ((texelFetch(__ctx.channel3, i32x2((CHAR_Z), 2)).data[0] != 0.0f))
    N = 2 * N + 1;
  float lwidth = 0.02f;
  f32x2 p = 1.05f * (2.0f * fragCoord - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  p += 1.0f;
  p *= 0.5f * float(N);
  float px = fwidth(p.data[0]);
  f32x3 col = f32x3(0.8f);
  if (min(p.data[0], p.data[1]) > -0.5f &&
      max(p.data[0], p.data[1]) < float(N) + 0.5f) {
    i32x2 ix = i32x2(floor(p));
    p = fract(p);
    int tt = ttype(__ctx, ix);
    float dx0 = p.data[0], dx1 = 1.0f - p.data[0];
    float dy0 = p.data[1], dy1 = 1.0f - p.data[1];
    float d = min(min(dx0, dx1), min(dy0, dy1));
    i32x2 delta;
    if (d == dx0)
      delta = i32x2(-1, 0);
    if (d == dx1)
      delta = i32x2(1, 0);
    if (d == dy0)
      delta = i32x2(0, -1);
    if (d == dy1)
      delta = i32x2(0, 1);
    int tt1 = ttype(__ctx, ix + delta);
    if (!(texelFetch(__ctx.channel3, i32x2((CHAR_C), 2)).data[0] != 0.0f) &&
        tt != 0)
      col = f32x3(1, 1, 0.1f);
    p = min(p, 1.0f - p);
    if (tt != tt1) {
      col *= smoothstep(0.0f, px, min(p.data[0], p.data[1]) - lwidth);
    }
    col *= smoothstep(0.0f, px, length(p) - 1.4f * lwidth);
  }
  col = pow(col, f32x3(0.4545f));
  if (alert)
    col.data[0] = 1.0f;
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
