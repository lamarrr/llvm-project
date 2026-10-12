// Port of https://www.shadertoy.com/view/Dl33zX; author: drliquidglitch.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dl33zX.json.
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
float time;
float plane(Context &__ctx, f32x3 p, f32x3 r) {
  p = min(p, r);
  return p.data[1];
}
float sphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float torus(Context &__ctx, f32x3 p, f32x2 t) {
  p.data[2] += cos(p.data[1] * .01f * sin(iTime));
  p.data[1] += sin(p.data[0] * .5f * cos(iTime));
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
float map(Context &__ctx, f32x3 p) {
  float mainBox = plane(__ctx, p, f32x3(1));
  float leftSphere = sphere(__ctx, p + f32x3(0, 0, 3), 1.f);
  float scene = max(mainBox, leftSphere);
  for (float i = 0.f; i < 14.f; i++) {
    float rightTorus =
        torus(__ctx, p - f32x3(2.f * sin(i), i - 7.f, i), f32x2(i, 0.5f));
    scene = min(scene, rightTorus);
  }
  return scene;
}
float raycast(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dist, res = 0.f;
  for (int i = 0; i < 128; i++) {
    dist = map(__ctx, ro + rd * res);
    if (dist < .0001f || res > 120.f)
      break;
    res += dist;
  }
  if (res > 120.f)
    res = 0.f;
  return res;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f) /
             f32x2(iResolution.data[1] / iResolution.data[0], 1);
  time = mod(iTime, 62.82f);
  f32x3 rayOrigin = f32x3(cos(time * .2f) + 4.f, 5, sin(time * .2f) + 14.f);
  f32x3 cameraForward = normalize(f32x3(0) - rayOrigin);
  f32x3 cameraLeft = normalize(cross(cameraForward, f32x3(0, 1, 0)));
  f32x3 cameraUp = normalize(cross(cameraLeft, cameraForward));
  f32x3 rayDirection =
      f32x3x3(cameraLeft, cameraUp, cameraForward) * normalize(f32x3(uv, .5f));
  f32x3 lightDirection = normalize(f32x3(-.1f, .4f, -.3f));
  f32x3 backgroundColor =
      f32x3(.5f * uv.data[1], .25f * sin(iTime), .1f * cos(iTime)) -
      length(uv) * .1f;
  f32x3 color = backgroundColor;
  float result = raycast(__ctx, rayOrigin, rayDirection);
  if (result > 0.f) {
    f32x3 hitPos = rayOrigin + rayDirection * result;
    f32x2 e = f32x2(.00035f, -.00035f);
    f32x3 normals = normalize(
        e.swizzle<0, 1, 1>() * map(__ctx, hitPos + e.swizzle<0, 1, 1>()) +
        e.swizzle<1, 1, 0>() * map(__ctx, hitPos + e.swizzle<1, 1, 0>()) +
        e.swizzle<1, 0, 1>() * map(__ctx, hitPos + e.swizzle<1, 0, 1>()) +
        e.swizzle<0, 0, 0>() * map(__ctx, hitPos + e.swizzle<0, 0, 0>()));
    f32x3 albedo = f32x3(.0f, .2f, .4f);
    float diffuse = max(0.f, dot(normals, lightDirection));
    float fresnel = pow(1.f + dot(normals, rayDirection), 4.f);
    float specular = pow(
        max(dot(reflect(-lightDirection, normals), -rayDirection), 0.f), 30.f);
    float ao = clamp(map(__ctx, hitPos + normals * .05f) / .05f, 0.f, 1.f);
    float sss =
        smoothstep(0.f, 1.f, map(__ctx, hitPos + lightDirection * .4f) / .4f);
    color = mix(specular + albedo * (ao + .2f) * (diffuse + sss * .1f),
                backgroundColor, fresnel);
    color = mix(backgroundColor, color, exp(-.002f * result * result * result));
  }
  fragColor = f32x4(pow(color, f32x3(sin(iTime))), 1);
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
