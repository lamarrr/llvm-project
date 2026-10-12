// Port of https://www.shadertoy.com/view/DtXSR8; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtXSR8.json.
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
struct object {
  int type;
  f32x4 param;
};
object[4] objects;
struct light {
  f32x3 pos;
  f32x3 col;
};
light[2] lights;
float[2] animTime;
float camTime;
float getDistance(Context &__ctx, f32x3 pos) {
  float minDist = 110.0f;
  for (int n = 0; n < objects.length(); n++) {
    switch (objects[n].type) {
    case 0:
      minDist = min(
          minDist,
          pos.data[1] - objects[n].param.data[1] +
              0.2f * sin(animTime[1]) *
                  sin(4.0f * animTime[0] - length(pos) / 0.2f) *
                  smoothstep(
                      22.f * clamp(0.9f + cos(animTime[1]), 0.f, 2.f), -0.1f,
                      pos.data[0] * pos.data[0] + pos.data[2] * pos.data[2]));
      break;
    case 1:
      minDist = min(minDist, length(pos - objects[n].param.swizzle<0, 1, 2>()) -
                                 objects[n].param.data[3]);
      break;
    case 2:
      minDist =
          min(minDist, length(pos - objects[n].param.swizzle<0, 1, 2>()) -
                           objects[n].param.data[3] +
                           0.1f * cos(animTime[1]) *
                               smoothstep(-0.2f, 1.2f,
                                          abs(cos(pos.data[0] * pos.data[1] *
                                                  pos.data[2]))));
      break;
    }
  }
  return minDist;
}
float rayMarch(Context &__ctx, f32x3 rayOrigin, f32x3 rayDirection) {
  float rayLength = 0.01f;
  float radSphere;
  for (int n = 0; n < 70; n++) {
    radSphere = getDistance(__ctx, rayOrigin + rayLength * rayDirection);
    rayLength += radSphere;
    if (rayLength > 110.0f || abs(radSphere) < 0.01f)
      break;
  }
  return rayLength;
}
f32x3 guessNormal(Context &__ctx, f32x3 pos) {
  f32x2 tangent = f32x2(0.01f, 0);
  f32x3 normal = getDistance(__ctx, pos) -
                 f32x3(getDistance(__ctx, pos - tangent.swizzle<0, 1, 1>()),
                       getDistance(__ctx, pos - tangent.swizzle<1, 0, 1>()),
                       getDistance(__ctx, pos - tangent.swizzle<1, 1, 0>()));
  return normalize(normal);
}
f32x3 getSpotlight(Context &__ctx, f32x3 pos, light myLight) {
  f32x3 lightDirection = normalize(myLight.pos - pos);
  f32x3 surfaceNormal = guessNormal(__ctx, pos);
  float diffLight = clamp(dot(lightDirection, surfaceNormal), 0.0f, 1.0f);
  diffLight = pow(diffLight, 3.f) * 1.5f;
  if (rayMarch(__ctx, pos + 2.0f * 0.01f * surfaceNormal, lightDirection) <
      length(myLight.pos - pos))
    diffLight *= 0.2f;
  return myLight.col * f32x3(diffLight);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 col = f32x3(0);
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  float myDist;
  animTime = float[](mod(2.133333f * iTime, 6.283185f),
                     mod(0.2666666f * iTime, 6.283185f));
  f32x3 camPos = f32x3(0, 2.5f, 0);
  f32x3 camDir = normalize(f32x3(uv.data[0], uv.data[1] * cos(.15f) - sin(.1f),
                                 uv.data[1] * sin(.15f) + cos(.1f)));
  camTime = 0.2f * iTime;
  camPos += f32x3(9.f * sin(camTime), 0, -9.f * cos(camTime));
  camDir = f32x3(camDir.data[0] * cos(camTime) - camDir.data[2] * sin(camTime),
                 camDir.data[1],
                 camDir.data[0] * sin(camTime) + camDir.data[2] * cos(camTime));
  objects = object[](
      object(0, f32x4(0)),
      object(2, f32x4(0, 2.9f, 0, 2.0f + cos(animTime[1]))),
      object(1, f32x4(-3, 0.4f + 3.f * abs(sin(animTime[0])), 5, 0.5f)),
      object(1, f32x4(3, 0.4f + 3.f * abs(cos(1.5f * animTime[0])), -5, 0.5f)));
  lights = light[](
      light(f32x3(9.f * cos(camTime + .1f), 3.5f, 9.f * sin(camTime + .1f)),
            normalize(f32x3(245.f * (1.f + 0.7f * cos(animTime[1])), 10, 193)) *
                1.5f),
      light(f32x3(8.f * sin(camTime - 0.4f), 4.5f, 8.f * -cos(camTime - 0.4f)),
            normalize(f32x3(73, 10, 245)) * 0.1f));
  myDist = rayMarch(__ctx, camPos, camDir);
  for (int n = 0; n < lights.length(); n++) {
    col += getSpotlight(__ctx, camPos + myDist * camDir, lights[n]);
  }
  col = pow(col, f32x3(.4545f));
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
