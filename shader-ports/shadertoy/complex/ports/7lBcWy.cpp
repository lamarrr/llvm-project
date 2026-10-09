// Port of https://www.shadertoy.com/view/7lBcWy; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/7lBcWy.json.
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
f32x2 _min(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] < b.data[0])
    return a;
  return b;
}
float _cube(Context &__ctx, f32x3 p, f32x3 s) {
  f32x3 l = abs(p) - s;
  return max(l.data[0], max(l.data[1], l.data[2]));
}
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 acc = f32x2(10000.f, -1.f);
  acc = _min(__ctx, acc, f32x2(length(p + f32x3(0.f, 0.f, -15.f)) - 1.f, 0.f));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() *
                      (r2d(__ctx, .5f * sin(.1f * p.data[1]))));
  p.set_swizzle<0, 2>(
      p.swizzle<0, 2>() +
      (f32x2(sin(iTime), cos(iTime * .75f + p.data[1] * .1f)) * 5.f));
  float rad = 20.f;
  f32x3 pdart = p + f32x3(0.f, iTime * 50.f, 0.f);
  float adart = atan(pdart.data[2], pdart.data[0]);
  float stpdart = 3.14159265f * 2.f / 12.f;
  float sector = mod(adart + stpdart * .5f, stpdart) - stpdart * .5f;
  pdart.set_swizzle<0, 2>(f32x2(sin(sector), cos(sector)) *
                          length(pdart.swizzle<0, 2>()));
  float repyd = 15.f;
  float idda = floor((pdart.data[1] + repyd * .5f) / repyd);
  float rada = mix(10.f, 20.f, sin(idda) * .5f + .5f);
  pdart -= f32x3(0.f, 45.f, rada);
  pdart.data[1] = mod(pdart.data[1] + repyd * .5f, repyd) - repyd * .5f;
  float dart = _cube(__ctx, pdart, f32x2(.1f, 5.f).swizzle<0, 0, 1>());
  acc = _min(__ctx, acc,
             f32x2(length(pdart - f32x3(0.f, 0.f, -5.f)) - 0.25f, -1.f));
  acc = _min(__ctx, acc, f32x2(dart, 0.f));
  f32x3 pcube = p;
  float stpcube = 3.14159265f * 2.f / 40.f;
  float sectorcube = mod(adart + stpcube * .5f, stpcube) - stpcube * .5f;
  pcube.set_swizzle<0, 2>(f32x2(sin(sectorcube), cos(sectorcube)) *
                          length(pcube.swizzle<0, 2>()));
  pcube -= f32x3(0.f, 0.f, rad);
  float repyc = .5f;
  pcube.data[1] = mod(pcube.data[1] + repyc * .5f, repyc) - repyc * .5f;
  acc = _min(__ctx, acc, f32x2(_cube(__ctx, pcube, f32x3(.5f, .2f, 2.f)), 1.f));
  float tunnel = -(length(p.swizzle<0, 2>()) - rad);
  acc = _min(__ctx, acc, f32x2(tunnel, 0.f));
  return acc;
}
f32x3 getCam(Context &__ctx, f32x3 rd, f32x2 uv) {
  f32x3 r = normalize(cross(rd, f32x3(0.f, 1.f, 0.f)));
  f32x3 u = normalize(cross(rd, r));
  return normalize(rd + r * uv.data[0] + u * uv.data[1]);
}
f32x3 getNorm(Context &__ctx, f32x3 p, float d) {
  f32x2 e = f32x2(0.01f, 0.f);
  return normalize(f32x3(d) -
                   f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
f32x3 accLight;
f32x3 trace(Context &__ctx, f32x3 ro, f32x3 rd, int steps) {
  accLight = f32x3(0.f);
  f32x3 p = ro;
  for (int i = 0; i < steps; ++i) {
    f32x2 res = map(__ctx, p);
    if (res.data[0] < 0.01f)
      return f32x3(res.data[0], distance(p, ro), res.data[1]);
    if (res.data[1] < 0.f)
      accLight +=
          (f32x3(172, 38, 235) / 255.f) * 0.1f +
          f32x3(sin(distance(p, ro) * 1.f + iTime) * .5f + .5f, .5f, .1f) *
              (1.f - clamp(res.data[0] / 5.5f, 0.f, 1.f)) * .2f;
    p += rd * res.data[0] * .5f;
  }
  return f32x3(-1.f);
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x3 background = f32x3(212, 140, 32) / 255.f;
  f32x3 col = background;
  f32x3 ro = f32x3(0.f, -5.f, -5.f);
  f32x3 ta = f32x3(0.f, 25.f, 0.f);
  f32x3 rd = normalize(ta - ro);
  rd = getCam(__ctx, rd, uv);
  float depth = 150.f;
  f32x3 res = trace(__ctx, ro, rd, 128);
  if (res.data[1] > 0.f) {
    depth = res.data[1];
    f32x3 p = ro + rd * res.data[1];
    f32x3 n = getNorm(__ctx, p, res.data[0]);
    col = n * .5f + .5f;
    col = (f32x3(23, 24, 51) / 255.f) *
          clamp(dot(normalize(f32x3(n.data[0], -1.f, n.data[2])), n), 0.f, 1.f);
  }
  col += accLight;
  col = mix(col, f32x3(0.f),
            1.f - clamp(exp(-depth * depth * 0.0001f), 0.f, 1.f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  f32x3 col = rdr(__ctx, uv);
  col *= 1.f - clamp(length(uv), 0.f, 1.f);
  col *= f32x3(199, 242, 58) / 255.f;
  fragColor = f32x4(col, 1.f);
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
