// Port of https://www.shadertoy.com/view/dtVcDD; author: amaxwell.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtVcDD.json.
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
const float PI = 3.14159f;
const float ep = 0.02f;
const f32x3 up = f32x3(0.0f, 1.0f, 0.0f);
const f32x3 lightColor = f32x3(0.08f, 0.02f, -0.01f);
const float atmosphere = 0.0625f;
const int maxIterations = 96;
const float holeWidth = 0.05f;
f32x2 rotate(Context &__ctx, f32x2 vector, float angle) {
  float cosAngle = cos(angle);
  float sinAngle = sin(angle);
  return f32x2(cosAngle * vector.data[0] - sinAngle * vector.data[1],
               sinAngle * vector.data[0] + cosAngle * vector.data[1]);
}
float sceneSDF(Context &__ctx, f32x3 pos) {
  float ground = dot(pos, up) + 0.75f;
  float structure = length(abs(mod(pos, 2.0f) - 1.0f)) - 1.35f +
                    0.05f * cos(PI * pos.data[0] * 4.0f) +
                    0.05f * sin(PI * pos.data[2] * 4.0f);
  float slice1 =
      length(max(abs(mod(pos, 2.0f) - 1.0f).swizzle<0, 2>() - 1.0f, 0.5f)) -
      0.075f + 0.1f * cos(pos.data[1] * 36.0f);
  float slice2 = length(abs(mod(pos, 0.5f)) - 0.25f) - 0.975f;
  float bubble_w =
      0.8f + 0.2f * cos(PI * pos.data[2]) + 0.2f * cos(PI * pos.data[0]);
  float bubble = length(mod(pos, 0.125f) - 0.0625f) - bubble_w;
  float hole = length(abs(mod(pos.swizzle<0, 2>(), 1.0f) - 0.5f)) - holeWidth;
  return max(slice1 + slice2,
             min(-structure, max(-hole - slice2 * 0.375f, ground + bubble)));
}
f32x3 calculateRayColor(Context &__ctx, f32x3 cameraLocation,
                        f32x3 rayDirection) {
  float rayLength = 0.0f;
  for (int i = 0; i < maxIterations; i++) {
    float distance = sceneSDF(__ctx, cameraLocation + rayDirection * rayLength);
    if (abs(distance) < 0.005f)
      break;
    rayLength += distance * 0.5f;
  }
  f32x3 hitPoint = cameraLocation + rayDirection * rayLength;
  float lightIntensity =
      dot(normalize(f32x3(sceneSDF(__ctx, hitPoint + f32x3(-ep, ep, ep)),
                          sceneSDF(__ctx, hitPoint + f32x3(ep, -ep, -ep)),
                          sceneSDF(__ctx, hitPoint + f32x3(ep, ep, -ep)))),
          f32x3(0.1f));
  f32x3 baseColor = f32x3(lightIntensity) - rayLength * atmosphere;
  return baseColor * baseColor + f32x3(lightIntensity) + rayLength * lightColor;
}
f32x3 getRayDirection(Context &__ctx, f32x2 fragCoord) {
  f32x3 rayDirection =
      normalize(f32x3((2.0f * fragCoord / iResolution.swizzle<0, 1>() - 1.0f) *
                          f32x2(1.625f, 1.0f),
                      0.75f));
  rayDirection.set_swizzle<1, 2>(rotate(__ctx, rayDirection.swizzle<1, 2>(),
                                        PI * 0.25f * sin(PI * iTime * 0.125f)));
  rayDirection.set_swizzle<2, 0>(rotate(__ctx, rayDirection.swizzle<2, 0>(),
                                        PI * cos(-PI * iTime * 0.05f)));
  rayDirection.set_swizzle<0, 1>(
      rotate(__ctx, rayDirection.swizzle<0, 1>(),
             PI * 0.125f * cos(PI * iTime * 0.125f)));
  return rayDirection;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 cameraLocation =
      f32x3(1.5f * cos(PI * iTime * 0.125f),
            4.0f - 3.0f * sin(PI * iTime * 0.125f), -1.0f - iTime);
  fragColor = f32x4(calculateRayColor(__ctx, cameraLocation,
                                      getRayDirection(__ctx, fragCoord)),
                    1.0f);
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
