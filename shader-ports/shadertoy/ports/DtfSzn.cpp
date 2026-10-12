// Port of https://www.shadertoy.com/view/DtfSzn; author: Meleshin.
// Dataset license metadata: mit; original retained in upstream/DtfSzn.json.
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
struct varsStruct {
  float fadeIn;
  float fadeOut;
  f32x3 colorA;
  f32x3 colorB;
  f32x3 rotVector;
  float rotAngle;
  float scale;
  f32x3 offset;
  float time;
  float fade;
  float clipTime;
};
varsStruct varsSeq[] = varsStruct[](
    varsStruct(4.f, 0.f, f32x3(1, 1, 1), f32x3(0, 0, 0), f32x3(0, 1, 1), 0.f,
               3.f, f32x3(1, 1, 1), 0.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1), 0.f,
               3.f, f32x3(1, 1, 1), 4.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               40.f, 3.f, f32x3(1, 1, 1), 8.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               40.f, 3.f, f32x3(1, 1, 1), 20.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               80.f, 3.f, f32x3(1, 1, 1), 24.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               80.f, 3.f, f32x3(1, 1, 1), 32.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 1), f32x3(0, 1, 1), 80.f,
               5.f, f32x3(1, 1, 1), 36.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 1), f32x3(0, 1, 1), 120.f,
               3.f, f32x3(1, 1, 1), 40.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 1), f32x3(0, 1, 1), 120.f,
               3.f, f32x3(1, 1, 1), 44.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 1), f32x3(0, 1, 1), 160.f,
               3.f, f32x3(1, 1, 1), 48.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 0), f32x3(0, 1, 1), 160.f,
               5.f, f32x3(1, 1, 1), 52.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 0), f32x3(0, 1, 1), 200.f,
               3.f, f32x3(1, 1, 1), 56.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 0), f32x3(0, 1, 1), 200.f,
               3.f, f32x3(1, 1, 1), 68.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 0), f32x3(0, 1, 1), 240.f,
               3.f, f32x3(1, 1, 1), 72.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 0), f32x3(0, 1, 1), 240.f,
               3.f, f32x3(1, 1, 1), 80.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               240.f, 5.f, f32x3(1, 1, 1), 84.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               280.f, 3.f, f32x3(1.2f, 1, 1), 88.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               280.f, 3.f, f32x3(1.2f, 1, 1), 92.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               320.f, 3.f, f32x3(1.2f, 1, 1), 96.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               320.f, 3.f, f32x3(1.2f, 1, 1), 100.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               360.f, 3.f, f32x3(1.2f, 1, 1), 104.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               360.f, 3.f, f32x3(1.2f, 1, 1), 108.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               400.f, 3.f, f32x3(1.2f, 1, 1), 112.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0.5f, 0), f32x3(0, 1, 1),
               440.f, 3.f, f32x3(1.2f, 1, 1), 116.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.9f, 0.1f, 0.8f),
               f32x3(0, 1, 1), 440.f, 5.f, f32x3(1, 1, 1), 120.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.9f, 0.1f, 0.8f),
               f32x3(0, 1, 1), 480.f, 3.f, f32x3(1, 1, 1), 124.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.9f, 0.1f, 0.8f),
               f32x3(0, 1, 1), 520.f, 3.f, f32x3(1, 1, 1), 128.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.9f, 0.1f, 0.8f),
               f32x3(0, 1, 1), 560.f, 3.f, f32x3(1, 1, 1), 132.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.9f, 0.1f, 0.8f),
               f32x3(0, 1, 1), 600.f, 3.f, f32x3(1, 1, 1), 136.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.9f, 0.1f, 0.8f),
               f32x3(0, 1, 1), 640.f, 3.f, f32x3(1, 1, 1), 140.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(7, 0.7f, 0.1f), f32x3(0, 1, 1),
               680.f, 5.f, f32x3(1, 1, 1), 144.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(7, 0.7f, 0.1f), f32x3(0, 1, 1),
               680.f, 3.f, f32x3(1, 1, 0.5f), 148.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(7, 0.7f, 0.1f), f32x3(0, 1, 1),
               680.f, 3.f, f32x3(1, 1, 0.5f), 152.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(7, 0.7f, 0.1f), f32x3(0, 1, 1),
               720.f, 3.f, f32x3(1, 1, 0.5f), 156.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(7, 0.7f, 0.1f), f32x3(0, 1, 1),
               720.f, 3.f, f32x3(1, 1, 0.5f), 164.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(7, 0.7f, 0.1f), f32x3(0, 1, 1),
               760.f, 3.f, f32x3(1, 1, 0.5f), 168.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(7, 0.7f, 0.1f), f32x3(0, 1, 1),
               760.f, 3.f, f32x3(1, 1, 0.5f), 176.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(7, 0.7f, 0.1f), f32x3(0, 1, 1),
               800.f, 3.f, f32x3(1, 1, 0.5f), 180.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0, 0.7f, 0.2f), f32x3(0, 1, 1),
               800.f, 5.f, f32x3(1, 1, 1), 184.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0, 0.7f, 0.2f), f32x3(0, 1, 1),
               840.f, 3.f, f32x3(1, 1, 1), 188.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0, 0.7f, 0.2f), f32x3(0, 1, 1),
               840.f, 3.f, f32x3(1, 1, 1), 200.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0, 0.7f, 0.2f), f32x3(0, 1, 1),
               880.f, 3.f, f32x3(1, 1, 1), 204.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0, 0.7f, 0.2f), f32x3(0, 1, 1),
               880.f, 3.f, f32x3(1, 1, 1), 212.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.5f, 0.1f, 1), f32x3(0, 1, 1),
               880.f, 5.f, f32x3(1, 1, 1), 216.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.5f, 0.1f, 1), f32x3(0, 1, 1),
               920.f, 3.f, f32x3(1, 1, 1), 220.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.5f, 0.1f, 1), f32x3(0, 1, 1),
               920.f, 3.f, f32x3(1, 1, 1), 232.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.5f, 0.1f, 1), f32x3(0, 1, 1),
               960.f, 3.f, f32x3(1, 1, 1), 236.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.5f, 0.1f, 1), f32x3(0, 1, 1),
               960.f, 3.f, f32x3(1, 1, 1), 240.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(3, 3, 0), f32x3(0, 1, 1), 960.f,
               5.f, f32x3(1, 1, 1), 244.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(3, 3, 0), f32x3(0, 1, 1), 1000.f,
               3.f, f32x3(1, 1, 1), 248.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(3, 3, 0), f32x3(0, 1, 1), 1000.f,
               3.f, f32x3(1, 1, 1), 260.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(3, 3, 0), f32x3(0, 1, 1), 1040.f,
               3.f, f32x3(1, 1, 1), 264.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(3, 3, 0), f32x3(0, 1, 1), 1040.f,
               3.f, f32x3(1, 1, 1), 276.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(3, 3, 0), f32x3(0, 1, 1), 1080.f,
               3.7f, f32x3(1, 0.8f, 1.9f), 280.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 7, 1), f32x3(0, 1, 1), 1080.f,
               5.f, f32x3(1, 1, 1), 284.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 7, 1), f32x3(0, 1, 1), 1120.f,
               3.f, f32x3(1, 1, 1), 288.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 7, 1), f32x3(0, 1, 1), 1120.f,
               3.f, f32x3(1, 1, 1), 292.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 7, 1), f32x3(0, 1, 1), 1160.f,
               3.f, f32x3(1, 1, 1), 296.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 7, 1), f32x3(0, 1, 1), 1160.f,
               3.f, f32x3(1, 1, 1), 300.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 1), f32x3(0, 1, 1), 1160.f,
               5.f, f32x3(1, 1, 1), 304.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 1), f32x3(0, 1, 1), 1200.f,
               3.f, f32x3(1, 1, 1), 308.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(1, 0, 1), f32x3(0, 1, 1), 1200.f,
               3.f, f32x3(1, 1, 1), 320.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               1240.f, 3.f, f32x3(1, 1, 1), 324.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               1240.f, 3.f, f32x3(1, 1, 1), 336.f, 1.f, 0.f),
    varsStruct(0.f, 0.f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               1280.f, 3.5f, f32x3(1, 1, 1), 340.f, 1.f, 0.f),
    varsStruct(0.f, 35.75f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               1280.f, 3.5f, f32x3(1, 1, 1), 340.f, 1.f, 0.f),
    varsStruct(0.f, 35.75f, f32x3(1, 1, 1), f32x3(0.7f, 1, 1), f32x3(0, 1, 1),
               1280.f, 3.5f, f32x3(1, 1, 1), 372.f, 1.f, 0.f));
varsStruct vars;
void varsInit(Context &__ctx) {
  const float beatTime = 60.f / 130.f;
  float loopLength = varsSeq[varsSeq.length() - 1].time * beatTime;
  float loopT = mod(iTime, loopLength);
  float clipT;
  float clipL;
  varsStruct a;
  varsStruct b;
  float t;
  for (int i = 0; i < varsSeq.length() - 1; i++) {
    float sTimeNext = varsSeq[i + 1].time * beatTime;
    if (sTimeNext > loopT) {
      float sTime = varsSeq[i].time * beatTime;
      a = varsSeq[i];
      b = varsSeq[i + 1];
      clipT = loopT - sTime;
      clipL = sTimeNext - sTime;
      t = clipT / clipL;
      break;
    }
  }
  vars.colorA = mix(a.colorA, b.colorA, t);
  vars.colorB = mix(a.colorB, b.colorB, t);
  vars.rotVector = mix(a.rotVector, b.rotVector, t);
  vars.rotAngle = mix(a.rotAngle, b.rotAngle, t);
  vars.scale = mix(a.scale, b.scale, t);
  vars.offset = mix(a.offset, b.offset, t);
  vars.time = loopT;
  vars.clipTime = t;
  vars.fade = 1.f;
  if (a.fadeIn != 0.f) {
    vars.fade *= clamp(clipT / (a.fadeIn * beatTime), 0.f, 1.f);
  }
  if (a.fadeOut != 0.f) {
    vars.fade *= clamp((clipL - clipT) / (a.fadeOut * beatTime), 0.f, 1.f);
  }
  vars.clipTime = mix(a.clipTime, b.clipTime, t);
}
struct varsPStruct {
  f32x3 pathStopDir;
  f32x3 path;
  float time;
  f32x3 pathDir;
  float pathMinDir;
  float clipTime;
};
varsPStruct varsPSeq[] =
    varsPStruct[](varsPStruct(f32x3(0, 0, 1), f32x3(0, 0, 0), 0.f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(0, 0, 10), 39.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(0, 12, 10), 87.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(10, 12, 10), 123.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(10, 18, 10), 147.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(10, 18, 20), 187.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(10, 26, 20), 219.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(18, 26, 20), 247.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(18, 36, 20), 287.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(18, 36, 26), 307.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(18, 36, 32), 331.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(18, 34, 32), 339.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(18, 34, 40), 367.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(18, 36, 40), 375.75f,
                              f32x3(0, 0, 1), 0.01f, 0.f),
                  varsPStruct(f32x3(0, 0, 1), f32x3(18, 36, 46), 396.f,
                              f32x3(0, 0, 1), 0.01f, 0.f));
varsPStruct varsP;
void varsPInit(Context &__ctx) {
  const float beatTime = 60.f / 130.f;
  float loopLength = varsPSeq[varsPSeq.length() - 1].time * beatTime;
  float loopT = mod(iTime, loopLength);
  float clipT;
  float clipL;
  varsPStruct a;
  varsPStruct b;
  float t;
  for (int i = 0; i < varsPSeq.length() - 1; i++) {
    float sTimeNext = varsPSeq[i + 1].time * beatTime;
    if (sTimeNext > loopT) {
      float sTime = varsPSeq[i].time * beatTime;
      a = varsPSeq[i];
      b = varsPSeq[i + 1];
      clipT = loopT - sTime;
      clipL = sTimeNext - sTime;
      t = clipT / clipL;
      break;
    }
  }
  float loopTLA = mod(iTime + 3.f, loopLength);
  varsPStruct aLA;
  varsPStruct bLA;
  float tLA;
  for (int i = 0; i < varsPSeq.length() - 1; i++) {
    float sTimeNext = varsPSeq[i + 1].time * beatTime;
    if (sTimeNext > loopTLA) {
      float sTime = varsPSeq[i].time * beatTime;
      aLA = varsPSeq[i];
      bLA = varsPSeq[i + 1];
      float clipT = loopTLA - sTime;
      float clipL = sTimeNext - sTime;
      tLA = clipT / clipL;
      break;
    }
  }
  varsP.path = mix(a.path, b.path, t);
  f32x3 pLA = mix(aLA.path, bLA.path, tLA);
  varsP.pathStopDir = mix(aLA.pathStopDir, bLA.pathStopDir, tLA);
  varsP.pathMinDir = mix(aLA.pathMinDir, bLA.pathMinDir, tLA);
  f32x3 pDir = pLA - varsP.path;
  float pStop = 1.f - clamp(length(pDir) / varsP.pathMinDir, 0.f, 1.f);
  pDir += varsP.pathStopDir * pStop;
  varsP.pathDir = normalize(pDir);
  varsP.time = loopT;
  varsP.clipTime = t;
  varsP.pathMinDir = mix(a.pathMinDir, b.pathMinDir, t);
  varsP.clipTime = mix(a.clipTime, b.clipTime, t);
}
f32x3x3 rot;
const float inf = 1e10f;
f32x3 ro0;
f32x3 ro;
float barPulse;
f32x3x3 rotMat(Context &__ctx, f32x3 v, float c, float s) {
  float k = (1.f - c);
  return f32x3x3(
      k * v.data[0] * v.data[0] + c, k * v.data[0] * v.data[1] - s * v.data[2],
      k * v.data[0] * v.data[2] + s * v.data[1],
      k * v.data[0] * v.data[1] + s * v.data[2], k * v.data[1] * v.data[1] + c,
      k * v.data[1] * v.data[2] - s * v.data[0],
      k * v.data[0] * v.data[2] - s * v.data[1],
      k * v.data[1] * v.data[2] + s * v.data[0], k * v.data[2] * v.data[2] + c);
}
f32x3x3 rotMat(Context &__ctx, f32x3 v, float angle) {
  return rotMat(__ctx, v, cos(angle), sin(angle));
}
f32x3x3 rotMat(Context &__ctx, f32x3 z, f32x3 d) {
  f32x3 cr = cross(z, d);
  return rotMat(__ctx, cr, dot(z, d), length(cr));
}
f32x3 opRepeat(Context &__ctx, f32x3 p, float repW) {
  float repWD = repW / 2.f;
  return mod(p + repWD, repW) - repWD;
}
float deMengerBased(Context &__ctx, f32x3 p, int iter, float scale,
                    f32x3 offset) {
  int n = 0;
  while (n < iter) {
    p = abs(p);
    if (p.data[0] < p.data[1]) {
      p.set_swizzle<0, 1>(p.swizzle<1, 0>());
    }
    p.data[0] += .24f;
    if (p.data[0] < p.data[2]) {
      p.set_swizzle<0, 2>(p.swizzle<2, 0>());
    }
    p.data[1] += .05f;
    if (p.data[1] < p.data[2]) {
      p.set_swizzle<1, 2>(p.swizzle<2, 1>());
    }
    p.data[2] -= .07f;
    p = scale * p - offset * (scale - 1.0f);
    if (p.data[2] < -0.5f * offset.data[2] * (scale - 1.0f))
      p.data[2] += offset.data[2] * (scale - 1.0f);
    p = rot * p;
    n++;
  }
  return length(p) * pow(scale, float(-n));
}
float map(Context &__ctx, f32x3 p) {
  float res = inf;
  p -= ro0;
  p = opRepeat(__ctx, p, 2.f);
  res = min(res, deMengerBased(__ctx, p, 8, vars.scale, vars.offset));
  return res;
}
f32x2 march(Context &__ctx, f32x3 ro, f32x3 rd, int maxSteps, float endDist,
            float maxDist) {
  float res = 0.f;
  int i = 0;
  for (; i < maxSteps; i++) {
    f32x3 p = ro + rd * res;
    float dist = map(__ctx, p);
    res += dist;
    if (res > maxDist || dist < endDist) {
      return f32x2(res, i);
    }
  }
  return f32x2(res, i);
}
f32x3 colorFast(Context &__ctx, float dist, float it) {
  f32x3 itL = clamp(1.f - it / 40.f, 0.f, 1.f) * vars.colorA;
  f32x3 distL = clamp((1.f + barPulse * 2.3f) * dist * dist / 400.f, 0.f, 1.f) *
                vars.colorB;
  float m = clamp(dist / 2.f, 0.f, 1.f);
  return mix(itL, distL, m);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  varsInit(__ctx);
  varsPInit(__ctx);
  const float beatTime = 60.f / 130.f;
  const float barTime = 4.f * beatTime;
  float timeInBars = vars.time / barTime;
  barPulse = pow(1.f - abs(timeInBars - round(timeInBars)) * 2.f, 3.f);
  const float foc = 1.f;
  ro0 = f32x3(0, 1, 0);
  ro = ro0 + varsP.path;
  f32x3 col = f32x3(0.f);
  f32x3 lightPos = ro;
  rot = rotMat(__ctx, normalize(vars.rotVector), radians(vars.rotAngle));
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.data[1];
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1], foc)) *
             rotMat(__ctx, f32x3(0, 0, 1), varsP.pathDir);
  f32x2 dist = march(__ctx, ro, rd, 100, .001f, 100.f);
  col += colorFast(__ctx, dist.data[0], dist.data[1]);
  fragColor = f32x4(col * vars.fade, 1);
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
