// Port of https://www.shadertoy.com/view/DdfSDl; author: timrolls.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdfSDl.json.
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

float eb(Context &__ctx, float x) {
  x = 1.f - x;
  float y = x * x * (3.f * x - 2.f);
  return 1.f - y;
}
float rand(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
float getBound(Context &__ctx, f32x2 seed, float i) {
  float p = rand(__ctx, seed.swizzle<0, 1>() + 1.f);
  if (i < 1.f) {
    return rand(__ctx, seed) * 0.1f + 0.4f;
  }
  if (i > 1.0f) {
    if (p < 0.07f)
      return 0.f;
    if (p > 0.93f)
      return 1.f;
  }
  return rand(__ctx, seed) * 0.4f + 0.3f;
}
float getBound(Context &__ctx, f32x2 seed1, f32x2 seed2, float intv, float i) {
  float v1 = getBound(__ctx, seed1, i);
  float v2 = getBound(__ctx, seed2, i);
  return mix(v1, v2,
             eb(__ctx, clamp(intv * 2.0f - i * 0.3f - rand(__ctx, seed1) * 0.2f,
                             0.f, 1.f)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) /
             iResolution.data[1] * 2.1f;
  f32x2 ouv = uv;
  f32x3 col = f32x3(1);
  float ti = iTime * 2.0f;
  f32x2 seed1 = f32x2(floor(ti), 1);
  f32x2 seed2 = f32x2(floor(ti) + 1.f, 1);
  f32x2 le1 = f32x2(floor(rand(__ctx, seed1.swizzle<1, 0>()) * 16.f),
                    floor(rand(__ctx, seed1.swizzle<0, 0>()) * 16.f)) /
              16.f;
  f32x2 le2 = f32x2(floor(rand(__ctx, seed2.swizzle<1, 0>()) * 16.f),
                    floor(rand(__ctx, seed2.swizzle<0, 0>()) * 16.f)) /
              16.f;
  float intv = fract(ti);
  float ttv = intv;
  if (abs(uv.data[0]) > 2.0f || abs(uv.data[1]) > 1.0f) {
  } else {
    float dif = 1.0f;
    uv = uv * f32x2(0.25f, 0.5f) + 0.5f;
    f32x2 center1 = f32x2(0.5f, 0.5f);
    f32x2 size1 = f32x2(1, 1);
    f32x2 center2 = f32x2(0.5f, 0.5f);
    f32x2 size2 = f32x2(1, 1);
    for (int i = 0; i < 8; i++) {
      float boundX = getBound(__ctx, seed1, seed2, intv, float(i));
      float boundX1 = getBound(__ctx, seed1, float(i));
      float boundX2 = getBound(__ctx, seed2, float(i));
      center1.data[0] -= (0.5f - boundX1) * size1.data[0];
      center2.data[0] -= (0.5f - boundX2) * size2.data[0];
      if (uv.data[0] < boundX) {
        uv.data[0] /= boundX;
        seed1.data[0] += dif, seed2.data[0] += dif;
        size1.data[0] *= boundX1, size2.data[0] *= boundX2;
        center1.data[0] -= size1.data[0] * 0.5f,
            center2.data[0] -= size2.data[0] * 0.5f;
      } else {
        uv.data[0] = (uv.data[0] - boundX) / (1.f - boundX);
        seed1.data[0] -= dif, seed2.data[0] -= dif;
        size1.data[0] *= 1.f - boundX1, size2.data[0] *= 1.f - boundX2;
        center1.data[0] += size1.data[0] * 0.5f,
            center2.data[0] += size2.data[0] * 0.5f;
        intv += 0.15f * dif;
      }
      seed1 = seed1.swizzle<1, 0>();
      seed2 = seed2.swizzle<1, 0>();
      dif /= 2.0f;
      if (i > 0) {
        float boundY = getBound(__ctx, seed1, seed2, intv, float(i));
        float boundY1 = getBound(__ctx, seed1, float(i));
        float boundY2 = getBound(__ctx, seed2, float(i));
        center1.data[1] -= (0.5f - boundY1) * size1.data[1];
        center2.data[1] -= (0.5f - boundY2) * size2.data[1];
        if (uv.data[1] < boundY) {
          uv.data[1] /= boundY;
          seed1.data[1] += dif, seed2.data[1] += dif;
          size1.data[1] *= boundY1, size2.data[1] *= boundY2;
          center1.data[1] -= size1.data[1] * 0.5f,
              center2.data[1] -= size2.data[1] * 0.5f;
        } else {
          uv.data[1] = (uv.data[1] - boundY) / (1.f - boundY);
          seed1.data[1] -= dif, seed2.data[1] -= dif;
          size1.data[1] *= 1.f - boundY1, size2.data[1] *= 1.f - boundY2;
          center1.data[1] += size1.data[1] * 0.5f,
              center2.data[1] += size2.data[1] * 0.5f;
          intv += 0.15f * dif;
        }
      }
      seed1 = seed1.swizzle<1, 0>();
      seed2 = seed2.swizzle<1, 0>();
      dif /= 2.0f;
    }
    f32x2 coord0 = (center1 - f32x2(0.25f, 0)) / f32x2(8.0f, 16.0f);
    f32x2 coord1 = (center2 - f32x2(0.25f, 0)) / f32x2(8.0f, 16.0f);
    coord0 = clamp(coord0, 0.f, 1.f / 16.f);
    coord1 = clamp(coord1, 0.f, 1.f / 16.f);
    f32x4 c0 = texture(__ctx.channel0, coord0 + le1).swizzle<0, 1, 2, 3>();
    f32x4 c1 = texture(__ctx.channel0, coord1 + le2).swizzle<0, 1, 2, 3>();
    ttv =
        smoothstep(0.f, 1.f, -0.7f + ttv * 4.0f - rand(__ctx, center1) * 2.0f);
    float ld = rand(__ctx, seed1 + seed2) < 0.5f ? 1.f : 0.f;
    float ldu = rand(__ctx, seed1 - seed2) < 0.5f ? 1.f : 0.f;
    float lb = mix(1.f, -1.f, ldu);
    float mixParam = ld < 0.5f ? step(uv.data[0], lb * ttv + ldu)
                               : step(uv.data[1], lb * ttv + ldu);
    col *= 1.f - mix(c1.data[0], c0.data[0], mixParam);
  }
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
