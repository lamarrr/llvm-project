// Port of https://www.shadertoy.com/view/Dlj3zm; author: Xenolupus.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dlj3zm.json.
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

float PHI = 1.61803398874989484820459f;
float gold_noise(Context &__ctx, f32x2 xy, float seed) {
  return fract(tan(distance(xy * PHI, xy) * seed) * xy.data[0]);
}
float waveMaker(Context &__ctx, f32x2 uv, float timeOffset, f32x2 distortOffset,
                float yOffset) {
  float sineWaveDistort =
      1.0f -
      sin(iTime + timeOffset + uv.data[0] * 20.0f * distortOffset.data[0]) *
          0.2f * distortOffset.data[1];
  float bigSineWave1 =
      sin(iTime + timeOffset + uv.data[0] * 5.0f * distortOffset.data[0]) *
      0.03f * sineWaveDistort * distortOffset.data[1];
  float sineWave1 =
      sin(iTime + timeOffset + uv.data[0] * 30.0f * distortOffset.data[0]) *
          0.03f * sineWaveDistort * distortOffset.data[1] +
      0.8f;
  float wave = clamp(
      (uv.data[1] - sineWave1 + bigSineWave1 + yOffset) * 1000.0f, 0.0f, 1.0f);
  return wave;
}
float combinedWave(Context &__ctx, float totalTimeOffset, float totalYOffset,
                   f32x2 uv) {
  float wave1 = waveMaker(__ctx, uv, 0.0f + totalTimeOffset, f32x2(1.0f, 1.0f),
                          0.0f + totalYOffset);
  float wave2 = waveMaker(__ctx, uv, 0.01f + totalTimeOffset,
                          f32x2(1.01f, 0.99f), 0.02f + totalYOffset);
  float wave3 = waveMaker(__ctx, uv, 0.02f + totalTimeOffset,
                          f32x2(1.03f, 1.11f), 0.04f + totalYOffset);
  float wave4 = waveMaker(__ctx, uv, 0.03f + totalTimeOffset,
                          f32x2(0.99f, 1.03f), 0.08f + totalYOffset);
  return wave1 * 0.25f + wave2 * 0.25f + wave3 * 0.25f + wave4 * 0.25f;
}
f32x3 beachCol(Context &__ctx, f32x2 uv, f32x2 fragCoord) {
  float wave1 = waveMaker(__ctx, uv, 0.0f, f32x2(1.0f, 1.0f), 0.0f);
  float wave2 = waveMaker(__ctx, uv, 0.01f, f32x2(1.01f, 0.99f), 0.02f);
  float wave3 = waveMaker(__ctx, uv, 0.02f, f32x2(1.03f, 1.11f), 0.04f);
  float wave4 = waveMaker(__ctx, uv, 0.03f, f32x2(0.99f, 1.03f), 0.08f);
  float wave = combinedWave(__ctx, 0.0f, 0.0f, uv);
  f32x3 waterCol = f32x3(0.0f, 0.5f, 1.0f);
  f32x3 frothCol = f32x3(0.95f, 0.95f, 0.95f);
  f32x3 waveCol = mix(waterCol, frothCol, 1.0f - wave + 0.25f);
  float wet =
      clamp(combinedWave(__ctx, -0.5f, 0.02f, uv) * 1000.0f, 0.0f, 1.0f);
  float sandMask = 1.0f - clamp(wave * 1000.0f, 0.0f, 1.0f);
  float sandSpecs =
      clamp((gold_noise(__ctx, fragCoord, fract(123.12f)) - 0.05f) * 20.0f,
            0.0f, 1.0f);
  f32x3 sandCol1 = f32x3(0.525f, 0.4f, 0.2f);
  f32x3 sandCol2 = f32x3(0.95f, 0.95f, 0.7f);
  f32x3 sancCol = mix(sandCol1, sandCol2, sandSpecs - wet * 0.3f);
  return mix(waveCol, sancCol, sandMask);
}
float boardWalkBoards(Context &__ctx, f32x2 uv) {
  return clamp((sin(uv.data[0] * 50.0f + 4.8f) + 0.85f) * 50.0f, 0.0f, 1.0f);
}
float boardWalkMask(Context &__ctx, f32x2 uv) {
  float boardwalk = 1.0f - clamp((uv.data[1] - 0.55f) * 1000.0f, 0.0f, 1.0f);
  boardwalk *= boardWalkBoards(__ctx, uv);
  boardwalk += 1.0f - clamp((uv.data[1] - 0.45f) * 1000.0f, 0.0f, 1.0f);
  return clamp(boardwalk, 0.0f, 1.0f);
}
float findDis(Context &__ctx, f32x2 pos1, f32x2 pos2) {
  return sqrt(pow(pos2.data[1] - pos1.data[1], 2.0f) +
              pow(pos2.data[0] - pos1.data[0], 2.0f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 boardwalkColor = f32x3(0.8f, 0.7f, 0.5f) *
                         clamp(boardWalkBoards(__ctx, uv) + 0.7f, 0.0f, 1.0f);
  boardwalkColor =
      boardwalkColor *
      clamp(0.8f + (1.0f - (boardWalkMask(__ctx, uv) -
                            boardWalkMask(__ctx, uv + f32x2(0.0f, 0.01f)))),
            0.0f, 1.0f);
  f32x3 beachColor = beachCol(__ctx, uv, fragCoord) -
                     boardWalkMask(__ctx, uv - f32x2(0.01f, -0.01f)) * 0.2f;
  f32x3 result = mix(beachColor, boardwalkColor, boardWalkMask(__ctx, uv));
  float aspect = iResolution.data[0] / iResolution.data[1];
  f32x2 aspectScaledUV = f32x2(uv.data[0] * aspect, uv.data[1]);
  float coconutDis =
      1.0f - findDis(__ctx, aspectScaledUV, (f32x2(0.5f * aspect, 0.2f)));
  float coconutMask = clamp((coconutDis - 0.9f) * 1000.0f, 0.0f, 1.0f);
  float coconutDisShade =
      coconutMask *
      clamp((1.0f -
             findDis(__ctx, aspectScaledUV, (f32x2(0.52f * aspect, 0.22f))) -
             0.6f) *
                2.25f,
            0.0f, 1.0f);
  float coconutShadowDis =
      1.0f - findDis(__ctx, aspectScaledUV, (f32x2(0.48f * aspect, 0.18f)));
  float coconutShadowMask =
      clamp(1.8f - clamp((coconutShadowDis - 0.9f) * 1000.0f, 0.0f, 1.0f), 0.0f,
            1.0f);
  float coconutHoleDis1 = clamp(
      1.0f - findDis(__ctx, aspectScaledUV, (f32x2(0.52f * aspect, 0.22f))),
      0.0f, 1.0f);
  float coconutHoleMask1 =
      clamp(1.5f - clamp((coconutHoleDis1 - 0.9875f) * 1000.0f, 0.0f, 1.0f),
            0.0f, 1.0f);
  float coconutHoleDis2 = clamp(
      1.0f - findDis(__ctx, aspectScaledUV, (f32x2(0.52f * aspect, 0.25f))),
      0.0f, 1.0f);
  float coconutHoleMask2 =
      clamp(1.5f - clamp((coconutHoleDis2 - 0.9875f) * 1000.0f, 0.0f, 1.0f),
            0.0f, 1.0f);
  float coconutHoleDis3 = clamp(
      1.0f - findDis(__ctx, aspectScaledUV, (f32x2(0.50f * aspect, 0.25f))),
      0.0f, 1.0f);
  float coconutHoleMask3 =
      clamp(1.5f - clamp((coconutHoleDis3 - 0.9875f) * 1000.0f, 0.0f, 1.0f),
            0.0f, 1.0f);
  f32x3 coconutColor = f32x3(0.5f, 0.3f, 0.0f) * coconutDisShade *
                       coconutHoleMask1 * coconutHoleMask2 * coconutHoleMask3;
  result = mix(result * coconutShadowMask, coconutColor, coconutMask);
  fragColor = f32x4(result, 0.0f);
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
