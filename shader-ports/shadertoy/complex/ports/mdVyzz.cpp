// Port of https://www.shadertoy.com/view/mdVyzz; author: hyrofumi.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdVyzz.json.
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

float roundBoxSDF(Context &__ctx, f32x3 p, f32x3 b, float r) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f) - r;
}
float boxSDF(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
f32x2x2 Rotate(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float smoothIntersectSDF(Context &__ctx, float distA, float distB, float k) {
  float h = clamp(0.5f - 0.5f * (distA - distB) / k, 0.f, 1.f);
  return mix(distA, distB, h) + k * h * (1.f - h);
}
float smoothUnionSDF(Context &__ctx, float distA, float distB, float k) {
  float h = clamp(0.5f + 0.5f * (distA - distB) / k, 0.f, 1.f);
  return mix(distA, distB, h) - k * h * (1.f - h);
}
float smoothDifferenceSDF(Context &__ctx, float distA, float distB, float k) {
  float h = clamp(0.5f - 0.5f * (distA + distB) / k, 0.f, 1.f);
  return mix(distA, -distB, h) + k * h * (1.f - h);
}
float GetDist(Context &__ctx, f32x3 p) {
  float d = 0.f;
  f32x3 bPos = p + f32x3(0, sin(iTime * 0.25f), 0);
  bPos.set_swizzle<0, 1>(bPos.swizzle<0, 1>() * (Rotate(__ctx, iTime * 0.25f)));
  bPos.set_swizzle<0, 2>(bPos.swizzle<0, 2>() * (Rotate(__ctx, iTime * 0.5f)));
  d = roundBoxSDF(__ctx, bPos, f32x3(.5f, .5f, .5f), .1f);
  float planeDist = p.data[1];
  d = smoothUnionSDF(__ctx, d, planeDist, smoothstep(1.f, 0.f, p.data[1]));
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float ds = GetDist(__ctx, p);
    dO += ds;
    if (dO > 100.f || ds < .01f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(.01f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
float GetLight(Context &__ctx, f32x3 p) {
  f32x3 lightPos = f32x3(5.f * sin(iTime), 5.f, 5.0f * cos(iTime));
  f32x3 l = normalize(lightPos - p);
  f32x3 n = GetNormal(__ctx, p);
  float dif = dot(n, l);
  dif = clamp(dif, 0.f, 1.f);
  float d = RayMarch(__ctx, p + n * .01f * 2.f, l);
  if (d < length(lightPos - p))
    dif *= .1f;
  return dif;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 uv2 = (2.f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
              iResolution.data[0];
  f32x3 ro = f32x3(0, 1, -3.5f);
  f32x3 rd = normalize(f32x3(uv2, 1));
  float d = RayMarch(__ctx, ro, rd);
  f32x3 p = ro + rd * d;
  float dif = GetLight(__ctx, p);
  f32x3 color = f32x3(dif);
  fragColor = f32x4(color, 1.0f);
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
