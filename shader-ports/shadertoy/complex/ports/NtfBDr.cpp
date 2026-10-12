// Port of https://www.shadertoy.com/view/NtfBDr; author: libreliu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NtfBDr.json.
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
float sphereSDF(Context &__ctx, f32x3 p, f32x3 c, float r) {
  return sqrt((p.data[0] - c.data[0]) * (p.data[0] - c.data[0]) +
              (p.data[1] - c.data[1]) * (p.data[1] - c.data[1]) +
              (p.data[2] - c.data[2]) * (p.data[2] - c.data[2])) -
         r;
}
float groundSDF(Context &__ctx, f32x3 p) { return abs(p.data[1] + 2.5f); }
float sceneSDF(Context &__ctx, f32x3 p) {
  return min(sphereSDF(__ctx, p, f32x3(0.0f, 0.0f, 0.0f), 1.5f),
             groundSDF(__ctx, p));
}
float rayMarch(Context &__ctx, f32x3 rayO, f32x3 rayD, float &dist) {
  f32x3 curIsect = rayO;
  rayD = normalize(rayD);
  float t = 0.0f;
  for (int i = 0; i < 100; i++) {
    dist = sceneSDF(__ctx, curIsect);
    if (dist < 1e-2f || dist > 100.5f) {
      break;
    }
    t += dist;
    curIsect = rayO + t * rayD;
  }
  return t;
}
f32x3 sceneNormal(Context &__ctx, f32x3 p) {
  float pSDF = sceneSDF(__ctx, p);
  f32x2 eps = f32x2(1e-3f, 0);
  f32x3 grad = pSDF - f32x3(sceneSDF(__ctx, p - eps.swizzle<0, 1, 1>()),
                            sceneSDF(__ctx, p - eps.swizzle<1, 0, 1>()),
                            sceneSDF(__ctx, p - eps.swizzle<1, 1, 0>()));
  return normalize(grad);
}
f32x3 shadeScene(Context &__ctx, f32x3 isect) {
  f32x3 lightPos =
      f32x3(5.0f * cos(2.0f * iTime), 0.0f, 5.0f * sin(2.0f * iTime));
  f32x3 light2Isect = normalize(isect - lightPos);
  float rayDist, lightT = rayMarch(__ctx, lightPos, light2Isect, rayDist);
  float visibility = 0.0f;
  if (length(lightPos + lightT * light2Isect - isect) < 10.f * 1e-2f) {
    visibility = 1.0f;
  }
  f32x3 lightColor = f32x3(0.5f, 0.5f, 0.5f);
  f32x3 lightL = lightColor * 5.5f;
  f32x3 isectN = sceneNormal(__ctx, isect);
  f32x3 lightContrib =
      visibility * clamp(dot(isectN, normalize(lightPos - isect)), 0.f, 1.f) *
      lightL / length(isect - lightPos);
  return lightContrib;
}
f32x3 shadeSkybox(Context &__ctx, f32x3 rayD) { return f32x3(0.0f); }
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float aspectRatio = iResolution.data[0] / iResolution.data[1];
  f32x3 camPos = f32x3(5.0f, 0.0f, 0.0f);
  f32x3 camLookat = f32x3(-1.0f, 0.0f, 0.0f);
  f32x3 camUp = f32x3(0.0f, 0.0f, 1.0f);
  f32x3 camZ = normalize(camLookat - camPos);
  f32x3 camX = normalize(cross(camZ, normalize(camUp)));
  f32x3 camY = cross(camX, camZ);
  float camVertFOV = 120.0f * (3.14159265359f / 180.0f) / 2.f;
  f32x3 camScrC = camPos + camZ;
  f32x3 camScrP = camScrC + camY * (uv.data[0] - 0.5f) * tan(camVertFOV) +
                  camX * (uv.data[1] - 0.5f) * tan(camVertFOV) / aspectRatio;
  f32x3 rayD = normalize(camScrP - camPos);
  f32x3 rayO = camPos;
  float distClosest, tClosest = rayMarch(__ctx, rayO, rayD, distClosest);
  f32x3 outColor;
  if (distClosest > 10.0f * 1e-2f) {
    outColor = shadeSkybox(__ctx, rayD);
  } else {
    outColor = shadeScene(__ctx, rayO + tClosest * rayD);
  }
  fragColor = f32x4(outColor, 1.0f);
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
