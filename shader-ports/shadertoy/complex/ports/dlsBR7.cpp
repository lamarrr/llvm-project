// Port of https://www.shadertoy.com/view/dlsBR7; author: adacohen.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlsBR7.json.
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
f32x3 cosPalette(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
float random(Context &__ctx, f32x2 seed) {
  float a = 12.9898f;
  float b = 78.233f;
  float c = 43758.5453f;
  float dt = dot(seed.swizzle<0, 1>(), f32x2(a, b));
  float sn = mod(dt, 3.14f);
  return fract(sin(sn) * c);
}
f32x2 spiralizeUv(Context &__ctx, f32x2 uv, float twistAmount) {
  float dist = length(uv);
  float angle = atan(uv.data[1], uv.data[0]);
  float spiralAngle = angle + twistAmount * dist;
  return dist * f32x2(cos(spiralAngle), sin(spiralAngle));
}
float spiral(Context &__ctx, f32x2 uv, float thickness, float twistAmount) {
  float blur = 20.0f;
  f32x2 spiralUv = spiralizeUv(__ctx, uv, twistAmount);
  float spiralScale = 1.0f + length(uv) * abs(twistAmount) / 10.0f;
  thickness *= pow(spiralScale, 3.0f);
  return 1.0f - smoothstep(thickness, thickness * blur, abs(spiralUv.data[0]));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  uv = uv * (1.0f + 0.5f * length(uv));
  f32x3 col = f32x3(0.0f);
  bool rotateDirection = true;
  for (float i = 0.f; i < 30.f; i++) {
    f32x2 curUv = uv;
    float rotationSpeed = random(__ctx, f32x2(i, 30.0f)) * 0.1f + 0.1f;
    if (rotateDirection) {
      rotationSpeed *= -1.0f;
    }
    rotateDirection = !rotateDirection;
    float rotation = iTime * rotationSpeed;
    float colorShiftSpeed = random(__ctx, f32x2(i, 31.0f)) * 0.1f + 0.1f;
    float thickness = random(__ctx, f32x2(i, 11.0f)) * 0.002f + 0.004f;
    float twistSpeed = random(__ctx, f32x2(i, 3.0f)) * 0.02f + 0.02f;
    float driftSpeed = random(__ctx, f32x2(i, 14.0f)) * 0.1f + 0.1f;
    f32x2 driftAmplitude =
        f32x2(random(__ctx, f32x2(i, 4.0f)), random(__ctx, f32x2(i, 5.0f))) *
        0.4f;
    const float maxTwist = 10.0f;
    float twistAmount =
        maxTwist *
        sin(iTime * twistSpeed + random(__ctx, f32x2(i, 6.0f)) * 6.28318f);
    f32x2 offset =
        f32x2(random(__ctx, f32x2(i, 0.0f)), random(__ctx, f32x2(i, 1.0f))) -
        0.5f;
    offset += f32x2(driftAmplitude.data[0] *
                        cos(iTime * driftSpeed +
                            random(__ctx, f32x2(i, 2.0f)) * 6.28318f),
                    driftAmplitude.data[1] *
                        sin(iTime * driftSpeed +
                            random(__ctx, f32x2(i, 7.0f)) * 6.28318f)) *
              0.5f;
    curUv = curUv + offset;
    curUv =
        f32x2x2(cos(rotation), -sin(rotation), sin(rotation), cos(rotation)) *
        curUv;
    float val = spiral(__ctx, curUv, thickness, twistAmount);
    float spiralColorT =
        random(__ctx, f32x2(i, 10.0f)) + iTime * colorShiftSpeed;
    f32x3 spiralColor =
        cosPalette(__ctx, spiralColorT, f32x3(1.00f, 0.44f, 0.00f),
                   f32x3(0.46f, 0.52f, 0.32f), f32x3(0.25f, 0.62f, 0.96f),
                   f32x3(0.04f, 0.40f, 0.69f)) +
        f32x3(0.f, 0.f, 0.15f);
    col += val * spiralColor * 0.35f;
  }
  const float overExposureThreshold = 3.0f;
  col = mix(col, f32x3(1.0f),
            smoothstep(overExposureThreshold, overExposureThreshold + 1.0f,
                       length(col)));
  fragColor = f32x4(col, 1.0f);
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
