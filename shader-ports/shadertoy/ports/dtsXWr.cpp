// Port of https://www.shadertoy.com/view/dtsXWr; author: oneshade.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtsXWr.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const f32x3[] palette =
    f32x3[](f32x3(0x000000 >> 16, 0x000000 >> 8 & 255, 0x000000 & 255) / 255.f,
            f32x3(0x626262 >> 16, 0x626262 >> 8 & 255, 0x626262 & 255) / 255.f,
            f32x3(0x898989 >> 16, 0x898989 >> 8 & 255, 0x898989 & 255) / 255.f,
            f32x3(0xADADAD >> 16, 0xADADAD >> 8 & 255, 0xADADAD & 255) / 255.f,
            f32x3(0xFFFFFF >> 16, 0xFFFFFF >> 8 & 255, 0xFFFFFF & 255) / 255.f,
            f32x3(0x9F4E44 >> 16, 0x9F4E44 >> 8 & 255, 0x9F4E44 & 255) / 255.f,
            f32x3(0xCB7E75 >> 16, 0xCB7E75 >> 8 & 255, 0xCB7E75 & 255) / 255.f,
            f32x3(0x6D5412 >> 16, 0x6D5412 >> 8 & 255, 0x6D5412 & 255) / 255.f,
            f32x3(0xA1683C >> 16, 0xA1683C >> 8 & 255, 0xA1683C & 255) / 255.f,
            f32x3(0xC9D487 >> 16, 0xC9D487 >> 8 & 255, 0xC9D487 & 255) / 255.f,
            f32x3(0x9AE29B >> 16, 0x9AE29B >> 8 & 255, 0x9AE29B & 255) / 255.f,
            f32x3(0x5CAB5E >> 16, 0x5CAB5E >> 8 & 255, 0x5CAB5E & 255) / 255.f,
            f32x3(0x6ABFC6 >> 16, 0x6ABFC6 >> 8 & 255, 0x6ABFC6 & 255) / 255.f,
            f32x3(0x887ECB >> 16, 0x887ECB >> 8 & 255, 0x887ECB & 255) / 255.f,
            f32x3(0x50459B >> 16, 0x50459B >> 8 & 255, 0x50459B & 255) / 255.f,
            f32x3(0xA057A3 >> 16, 0xA057A3 >> 8 & 255, 0xA057A3 & 255) / 255.f);
float Hash21(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(p.swizzle<0, 1, 0>() * 0.1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 imgColor =
      texture(__ctx.channel0, fragCoord / iResolution.swizzle<0, 1>())
          .swizzle<0, 1, 2>();
  if (1 == 1) {
    float[palette.length()] M;
    float total = 0.0f;
    for (int i = 0; i < palette.length(); i++) {
      f32x3 palColor = palette[i];
      float m = 1.0f - max(abs(palColor.data[0] - imgColor.data[0]),
                           max(abs(palColor.data[1] - imgColor.data[1]),
                               abs(palColor.data[2] - imgColor.data[2])));
      M[i] = m;
      total += m;
    }
    float totalM = total;
    total = 0.0f;
    for (int i = 0; i < palette.length(); i++) {
      float pickChance = M[i] / totalM;
      pickChance = pow(pickChance, 20.0f);
      M[i] = pickChance;
      total += pickChance;
    }
    float rand = Hash21(__ctx, fragCoord);
    float low = 0.0f;
    for (int i = 0; i < palette.length(); i++) {
      f32x3 palColor = palette[i];
      float pickChance = M[i] / total;
      float high = low + pickChance;
      if (rand >= low && rand <= high) {
        fragColor = f32x4(palColor, 1.0f);
        break;
      }
      low = high;
    }
  } else {
    float bestM = 0.0f;
    for (int i = 0; i < palette.length(); i++) {
      f32x3 palColor = palette[i];
      float m = 1.0f - max(abs(palColor.data[0] - imgColor.data[0]),
                           max(abs(palColor.data[1] - imgColor.data[1]),
                               abs(palColor.data[2] - imgColor.data[2])));
      if (m > bestM) {
        fragColor = f32x4(palColor, 1.0f);
        bestM = m;
      }
    }
  }
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
