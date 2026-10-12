// Port of https://www.shadertoy.com/view/ddBczW; author: Elyades.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddBczW.json.
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

float PI = 3.14159265358979323846f;
float TWOPI = 6.28318530718f;
float PIONTWO = 1.57079632679f;
float norm(Context &__ctx, f32x2 uv) {
  return (float(uv.data[0] * uv.data[0] + uv.data[1] * uv.data[1]));
}
float atan2(Context &__ctx, float y, float x) {
  float angle = atan(y, x);
  if (angle < 0.0f) {
    angle += TWOPI;
  }
  return (angle);
}
f32x2 exp2d(Context &__ctx, f32x2 uv) {
  return (exp(uv.data[0]) * f32x2(cos(uv.data[1]), sin(uv.data[1])));
}
f32x2 log2d(Context &__ctx, f32x2 uv) {
  return (f32x2(log(length(uv)), atan2(__ctx, uv.data[1], uv.data[0])));
}
f32x2 power(Context &__ctx, f32x2 uv, float n) {
  float modulus = length(uv);
  float angle = atan2(__ctx, uv.data[1], uv.data[0]);
  angle *= n;
  return (exp(n * log(modulus)) * f32x2(cos(angle), sin(angle)));
}
f32x2 times(Context &__ctx, f32x2 z1, f32x2 z2) {
  return f32x2(z1.data[0] * z2.data[0] - z1.data[1] * z2.data[1],
               z1.data[0] * z2.data[1] + z1.data[1] * z2.data[0]);
}
f32x2 oneoverz(Context &__ctx, f32x2 uv) {
  float squaremodulus = norm(__ctx, uv);
  return (f32x2(uv.data[0] / squaremodulus, -uv.data[1] / squaremodulus));
}
f32x2 power2d(Context &__ctx, f32x2 uv, f32x2 z) {
  return (exp2d(
      __ctx,
      times(__ctx, f32x2(log(length(uv)), atan2(__ctx, uv.data[1], uv.data[0])),
            z)));
}
f32x2 cos2d(Context &__ctx, f32x2 uv) {
  return (f32x2(0.5f * (exp2d(__ctx, times(__ctx, uv, f32x2(0.0f, 1.0f))) +
                        exp2d(__ctx, times(__ctx, -uv, f32x2(0.0f, 1.0f))))));
}
f32x2 sin2d(Context &__ctx, f32x2 uv) {
  return (times(__ctx,
                0.5f * (exp2d(__ctx, times(__ctx, uv, f32x2(0.0f, 1.0f))) -
                        exp2d(__ctx, times(__ctx, -uv, f32x2(0.0f, 1.0f)))),
                f32x2(0.0f, 1.0f)));
}
f32x2 function(Context &__ctx, f32x2 uv, f32x2 c) {
  f32x2 result = tan(atan2(__ctx, uv.data[1], uv.data[0])) + c;
  return result;
}
f32x3 palette2(Context &__ctx, float t) {
  f32x3 a = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 b = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 c = f32x3(1.0f, 1.0f, 1.0f);
  f32x3 d = f32x3(0.0f, 0.1f, 0.2f);
  return f32x3(a + b * cos(6.28318530718f * (c * (t) + d)));
}
f32x3 palette5(Context &__ctx, float iteration) {
  float value = 0.8f;
  f32x3 a = f32x3(value);
  f32x3 b = f32x3(1.0f - value);
  f32x3 c = f32x3(2.0f, 1.0f, 1.0f);
  f32x3 d = f32x3(0.3f, 0.0f, 0.0f);
  return (a + b * cos(TWOPI * (c * iteration + d)));
}
f32x2 functionPolar(Context &__ctx, f32x2 uv) {
  float n = 3.0f;
  f32x2 c = f32x2(0.1f, -0.8f);
  f32x2 result;
  result.data[0] = pow(uv.data[0], n);
  result.data[1] = n * uv.data[1];
  return result;
}
f32x2 cartesianToPolar(Context &__ctx, f32x2 uv) {
  float r = length(uv);
  float theta = atan(uv.data[1], uv.data[0]);
  return f32x2(r, theta);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv = uv - 0.5f;
  uv.data[0] += 0.0f;
  uv *= 10.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 zoomPoint = f32x2(-0.0f, 0.0f);
  float zoomAmount = min(1.0f, 1.0f / pow(max(iTime - 3.0f, 1.0f), 3.0f));
  uv = uv * zoomAmount;
  uv += zoomPoint;
  float tol = pow(10.0f, -6.0f);
  float DistanceToPrevious = 1.0f;
  float currentModulus = length(uv);
  f32x2 previousIteration;
  int iterationAmount;
  int testAmount = 1000;
  f32x2 c = uv;
  for (iterationAmount = 0; iterationAmount < testAmount; iterationAmount++) {
    previousIteration = uv;
    uv = function(__ctx, uv, c);
    currentModulus = length(uv);
    DistanceToPrevious = length(uv - previousIteration);
    if (DistanceToPrevious < tol) {
      break;
    }
    if (currentModulus > 3.0f) {
      break;
    }
  }
  if (currentModulus < 3.0f) {
    fragColor = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
  } else {
    iterationAmount *= 5;
    int periodFast = 5;
    int residueFast = iterationAmount % periodFast;
    float indexFast = float(residueFast) / float(periodFast);
    f32x3 colorFast = palette5(__ctx, indexFast);
    int periodMedium = 20;
    int residueMedium = iterationAmount % periodMedium;
    float indexMedium = float(residueMedium) / float(periodMedium);
    f32x3 colorMedium = palette5(__ctx, indexMedium);
    int periodSlow = 50;
    int residueSlow = iterationAmount % periodSlow;
    float indexSlow = float(residueSlow) / float(periodSlow);
    f32x3 colorSlow = palette5(__ctx, indexSlow);
    float finalIndex = (indexSlow + indexMedium + indexFast) * 0.333333333f;
    f32x3 finalColor = (colorSlow + colorMedium + colorFast) * 0.3333333333f;
    fragColor = f32x4(finalColor, 1.0f);
  }
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
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
