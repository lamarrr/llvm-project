// Port of https://www.shadertoy.com/view/ddcBz8; author: barth.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddcBz8.json.
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
f32x3 rotateAround(Context &__ctx, f32x3 vector, f32x3 axis, float theta) {
  return cos(theta) * vector + cross(axis, vector) * sin(theta) +
         axis * dot(axis, vector) * (1.0f - cos(theta));
}
bool rayIntersectSphere(Context &__ctx, f32x3 rayOrigin, f32x3 rayDir,
                        f32x3 spherePosition, float sphereRadius, float &t0,
                        float &t1) {
  f32x3 relativeOrigin = rayOrigin - spherePosition;
  float a = 1.0f;
  float b = 2.0f * dot(relativeOrigin, rayDir);
  float c = dot(relativeOrigin, relativeOrigin) - sphereRadius * sphereRadius;
  float d = b * b - 4.0f * a * c;
  if (d < 0.0f)
    return false;
  float s = sqrt(d);
  float r0 = (-b - s) / (2.0f * a);
  float r1 = (-b + s) / (2.0f * a);
  t0 = min(r0, r1);
  t1 = max(r0, r1);
  return t1 > 0.0f;
}
const float EARTH_RADIUS = 1000e3f;
const float ATMOSPHERE_RADIUS = 100e3f;
const f32x3 SUN_DIR = f32x3(0.0f, 1.0f, 0.0f);
struct Object {
  float radius;
  f32x3 position;
};
const Object object = Object(EARTH_RADIUS, f32x3(0.0f));
struct Atmosphere {
  float radius;
  float falloff;
  float sunIntensity;
  float rayleighStrength;
  float mieStrength;
  float densityModifier;
  float redWaveLength;
  float greenWaveLength;
  float blueWaveLength;
  float mieHaloRadius;
};
const Atmosphere atmosphere =
    Atmosphere(EARTH_RADIUS + ATMOSPHERE_RADIUS, 1.0f, 10.0f, 1.0f, 1.0f, 1.0f,
               700.0f, 530.0f, 440.0f, 0.6f);
f32x2 densityAtPoint(Context &__ctx, f32x3 samplePoint) {
  float heightAboveSurface =
      length(samplePoint - object.position) - object.radius;
  float height01 = heightAboveSurface / (atmosphere.radius - object.radius);
  f32x2 localDensity = f32x2(
      atmosphere.densityModifier * exp(-height01 * atmosphere.falloff),
      atmosphere.densityModifier * exp(-height01 * atmosphere.falloff * 0.5f));
  localDensity *= (1.0f - height01);
  return localDensity;
}
f32x2 opticalDepth(Context &__ctx, f32x3 rayOrigin, f32x3 rayDir,
                   float rayLength) {
  f32x3 densitySamplePoint = rayOrigin;
  float stepSize = rayLength / float(120 - 1);
  f32x2 accumulatedOpticalDepth = f32x2(0.0f);
  for (int i = 0; i < 120; i++) {
    accumulatedOpticalDepth +=
        densityAtPoint(__ctx, densitySamplePoint) * stepSize;
    densitySamplePoint += rayDir * stepSize;
  }
  return accumulatedOpticalDepth;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 vUV = fragCoord / iResolution.swizzle<0, 1>();
  float height01 = vUV.data[1];
  float lutx = vUV.data[0];
  float costheta = lutx * 2.0f - 1.0f;
  float theta = acos(costheta);
  f32x3 surfacePoint = rotateAround(__ctx, f32x3(0.0f, EARTH_RADIUS, 0.0f),
                                    f32x3(0.0f, 0.0f, 1.0f), theta);
  f32x3 surfaceNormal = normalize(surfacePoint);
  f32x3 atmospherePoint =
      surfacePoint + surfaceNormal * ATMOSPHERE_RADIUS * height01;
  float t0, t1;
  rayIntersectSphere(__ctx, atmospherePoint, SUN_DIR, f32x3(0.0f),
                     EARTH_RADIUS + ATMOSPHERE_RADIUS, t0, t1);
  f32x2 sunRayOpticalDepth = opticalDepth(__ctx, atmospherePoint, SUN_DIR, t1);
  f32x3 color =
      f32x3(sunRayOpticalDepth.data[0], sunRayOpticalDepth.data[1], 0.0f);
  color = 1.0f / (1.0f + 1e-5f * color);
  fragColor = f32x4(color, 1.0f);
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
