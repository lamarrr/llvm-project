// Port of https://www.shadertoy.com/view/ssSfzR; author: ErinZ.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssSfzR.json.
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
f32x2x2 rotate(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float smUni(Context &__ctx, float d1, float d2, float k) {
  float h = clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
  return mix(d2, d1, h) - k * h * (1.0f - h);
}
float f(Context &__ctx, f32x3 p) {
  p.set_swizzle<2, 0>(p.swizzle<2, 0>() * (rotate(__ctx, iTime)));
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotate(__ctx, iTime)));
  float k = .7f;
  float d1 = sdSphere(__ctx, p, f32x3(sin(iTime * .5f)), 1.3f);
  float d2 = sdSphere(__ctx, p, f32x3(0, 2.f * sin(iTime), 0.f), .5f);
  float d = smUni(__ctx, d1, d2, k);
  float d3 = sdSphere(__ctx, p,
                      f32x3(sin(iTime + 5.f), 0, 2.f * sin(iTime + 5.f)), .9f);
  d = smUni(__ctx, d, d3, k);
  float d4 = sdSphere(
      __ctx, p, f32x3(-cos(iTime + .3f), cos(iTime + .3f), cos(iTime + .3f)),
      .8f);
  d = smUni(__ctx, d, d4, k);
  float d5 = sdSphere(__ctx, p, f32x3(3.f * cos(iTime + .7f), 0.f, .5f), .3f);
  d = smUni(__ctx, d, d5, k);
  return d;
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.01f;
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
f32x3x3 setCamera(Context &__ctx, f32x3 camtar, f32x3 campos, float camro) {
  f32x3 z = normalize(camtar - campos);
  f32x3 cp = f32x3(sin(camro), cos(camro), 0.f);
  f32x3 x = normalize(cross(cp, z));
  f32x3 y = cross(z, x);
  return f32x3x3(x, y, z);
}
f32x3 render(Context &__ctx, f32x2 uv) {
  f32x3 lightPos = f32x3(-5.f, 5.f, -5);
  f32x3 cam_tar = f32x3(-1, 3, 2);
  f32x3 cam_pos = cam_tar + f32x3(-3, 8, 5);
  f32x3 rd = f32x3(uv, 9.f);
  rd = normalize(setCamera(__ctx, cam_tar, cam_pos, 0.f) * rd);
  float t = rayMarch(__ctx, cam_pos, rd);
  f32x3 color = f32x3(0);
  if (t > 200.f)
    return color;
  f32x3 p = cam_pos + t * rd;
  f32x3 n = calcNormal(__ctx, p);
  color = n * .5f + .5f;
  color *= exp(-0.0001f * t * t * t);
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
