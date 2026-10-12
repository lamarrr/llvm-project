// Port of https://www.shadertoy.com/view/ddsfR7; author: shlynz.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddsfR7.json.
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
float noise21(Context &__ctx, f32x2 p) {
  return fract(sin(p.data[0] * 100.f + p.data[1] * 8943.f) * 5647.f);
}
float smoothNoise(Context &__ctx, f32x2 uv) {
  f32x2 localUv = smoothstep(.0f, 1.f, fract(uv));
  f32x2 id = floor(uv);
  float bottomLeft = noise21(__ctx, id);
  float bottomRight = noise21(__ctx, id + f32x2(1.f, .0f));
  float bottom = mix(bottomLeft, bottomRight, localUv.data[0]);
  float topLeft = noise21(__ctx, id + f32x2(.0f, 1.f));
  float topRight = noise21(__ctx, id + f32x2(1.f, 1.f));
  float top = mix(topLeft, topRight, localUv.data[0]);
  return mix(bottom, top, localUv.data[1]);
}
float smoothNoiseLayers(Context &__ctx, f32x2 uv, float amountOfLayers) {
  float result = .0f;
  float sumOfAmplitudes = .0f;
  float frequency = 4.f;
  float amplitude = 1.f;
  for (float i = .0f; i < amountOfLayers; i++) {
    result += smoothNoise(__ctx, uv * frequency) * amplitude;
    sumOfAmplitudes += amplitude;
    frequency *= 2.f;
    amplitude /= 2.f;
  }
  return result / sumOfAmplitudes;
}
f32x2 rotateUv(Context &__ctx, f32x2 uv, float rotation) {
  f32x2 cosVal = uv * cos(rotation);
  f32x2 sinVal = uv * sin(rotation);
  return f32x2(cosVal.data[0] + sinVal.data[1],
               cosVal.data[1] - sinVal.data[0]);
}
f32x2 remapRotateUv(Context &__ctx, f32x2 uv, f32x2 newOrigin, float rotation) {
  f32x2 newPoint = rotateUv(__ctx, newOrigin, rotation);
  f32x2 movingUv = uv - newPoint;
  return movingUv;
}
float glowyOutline(Context &__ctx, float sdfValue, f32x2 point) {
  float noiseOffset =
      .5f - smoothNoiseLayers(__ctx, point + (iTime * .2f), 5.f);
  sdfValue += noiseOffset * .02f;
  noiseOffset *= .1f;
  return pow(.01f * (.5f + noiseOffset) / abs(sdfValue),
             1.f / (1.1f + noiseOffset));
}
f32x3 runeCircle(Context &__ctx, f32x2 point, float size, f32x3 color) {
  float sdf = length(point) - size;
  return color * glowyOutline(__ctx, sdf, point);
}
f32x3 runeSquare(Context &__ctx, f32x2 point, float size, f32x3 color) {
  f32x2 dist = abs(point) - f32x2(size);
  float outerDistance = length(max(dist, .0f));
  float innerDistance = min(max(dist.data[0], dist.data[1]), .0f);
  float sdf = outerDistance + innerDistance;
  return color * glowyOutline(__ctx, sdf, point);
}
f32x3 getColor(Context &__ctx) {
  return .5f + .5f * cos(iTime * .1f + f32x3(0, 2, 4));
}
f32x3 getComplementaryColor(Context &__ctx) {
  f32x3 color = getColor(__ctx);
  float minVal = min(color.data[0], min(color.data[1], color.data[2]));
  float maxVal = max(color.data[0], max(color.data[1], color.data[2]));
  return f32x3(maxVal - minVal) - color;
}
f32x3 glowySeal(Context &__ctx, f32x2 uv) {
  float piFourth = 3.1415926536f / 4.f;
  float randomBrightnessReduction =
      .9f + smoothNoiseLayers(__ctx, uv + iTime, 5.f) * .1f;
  f32x3 col = getColor(__ctx) * randomBrightnessReduction;
  f32x3 inverseCol = getComplementaryColor(__ctx) * randomBrightnessReduction;
  f32x3 finalColor = f32x3(.0f);
  finalColor += runeSquare(__ctx, rotateUv(__ctx, uv, iTime), .6f, col);
  finalColor +=
      runeSquare(__ctx, rotateUv(__ctx, uv, iTime + piFourth), .6f, col);
  finalColor +=
      runeSquare(__ctx, rotateUv(__ctx, uv, -iTime), .35f, inverseCol);
  finalColor += runeCircle(__ctx, uv, .7f, col);
  finalColor += runeCircle(__ctx, uv, .42f, inverseCol);
  for (float i = .0f; i < 8.f; i++) {
    finalColor += runeCircle(
        __ctx, remapRotateUv(__ctx, uv, f32x2(.6f), -iTime - piFourth * i),
        .15f, col);
  }
  return finalColor;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>() * 2.f - 1.f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  fragColor = f32x4(glowySeal(__ctx, uv), 1.0f);
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
