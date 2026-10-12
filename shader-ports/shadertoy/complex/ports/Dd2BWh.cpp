// Port of https://www.shadertoy.com/view/Dd2BWh; author: shaderNoob4489.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dd2BWh.json.
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
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ap, ab) / dot(ab, ab);
  t = clamp(t, 0.f, 1.f);
  f32x3 c = a + t * ab;
  return length(c - p) - r;
}
float sdCylinder(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ap, ab) / dot(ab, ab);
  f32x3 c = a + t * ab;
  float x = length(p - c) - r;
  float y = (abs(t - .5f) - .5f) * length(ab);
  float e = length(max(f32x2(x, y), 0.f));
  float i = min(max(x, y), 0.f);
  return e + i;
}
float sdTours(Context &__ctx, f32x3 p, f32x2 r) {
  float x = length(p.swizzle<0, 2>()) - r.data[0];
  return length(f32x2(x, p.data[1])) - r.data[1];
}
float sdBox(Context &__ctx, f32x3 p, f32x3 size) {
  f32x3 d = abs(p) - size;
  float e = length(max(d, 0.f));
  float i = min(max(max(d.data[0], d.data[1]), d.data[2]), 0.f);
  return e + i;
}
float GetDist(Context &__ctx, f32x3 p) {
  f32x4 sphere = f32x4(0, 1, 6, 1);
  float sphereDist = length(p - sphere.swizzle<0, 1, 2>()) - sphere.data[3];
  float planeDist = p.data[1];
  float cd = sdCapsule(__ctx, p, f32x3(0, 1, 6), f32x3(1, 2, 6), .2f);
  float td = sdTours(__ctx, p - f32x3(0, .5f, 6), f32x2(1.5f, .3f));
  float bd = sdBox(__ctx, p - f32x3(-3, .75f, 6), f32x3(.75f));
  float cyld = sdCylinder(__ctx, p, f32x3(0, .3f, 3), f32x3(2, .3f, 5), .3f);
  float d = min(planeDist, cd);
  d = min(td, d);
  d = min(bd, d);
  d = min(cyld, d);
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 50; i++) {
    f32x3 p = ro + dO * rd;
    float dS = GetDist(__ctx, p);
    dO += dS;
    if (dO > 50.f || dO < .01f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(.01f, 0);
  f32x3 normal = f32x3(GetDist(__ctx, p + e.swizzle<0, 1, 1>()),
                       GetDist(__ctx, p + e.swizzle<1, 0, 1>()),
                       GetDist(__ctx, p + e.swizzle<1, 1, 0>()));
  return normalize(normal);
}
float shadow(Context &__ctx, f32x3 p, f32x3 lightDir, float ldis) {
  float shadow = 1.0f;
  float t = 0.001f;
  for (int i = 0; i < 50; i++) {
    float d = GetDist(__ctx, p + t * lightDir);
    t += d;
    if (t > 30.f)
      break;
    if (d < .01f)
      return 0.f;
    shadow = min(2.f * d / t, shadow);
  }
  return shadow;
}
float ao(Context &__ctx, f32x3 p, f32x3 n) {
  float a = 0.f;
  float e = .1f;
  int ignored;
  float weight = .5f;
  for (int i = 1; i <= 5; i++) {
    float d = e * float(i);
    a += weight * (1.0f - (d - GetDist(__ctx, p + n * d)));
    weight *= .5f;
  }
  return a;
}
f32x3 GetLight(Context &__ctx, f32x3 p) {
  f32x3 lightPos = f32x3(0, 5, 0);
  lightPos += f32x3(5.f * cos(iTime) * .5f, 0, 5.f * sin(iTime) * .5f);
  f32x3 lightDir = normalize(lightPos - p);
  f32x3 normal = GetNormal(__ctx, p);
  float dif = clamp(dot(lightDir, normal), 0.f, 1.f);
  float ldis = length(lightPos - p);
  dif *= shadow(__ctx, p + normal * .01f * 2.f, lightDir, ldis);
  f32x3 ambient = ao(__ctx, p, normal) * f32x3(.2f);
  return f32x3(dif) + ambient;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0);
  f32x3 ro = f32x3(0, 2, 0);
  f32x3 rd = normalize(f32x3(uv.data[0], uv.data[1] - .2f, 1));
  f32x3 p = ro + rd * RayMarch(__ctx, ro, rd);
  col = GetLight(__ctx, p);
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
