// Port of https://www.shadertoy.com/view/DdVfR1; author: gehtsiegarnixan.
// Dataset license metadata: mit; original retained in upstream/DdVfR1.json.
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
f32x2 ScreenSpaceDither22(Context &__ctx, f32x2 vScreenPos, float time) {
  float vDither = dot(f32x2(171.0f, 231.0f), vScreenPos.swizzle<0, 1>() + time);
  return fract(vDither / f32x2(103.0f, 71.0f));
}
f32x3 ditherblur(Context &__ctx, f32x2 uv, f32x2 fragCoord, float time,
                 float blurStrength) {
  f32x2 dither = ScreenSpaceDither22(__ctx, fragCoord, time) - 0.5f;
  f32x2 noisyUV = fract(uv - dither * blurStrength);
  f32x2 texSize = f32x2(textureSize(__ctx.channel0, 0));
  i32x2 texelCoord = i32x2(noisyUV * texSize);
  f32x3 color = texelFetch(__ctx.channel0, texelCoord).swizzle<0, 1, 2>();
  return color;
}
f32x3 mipBlur(Context &__ctx, f32x2 uv, float blurStrength) {
  f32x2 texSize = f32x2(textureSize(__ctx.channel0, 0));
  float maxMipLevel = log2(max(texSize.data[0], texSize.data[1]));
  float blurPixelRange = max(texSize.data[0], texSize.data[1]) * blurStrength;
  float mipLevel = log2(blurPixelRange);
  f32x3 color = textureLod(__ctx.channel0, uv, mipLevel).swizzle<0, 1, 2>();
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float border;
  float blurStrength;
  if (iMouse.data[0] > 0.0f) {
    f32x2 mouse = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
    border = mouse.data[0];
    blurStrength = mix(0.0f, 0.1f, pow(mouse.data[1], 2.f));
  } else {
    border = sin(iTime * 0.5f) * 0.5f + 0.5f;
    blurStrength = mix(0.0f, 0.1f, pow(cos(iTime * 0.3f) * 0.5f + 0.5f, 2.f));
  }
  blurStrength *= iResolution.data[0] / iResolution.data[1];
  f32x3 color;
  if (uv.data[0] > border) {
    for (int i = 0; i < 8; i++) {
      color += ditherblur(__ctx, uv, fragCoord, iTime - float(i), blurStrength);
    }
    color /= float(8);
  } else {
    color += mipBlur(__ctx, uv, blurStrength).swizzle<0, 1, 2>();
  }
  color -= step(abs(uv.data[0] - border), 0.002f);
  fragColor = f32x4(color, 1.0f);
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
