// Port of https://www.shadertoy.com/view/ctj3zm; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctj3zm.json.
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
const int CHAR_X = 88;
const int CHAR_Y = 89;
const int CHAR_Z = 90;
const float PI = 3.4159f;
unsigned ihash(Context &__ctx, unsigned x) {
  x += 0x12345678u;
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}
f32x3 h2rgb(Context &__ctx, float h) {
  f32x3 rgb =
      clamp(abs(mod(h * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
            0.0f, 1.0f);
  return rgb * rgb * (3.0f - 2.0f * rgb);
}
float rectdist(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  return max(max(a.data[0] - p.data[0], p.data[0] - b.data[0]),
             max(a.data[1] - p.data[1], p.data[1] - b.data[1]));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  const float phi = 0.5f * (sqrt(5.0f) - 1.0f);
  float lwidth = 0.001f;
  f32x2 p = 1.1f * (2.0f * fragCoord - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  float px = fwidth(p.data[0]);
  f32x3 col = f32x3(1, 1, 0.5f);
  f32x2 a = f32x2(-1.0f - phi, -1);
  f32x2 b = f32x2(1.0f + phi, 1);
  int type = 1;
  float d = rectdist(__ctx, p, a, b);
  if (d < 0.0f) {
    unsigned h = unsigned(iTime);
    int N = 11;
    if ((texelFetch(__ctx.channel3, i32x2((CHAR_Z), 2)).data[0] != 0.0f)) {
      N = int(iTime) % 14;
      h = unsigned(iTime) / 14u;
    }
    for (int i = 0;
         (texelFetch(__ctx.channel3, i32x2((CHAR_Y), 2)).data[0] != 0.0f)
             ? length(a - b) > 0.1f
             : i < N || type == 2;
         i++, h = ihash(__ctx, h ^ unsigned(type))) {
      if (type == 0) {
        bool flip = bool(h & 1u);
        if (flip) {
          p = p.swizzle<1, 0>();
          a = a.swizzle<1, 0>();
          b = b.swizzle<1, 0>();
        }
        bool reverse = bool(h & 2u);
        if (reverse) {
          float x1 = phi * a.data[0] + (1.0f - phi) * b.data[0];
          if (p.data[0] < x1) {
            b = f32x2(x1, b.data[1]);
            type = 2;
          } else {
            a = f32x2(x1, a.data[1]);
            type = 1;
          }
        } else {
          float x1 = (1.0f - phi) * a.data[0] + phi * b.data[0];
          if (p.data[0] < x1) {
            b = f32x2(x1, b.data[1]);
            type = 1;
          } else {
            a = f32x2(x1, a.data[1]);
            type = 2;
          }
        }
        if (flip) {
          p = p.swizzle<1, 0>();
          a = a.swizzle<1, 0>();
          b = b.swizzle<1, 0>();
        }
      } else {
        bool flip = b.data[1] - a.data[1] > b.data[0] - a.data[0];
        if (flip) {
          p = p.swizzle<1, 0>();
          a = a.swizzle<1, 0>();
          b = b.swizzle<1, 0>();
        }
        bool reverse = bool(h & 1u);
        if (reverse) {
          float x1 = b.data[0] - (b.data[1] - a.data[1]);
          if (p.data[0] < x1) {
            b = f32x2(x1, b.data[1]);
            type = 1;
          } else {
            a = f32x2(x1, a.data[1]);
            type = 0;
          }
        } else {
          float x1 = a.data[0] + b.data[1] - a.data[1];
          if (p.data[0] < x1) {
            b = f32x2(x1, b.data[1]);
            type = 0;
          } else {
            a = f32x2(x1, a.data[1]);
            type = 1;
          }
        }
        if (flip) {
          p = p.swizzle<1, 0>();
          a = a.swizzle<1, 0>();
          b = b.swizzle<1, 0>();
        }
      }
    }
    col = 0.25f + 0.75f * h2rgb(__ctx, float(h) / exp2(32.0f));
    d = min(min(p.data[0] - a.data[0], b.data[0] - p.data[0]),
            min(p.data[1] - a.data[1], b.data[1] - p.data[1]));
    if ((texelFetch(__ctx.channel3, i32x2((CHAR_X), 2)).data[0] != 0.0f))
      col *= 0.75f - 0.25f * cos(PI * d / px);
  }
  col *= smoothstep(0.0f, px, d - lwidth);
  col = pow(col, f32x3(0.4545f));
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
