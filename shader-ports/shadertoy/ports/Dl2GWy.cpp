// Port of https://www.shadertoy.com/view/Dl2GWy; author: wingei.
// Dataset license metadata: cc0-1.0; original retained in upstream/Dl2GWy.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float DigitBin(Context &__ctx, const int x) {
  return x == 0   ? 480599.0f
         : x == 1 ? 139810.0f
         : x == 2 ? 476951.0f
         : x == 3 ? 476999.0f
         : x == 4 ? 350020.0f
         : x == 5 ? 464711.0f
         : x == 6 ? 464727.0f
         : x == 7 ? 476228.0f
         : x == 8 ? 481111.0f
         : x == 9 ? 481095.0f
                  : 0.0f;
}
float PrintValue(Context &__ctx, f32x2 vStringCoords, float fValue,
                 float fMaxDigits, float fDecimalPlaces) {
  if ((vStringCoords.data[1] < 0.0f) || (vStringCoords.data[1] >= 1.0f))
    return 0.0f;
  bool bNeg = (fValue < 0.0f);
  fValue = abs(fValue);
  float fLog10Value = log2(abs(fValue)) / log2(10.0f);
  float fBiggestIndex = max(floor(fLog10Value), 0.0f);
  float fDigitIndex = fMaxDigits - floor(vStringCoords.data[0]);
  float fCharBin = 0.0f;
  if (fDigitIndex > (-fDecimalPlaces - 1.01f)) {
    if (fDigitIndex > fBiggestIndex) {
      if ((bNeg) && (fDigitIndex < (fBiggestIndex + 1.5f)))
        fCharBin = 1792.0f;
    } else {
      if (fDigitIndex == -1.0f) {
        if (fDecimalPlaces > 0.0f)
          fCharBin = 2.0f;
      } else {
        float fReducedRangeValue = fValue;
        if (fDigitIndex < 0.0f) {
          fReducedRangeValue = fract(fValue);
          fDigitIndex += 1.0f;
        }
        float fDigitValue =
            (abs(fReducedRangeValue / (pow(10.0f, fDigitIndex))));
        fCharBin = DigitBin(__ctx, int(floor(mod(fDigitValue, 10.0f))));
      }
    }
  }
  return floor(mod(
      (fCharBin / pow(2.0f, floor(fract(vStringCoords.data[0]) * 4.0f) +
                                (floor(vStringCoords.data[1] * 5.0f) * 4.0f))),
      2.0f));
}
float PrintValue(Context &__ctx, const f32x2 fragCoord,
                 const f32x2 vPixelCoords, const f32x2 vFontSize,
                 const float fValue, const float fMaxDigits,
                 const float fDecimalPlaces) {
  f32x2 vStringCharCoords =
      (fragCoord.swizzle<0, 1>() - vPixelCoords) / vFontSize;
  return PrintValue(__ctx, vStringCharCoords, fValue, fMaxDigits,
                    fDecimalPlaces);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 vColour = f32x3(0.0f);
  f32x2 vFontSize = f32x2(8.0f, 15.0f);
  f32x2 vPixelCoord1 = f32x2(5.0f, 5.0f);
  float customValueToPrint = 7.0f;
  float customDigit = PrintValue(__ctx, (fragCoord - vPixelCoord1) / vFontSize,
                                 customValueToPrint, 1.0f, 1.0f);
  vColour = mix(vColour, f32x3(0.0f, 1.0f, 1.0f), customDigit);
  fragColor = f32x4(vColour, 1.0f);
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
