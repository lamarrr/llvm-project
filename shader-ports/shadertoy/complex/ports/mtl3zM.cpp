// Port of https://www.shadertoy.com/view/mtl3zM; author: janaTox.
// Dataset license metadata: mit; original retained in upstream/mtl3zM.json.
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
struct Context {
  TextureCube channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
float GetDist(Context &__ctx, f32x3 p) {
  float d = sdBox(__ctx, p, f32x3(1));
  float c = cos(3.141592f / 5.f), s = sqrt(.75f - c * c);
  f32x3 n = f32x3(-.5f, -c, s);
  p = abs(p);
  p -= 2.f * min(0.f, dot(p, n)) * n;
  p.set_swizzle<0, 1>(abs(p.swizzle<0, 1>()));
  p -= 2.f * min(0.f, dot(p, n)) * n;
  p.set_swizzle<0, 1>(abs(p.swizzle<0, 1>()));
  p -= 2.f * min(0.f, dot(p, n)) * n;
  d = p.data[2] - 1.f;
  return d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd, float side) {
  float dO = 1.f;
  for (int i = 0; i < 100; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p) * side;
    dO += dS;
    if (dO > 100.f || abs(dS) < .001f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(.01f, 0);
  f32x3 n = GetDist(__ctx, p) - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u;
  return normalize(i);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 3, -3);
  ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() *
                       (Rot(__ctx, -m.data[1] * 3.141592f + 1.f)));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() *
                       (Rot(__ctx, -m.data[0] * 6.283185f)));
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0, 0.f, 0), 3.5f);
  f32x3 col = texture(__ctx.channel0, rd).swizzle<0, 1, 2>();
  float d = RayMarch(__ctx, ro, rd, 1.f);
  float IOR = 1.45f;
  if (d < 100.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    f32x3 r = reflect(rd, n);
    f32x3 refOutside = texture(__ctx.channel0, r).swizzle<0, 1, 2>();
    f32x3 rdIn = refract(rd, n, 1.f / IOR);
    f32x3 pEnter = p - n * .001f * 3.f;
    float dIn = RayMarch(__ctx, pEnter, rdIn, -1.f);
    f32x3 pExit = pEnter + rdIn * dIn;
    f32x3 nExit = GetNormal(__ctx, pExit);
    f32x3 reflTex = f32x3(0.f);
    f32x3 rdOut = f32x3(0.f);
    float abb = .01f;
    rdOut = refract(rdIn, nExit, IOR - abb);
    if (dot(rdOut, rdOut) == 0.f)
      rdOut = reflect(rdIn, nExit);
    reflTex.data[0] = texture(__ctx.channel0, rdOut).data[0];
    rdOut = refract(rdIn, nExit, IOR);
    if (dot(rdOut, rdOut) == 0.f)
      rdOut = reflect(rdIn, nExit);
    reflTex.data[1] = texture(__ctx.channel0, rdOut).data[1];
    rdOut = refract(rdIn, nExit, IOR + abb);
    if (dot(rdOut, rdOut) == 0.f)
      rdOut = reflect(rdIn, nExit);
    reflTex.data[2] = texture(__ctx.channel0, rdOut).data[2];
    float dens = .2f;
    float optDist = exp(-dIn * dens);
    col = reflTex * optDist * f32x3(.8f);
    float fresnel = pow(1.f + dot(rd, n), 5.f);
    col = mix(reflTex, refOutside, fresnel);
  }
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
