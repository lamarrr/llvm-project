// Port of https://www.shadertoy.com/view/Dd2BR3; author: Elyades.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dd2BR3.json.
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
float PI = 3.1415926535897932f;
float PIONTWO = 1.57079632679f;
float add(Context &__ctx, float a, float b, float scalingFactor) {
  a *= scalingFactor;
  b *= scalingFactor;
  return ((a + b) / scalingFactor);
}
f32x2 add2(Context &__ctx, float a, float b) {
  float s = a + b;
  float e = s - a;
  float t = b - e;
  float high = s;
  float low = a - (s - t);
  return f32x2(high, low);
}
float lengthsquared(Context &__ctx, f32x2 uv) {
  return (uv.data[0] * uv.data[0] + uv.data[1] * uv.data[1]);
}
f32x2 itimes(Context &__ctx, f32x2 uv) {
  return (f32x2(-uv.data[1], uv.data[0]));
}
f32x2 oneoverz(Context &__ctx, f32x2 uv) {
  float squaremodulus = lengthsquared(__ctx, uv);
  return (f32x2(uv.data[0] / squaremodulus, -uv.data[1] / squaremodulus));
}
float mult(Context &__ctx, float a, float b, float scalingFactor) {
  a *= scalingFactor;
  b *= scalingFactor;
  return (a * b / (scalingFactor * scalingFactor));
}
f32x2 add2d(Context &__ctx, f32x2 a, f32x2 b, float scalingFactor) {
  a *= scalingFactor;
  b *= scalingFactor;
  return ((a + b) / scalingFactor);
}
f32x2 mult2d(Context &__ctx, f32x2 a, f32x2 b, float scalingFactor) {
  a *= scalingFactor;
  b *= scalingFactor;
  return (f32x2(add(__ctx, a.data[0] * b.data[0], -a.data[1] * b.data[1],
                    scalingFactor),
                add(__ctx, a.data[0] * b.data[1], a.data[1] * b.data[0],
                    scalingFactor)) /
          (scalingFactor * scalingFactor));
}
f32x2 exp2d(Context &__ctx, f32x2 uv) {
  return (exp(uv.data[0]) * f32x2(cos(uv.data[1]), sin(uv.data[1])));
}
f32x2 log2d(Context &__ctx, f32x2 uv) {
  return (f32x2(log(length(uv)), atan(uv.data[1], uv.data[0])));
}
f32x2 rotate(Context &__ctx, f32x2 uv, float angle) {
  float co = cos(angle);
  float si = sin(angle);
  uv *= f32x2x2(co, -si, si, co);
  return (uv);
}
f32x2 cos2d(Context &__ctx, f32x2 uv) {
  return (f32x2(exp2d(__ctx, itimes(__ctx, uv)) +
                exp2d(__ctx, itimes(__ctx, -uv))) *
          0.5f);
}
f32x2 sin2d(Context &__ctx, f32x2 uv) {
  f32x2 result =
      (exp2d(__ctx, itimes(__ctx, uv)) - exp2d(__ctx, itimes(__ctx, -uv))) *
      0.5f;
  return f32x2(-itimes(__ctx, result));
}
f32x2 power(Context &__ctx, f32x2 uv, float n) {
  float r = length(uv);
  float angle = (n * (atan(uv.data[1], uv.data[0])));
  return (pow(r, n) * f32x2(cos(angle), sin(angle)));
}
f32x2 iteration(Context &__ctx, f32x2 uv, f32x2 c) {
  f32x2 result = cos2d(__ctx, oneoverz(__ctx, sin2d(__ctx, uv))) + c;
  return (result);
}
f32x3 palette1(Context &__ctx, float t) {
  f32x3 a = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 b = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 c = f32x3(1.0f, 1.0f, 1.0f);
  f32x3 d = f32x3(0.33f, 0.67f, 1.0f);
  return f32x3(a + b * cos(6.28318530718f * (c * (t) + d)));
}
f32x3 palette2(Context &__ctx, float t) {
  f32x3 a = f32x3(0.5f, 0.3f, 0.5f);
  f32x3 b = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 c = f32x3(2.0f, 0.0f, 1.0f);
  f32x3 d = f32x3(0.25f, 0.25f, 0.25f);
  return f32x3(a + b * cos(6.28318530718f * (c * (t) + d)));
}
f32x3 thirdDegreeProcessing(Context &__ctx, f32x3 color) {
  float a = 8.92587f;
  float b = -13.75f;
  float c = 5.82143f;
  return f32x3(a * color * color * color - b * color * color + c * color);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float scale = 5.0f;
  uv += -0.5f;
  uv *= scale;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  float zoomAmount = pow(10.0f, -iTime / 5.0f);
  f32x2 mousePoint =
      (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() / 2.0f) * 8.0f /
      iResolution.swizzle<0, 1>();
  f32x2 zoomPoint = f32x2(-0.1f, 0.0f);
  uv = uv * zoomAmount;
  uv += zoomPoint;
  f32x2 c =
      0.5f * (iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f);
  float tol = pow(10.0f, -8.0f);
  float DistanceToPrevious = 1.0f;
  float currentModulus = lengthsquared(__ctx, uv);
  f32x2 previousIteration;
  int iterationAmount;
  int testAmount = 80;
  f32x2 dummyuv = uv;
  for (iterationAmount = 0; iterationAmount < testAmount; iterationAmount++) {
    previousIteration = dummyuv;
    dummyuv = iteration(__ctx, dummyuv, c);
    currentModulus = lengthsquared(__ctx, dummyuv);
    DistanceToPrevious = lengthsquared(__ctx, dummyuv - previousIteration);
    if (DistanceToPrevious < tol) {
      break;
    }
    if (currentModulus > pow(10.0f, 10.0f)) {
      break;
    }
  }
  if (currentModulus < pow(10.0f, 10.0f)) {
    fragColor = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
  } else {
    {
      int periodFast = 5;
      int residueFast = iterationAmount % periodFast;
      float indexFast = float(residueFast) / float(periodFast);
      f32x3 colorFast = palette2(__ctx, indexFast);
      int periodMedium = 15;
      int residueMedium = iterationAmount % periodMedium;
      float indexMedium = float(residueMedium) / float(periodMedium);
      f32x3 colorMedium = palette2(__ctx, indexMedium);
      int periodSlow = 50;
      int residueSlow = iterationAmount % periodSlow;
      float indexSlow = float(residueSlow) / float(periodSlow);
      f32x3 colorSlow = palette2(__ctx, indexSlow);
      float finalIndex = (indexSlow + indexMedium + indexFast) * 0.333333333f;
      f32x3 finalColor = (colorSlow + colorMedium + colorFast) * 0.3333333333f;
      fragColor = f32x4(finalColor, 1.0f);
    }
  }
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
