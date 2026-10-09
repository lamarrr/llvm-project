// Port of https://www.shadertoy.com/view/msySRm; author: Virg.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/msySRm.json.
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
f32x3 camPose = f32x3(-20.f, 8.f, 0.f);
f32x3 camDir = f32x3(0.8f, -0.3f, 0.2f);
f32x3 light = f32x3(0.f, 20.f, 0.f);
float rev;
f32x2 ext;
float castDist = 100.f;
float displayDisCenter = 1.f;
float stepC = 0.01f;
float gFov = 3.1415926535f / 2.f;
void rayCast(Context &__ctx, f32x4 &fragColor, f32x2 ang);
f32x3 rotateX(f32x3 vec, float ang);
f32x3 rotateZ(f32x3 vec, float ang);
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  camDir.data[0] = cos(iTime);
  camDir.data[2] = sin(iTime);
  camPose.data[0] = sin(iTime) * 40.f;
  camPose.data[2] = -cos(iTime) * 40.f;
  light.data[0] = cos(iTime) * 20.f;
  light.data[2] = sin(iTime) * 20.f;
  rev = iResolution.data[0] / iResolution.data[1];
  ext = iResolution.swizzle<0, 1>();
  fragColor = f32x4(f32x3(0), 1.0f);
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv.data[0] *= rev;
  float vFov = gFov / rev;
  float gFov2 = gFov / 2.f;
  float vFov2 = vFov / 2.f;
  f32x2 ang = f32x2(gFov * (uv.data[0] / rev) - gFov / 2.f,
                    vFov * (uv.data[1] / 1.f) - vFov / 2.f);
  rayCast(__ctx, fragColor, ang);
}
f32x3 rotateXYZ(Context &__ctx, f32x3 vec, f32x3 axis, float t) {
  float x2 = vec.data[0];
  float y2 = vec.data[1];
  float z2 = vec.data[2];
  float x = axis.data[0];
  float y = axis.data[1];
  float z = axis.data[2];
  return axis * (dot(axis, vec)) + cos(t) * cross(cross(axis, vec), axis) +
         sin(t) * (cross(axis, vec));
}
void rayCast(Context &__ctx, f32x4 &fragColor, f32x2 ang) {
  f32x4 spheres[2];
  spheres[0] = f32x4(f32x3(0.f, 2.f, 0.f), 5.f);
  spheres[1] = f32x4(f32x3(0.f, 0.f, 12.f), 4.f);
  f32x3 diagonalAxis = normalize(cross(camDir, f32x3(0.f, 1.f, 0.f)));
  f32x3 verticalAxis = normalize(cross(camDir, diagonalAxis));
  f32x3 res = rotateXYZ(__ctx, camDir, verticalAxis, ang.data[0]);
  res = normalize(rotateXYZ(__ctx, res, diagonalAxis, ang.data[1]));
  float dist = 0.f;
  f32x3 color = f32x3(0.f, 0.f, 0.f);
  bool breakCasting = false;
  for (float i = 1.f; i < 100.f; i += 0.1f) {
    if (breakCasting) {
      break;
    }
    f32x3 pnt = camPose + res * i;
    dist = i / 100.f;
    if (pnt.data[1] < 0.f) {
      color = f32x3(1.f);
      break;
    }
    for (int sp = 0; sp < 2; sp++) {
      if (distance(pnt, spheres[sp].swizzle<0, 1, 2>()) < spheres[sp].data[3]) {
        f32x3 normal = normalize(pnt - spheres[sp].swizzle<0, 1, 2>());
        f32x3 shapeToLight = normalize(light - pnt);
        float colorAberation = dot(normal, shapeToLight);
        float codir = dot(-res, normal) / 2.f;
        f32x3 softLight = f32x3(0.5f, 0.5f, 0.5f);
        color = mix(softLight, f32x3(1.f), colorAberation);
        breakCasting = true;
        break;
      }
    }
  }
  color = mix(color, f32x3(0.f, 0.f, 0.f), dist);
  fragColor = f32x4(color, 1.f);
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
