// Port of https://www.shadertoy.com/view/NslfWB; author: ErinZ.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NslfWB.json.
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
float logo(Context &__ctx, f32x2 uv);
float sdSphere(Context &__ctx, f32x3 p, f32x3 o, float r) {
  return length(p - o) - r;
}
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ab, ap) / dot(ab, ab);
  t = clamp(t, 0.f, 1.f);
  f32x3 c = a + t * ab;
  return length(p - c) - r;
}
float sdTorus(Context &__ctx, f32x3 p, f32x3 o, float r1, float r2) {
  p -= o;
  return length(f32x2(length(f32x2(p.data[0], p.data[2])) - r1, p.data[1])) -
         r2;
}
float sdBox(Context &__ctx, f32x3 p, f32x3 o, f32x3 s) {
  p -= o;
  return length(max(abs(p) - s, 0.f));
}
float sdCylinder(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 ab = b - a;
  f32x3 ap = p - a;
  float t = dot(ab, ap) / dot(ab, ab);
  f32x3 c = a + t * ab;
  float x = length(p - c) - r;
  float y = (abs(t - .5f) - .5f) * length(ab);
  float e = length(max(f32x2(x, y), 0.f));
  float i = min(max(x, y), 0.f);
  return e + i;
}
float f(Context &__ctx, f32x3 p) {
  float ds = sdSphere(__ctx, p, f32x3(0, 1, 3), 1.f);
  float dc = sdCapsule(__ctx, p, f32x3(1, .5f, 2), f32x3(3.5f, .5f, 3), .5f);
  float dt = sdTorus(__ctx, p, f32x3(-2, .5f, 4), 1.f, .5f);
  float db = sdBox(__ctx, p, f32x3(1, 1, 5), f32x3(1, 2, 1));
  float dcy = sdCylinder(__ctx, p, f32x3(1.5f, 1.7f, 2), f32x3(4, .5f, 5), .5f);
  float dp = p.data[1];
  float d = min(ds, dc);
  d = min(d, dt);
  d = min(d, dp);
  d = min(d, db);
  d = min(d, dcy);
  return d;
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.1f;
  for (int i = 0; i < 128; i++) {
    f32x3 p = ro + t * rd;
    float d = f(__ctx, p);
    t += d;
    if (d < .001f || t > 200.f)
      break;
  }
  return t;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = 0.0001f;
  const f32x2 k = f32x2(1, -1);
  return normalize(
      k.swizzle<0, 1, 1>() * f(__ctx, p + k.swizzle<0, 1, 1>() * h) +
      k.swizzle<1, 1, 0>() * f(__ctx, p + k.swizzle<1, 1, 0>() * h) +
      k.swizzle<1, 0, 1>() * f(__ctx, p + k.swizzle<1, 0, 1>() * h) +
      k.swizzle<0, 0, 0>() * f(__ctx, p + k.swizzle<0, 0, 0>() * h));
}
f32x3 render(Context &__ctx, f32x2 uv) {
  f32x3 lightPos = f32x3(5.f * cos(iTime), 5.f, 5.f * sin(iTime) + 3.f);
  f32x3 ro = f32x3(0, 2, -4);
  f32x3 rd = normalize(f32x3(uv, 2));
  f32x3 color = f32x3(0);
  f32x3 amb = f32x3(.1f);
  f32x3 diffusecol = f32x3(1.f, 1.f, 1.f);
  float t = rayMarch(__ctx, ro, rd);
  f32x3 p = ro + t * rd;
  f32x3 n = calcNormal(__ctx, p);
  f32x3 l = normalize(lightPos - p);
  f32x3 dif = clamp(dot(l, n), 0.f, 1.f) * diffusecol;
  float shadow = rayMarch(__ctx, p, l);
  if (shadow < length(lightPos - p))
    dif *= .1f;
  color = amb + dif;
  return color;
}
f32x2 getuv(Context &__ctx, f32x2 coord) {
  return (2.f * coord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 color = f32x3(0.f);
  for (int m = 0; m < 3; m++) {
    for (int n = 0; n < 3; n++) {
      f32x2 offset = 2.f * (f32x2(float(m), float(n)) / float(3) - .5f);
      f32x2 uv = getuv(__ctx, fragCoord + offset);
      color += render(__ctx, uv);
    }
  }
  color /= float(3 * 3);
  color = mix(color, f32x3(1), logo(__ctx, uv));
  fragColor = f32x4(color, 1.0f);
}
float logo(Context &__ctx, f32x2 uv) {
  float n = 0.f;
  n += smoothstep(abs(length(uv - f32x2(-1.23f, 0)) - .12f),
                  abs(length(uv - f32x2(-1.23f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n *= 1.f -
       smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -1.25f);
  n += smoothstep(abs(uv.data[1]), abs(uv.data[1]) + 3.f / iResolution.data[1],
                  .17f) *
       smoothstep(abs(uv.data[0] + 1.29f),
                  abs(uv.data[0] + 1.29f) + 3.f / iResolution.data[1], .05f);
  n += smoothstep(abs(length(uv - f32x2(-1.5f, 0)) - .12f),
                  abs(length(uv - f32x2(-1.5f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += smoothstep(abs(length(uv - f32x2(-0.9f, 0)) - .12f),
                  abs(length(uv - f32x2(-0.9f, 0)) - .12f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += (1.f +
        smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -.6f) -
        smoothstep(uv.data[1], uv.data[1] + 3.f / iResolution.data[1], .05f)) *
       smoothstep(abs(length(uv - f32x2(-0.6f, .03f)) - .09f),
                  abs(length(uv - f32x2(-0.6f, .03f)) - .09f) +
                      3.f / iResolution.data[1],
                  .05f);
  n += (1.f -
        smoothstep(uv.data[0], uv.data[0] + 3.f / iResolution.data[1], -.6f) +
        smoothstep(uv.data[1], uv.data[1] + 3.f / iResolution.data[1], -.05f)) *
       smoothstep(abs(length(uv - f32x2(-0.6f, -.04f)) - .09f),
                  abs(length(uv - f32x2(-0.6f, -.04f)) - .09f) +
                      3.f / iResolution.data[1],
                  .05f);
  return n;
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
