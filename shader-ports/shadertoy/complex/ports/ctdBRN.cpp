// Port of https://www.shadertoy.com/view/ctdBRN; author: DrieStone.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctdBRN.json.
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
float edgeThreshold = 0.4f;
float brightness = 0.2f;
float contrast = 2.5f;
f32x3 rgb2hsv(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
  f32x4 p = mix(f32x4(c.swizzle<2, 1>(), K.swizzle<3, 2>()),
                f32x4(c.swizzle<1, 2>(), K.swizzle<0, 1>()),
                step(c.data[2], c.data[1]));
  f32x4 q =
      mix(f32x4(p.swizzle<0, 1, 3>(), c.data[0]),
          f32x4(c.data[0], p.swizzle<1, 2, 0>()), step(p.data[0], c.data[0]));
  float d = q.data[0] - min(q.data[3], q.data[1]);
  float e = 1.0e-10f;
  return f32x3(abs(q.data[2] + (q.data[3] - q.data[1]) / (6.0f * d + e)),
               d / (q.data[0] + e), q.data[0]);
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
f32x3 boostSaturation(Context &__ctx, f32x3 color, float amount) {
  f32x3 hsv = rgb2hsv(__ctx, color);
  hsv.data[1] *= amount;
  return hsv2rgb(__ctx, hsv);
}
f32x3 selectiveGaussianBlur(Context &__ctx, f32x2 uv, f32x3 color,
                            float saturationThreshold, float blurAmount) {
  f32x3 blurredColor = f32x3(0.0f);
  f32x2 blurSize = f32x2(blurAmount) / iResolution.swizzle<0, 1>();
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      f32x2 offset = f32x2(float(x), float(y)) * blurSize;
      f32x3 neighborColor =
          texture(__ctx.channel0, uv + offset).swizzle<0, 1, 2>();
      if (rgb2hsv(__ctx, neighborColor).data[1] < saturationThreshold) {
        blurredColor += neighborColor;
      } else {
        blurredColor += color;
      }
    }
  }
  return blurredColor / 9.0f;
}
f32x3 adjustBrightness(Context &__ctx, f32x3 color) {
  return color + brightness;
}
f32x3 adjustContrast(Context &__ctx, f32x3 color) {
  return (color - 0.5f) * contrast + 0.5f;
}
float edgeDetection(Context &__ctx, f32x2 uv) {
  f32x3 edgeColor = f32x3(0.0f);
  f32x2 texelSize = 1.0f / iResolution.swizzle<0, 1>();
  float scaleFactor = 1.5f;
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      f32x2 offset = scaleFactor * f32x2(x, y) * texelSize;
      f32x3 neighborColor =
          texture(__ctx.channel0, uv + offset).swizzle<0, 1, 2>();
      edgeColor +=
          abs(neighborColor - texture(__ctx.channel0, uv).swizzle<0, 1, 2>());
    }
  }
  float edgeStrength = length(edgeColor);
  return edgeStrength > edgeThreshold ? 1.0f : 0.0f;
}
f32x3 quantizeColor(Context &__ctx, f32x3 color) {
  int numColors = 3;
  f32x3 quantized = floor(color * float(numColors)) / float(numColors);
  return quantized;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x4 webcamColor = texture(__ctx.channel0, uv);
  f32x3 adjustedColor = adjustBrightness(__ctx, webcamColor.swizzle<0, 1, 2>());
  adjustedColor = adjustContrast(__ctx, adjustedColor);
  adjustedColor = boostSaturation(__ctx, adjustedColor, 1.1f);
  float saturationThreshold = 0.2f;
  float blurAmount = 4.0f;
  f32x3 colorAfterBlur = selectiveGaussianBlur(__ctx, uv, adjustedColor,
                                               saturationThreshold, blurAmount);
  float edgeStrength = edgeDetection(__ctx, uv);
  f32x3 quantizedColor = quantizeColor(__ctx, colorAfterBlur);
  f32x3 finalColor = mix(quantizedColor, f32x3(0.0f), 0.5f * edgeStrength);
  fragColor = f32x4(finalColor, 1.0f);
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
