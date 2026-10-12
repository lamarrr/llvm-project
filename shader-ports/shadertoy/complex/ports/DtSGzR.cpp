// Port of https://www.shadertoy.com/view/DtSGzR; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtSGzR.json.
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
f32x2x2 rotMat(Context &__ctx, float r) {
  return f32x2x2(cos(r), -sin(r), sin(r), cos(r));
}
f32x3 rotX(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rotY(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rotZ(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rotMat(__ctx, r)));
  return p;
}
float hash11(Context &__ctx, float p) {
  p = fract(p * .1031f);
  p *= p + 33.33f;
  p *= p + p;
  return fract(p);
}
f32x2 hash12(Context &__ctx, float p) {
  f32x3 p3 = fract(f32x3(p, p, p) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
f32x2 hash22(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * f32x3(.1031f, .1030f, .0973f));
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.swizzle<0, 0>() + p3.swizzle<1, 2>()) * p3.swizzle<2, 1>());
}
float hash21(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * .1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float cosmooth(Context &__ctx, float x) {
  return 1.f - cos(3.141593f * x) * 0.5f - 0.5f;
}
float shash(Context &__ctx, float x) {
  float fx = floor(x);
  float s = mod(fx, 2.f) * 2.f - 1.f;
  float a = hash11(__ctx, fx) * s;
  float b = hash11(__ctx, fx + 1.f) * -s;
  return mix(a, b, cosmooth(__ctx, fract(x))) * 0.5f;
}
f32x3 planeIntersect(Context &__ctx, f32x3 ro, f32x3 rd, float y) {
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() +
                       rd.swizzle<0, 2>() * (y - ro.data[1]) / rd.data[1]);
  return f32x3(ro.data[0], y, ro.data[2]);
}
f32x3 getEnvironmentMap(Context &__ctx, f32x3 rd, f32x3 l) {
  float ld = dot(rd, l);
  return (1.f / (max(1e-10f, 1.f - ld + 0.003f)) * 0.01f +
          0.02f / (abs(ld - 0.95f) + 0.07f)) *
             f32x3(0.733f, 0.302f, 0.769f) +
         exp(-rd.data[1] * 3.f) * f32x3(0.125f, 0.075f, 0.110f);
}
float getHeight(Context &__ctx, f32x2 p) {
  float r = 0.f;
  for (float i = 0.f; i < 3.f; i++) {
    float e = exp2(-i);
    r += abs(sin(p.data[0] / e + iTime)) * e;
    p = rotZ(__ctx, p.swizzle<0, 1, 0>(), hash11(__ctx, i * 25.f) * 0.2f)
            .swizzle<0, 1>();
  }
  return r;
}
f32x2 heightNormal(Context &__ctx, f32x2 p, float dist) {
  f32x2 e = f32x2(0.2f, 0);
  return normalize(f32x2(getHeight(__ctx, p + e.swizzle<0, 1>()) -
                             getHeight(__ctx, p - e.swizzle<0, 1>()),
                         getHeight(__ctx, p + e.swizzle<1, 0>()) -
                             getHeight(__ctx, p - e.swizzle<1, 0>())));
}
f32x3 gradToVec(Context &__ctx, f32x2 d) {
  f32x3 a = normalize(f32x3(1, -d.data[0], 0)),
        b = normalize(f32x3(0, -d.data[1], 1));
  return cross(a, b);
}
f32x3 halfVec(Context &__ctx, f32x3 a, f32x3 b) { return normalize(a + b); }
void _mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 nuv = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * 0.5f) / iResolution.data[1];
  f32x2 m = (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * 0.5f) /
            iResolution.data[1] * step(0.01f, iMouse.data[2]);
  f32x2 rot = f32x2(3.14f * -0.5f, 0) - m.swizzle<0, 1>() * 4.f -
              f32x2(shash(__ctx, iTime * 0.25f),
                    shash(__ctx, iTime * 0.25f + 0.7071067812f)) *
                  0.5f;
  f32x3 ro = f32x3(0, 1, 0);
  f32x3 rd = rotY(__ctx, rotX(__ctx, normalize(f32x3(uv, 0.6f)), rot.data[1]),
                  rot.data[0]);
  f32x3 center =
      rotY(__ctx, rotX(__ctx, normalize(f32x3(0, 0, 0.6f)), rot.data[1]),
           rot.data[0]);
  ;
  f32x3 lgt = rotZ(__ctx, f32x3(1, 0, 0), 0.2f);
  f32x3 pli = planeIntersect(__ctx, ro, rd, 0.f);
  float blend = smoothstep(-0.05f, 0.f, rd.data[1]);
  f32x3 planeNormal = gradToVec(
      __ctx,
      0.2f *
          heightNormal(__ctx,
                       clamp(pli.swizzle<0, 2>(), f32x2(-1e4f), f32x2(1e4f)),
                       1.f) /
          max(1.f, distance(ro, pli) * 0.5f));
  f32x3 env =
      getEnvironmentMap(__ctx, mix(reflect(rd, planeNormal), rd, blend), lgt);
  float vignette = 4.f * nuv.data[0] * (1.f - nuv.data[0]) * 4.f * nuv.data[1] *
                       (1.f - nuv.data[1]) * 0.6f +
                   0.4f;
  float glow = pow(max(0.f, dot(rd, halfVec(__ctx, center, lgt))), 12.f) *
               0.3f * mix(0.2f, 1.5f, pow(max(0.f, dot(center, lgt)), 25.f));
  fragColor = f32x4(sqrt((env + glow) * vignette), 1.0f);
}
f32x4 simpleAA(Context &__ctx, f32x2 fc, f32x4 m) {
  f32x4 res = f32x4(0);
  float st = 2.0f / 16.f;
  for (f32x2 o = f32x2(-1.f); o.data[0] < 1.f; o.data[0] += st)
    for (o.data[1] = -1.f; o.data[1] < 1.f; o.data[1] += st) {
      f32x4 color;
      _mainImage(__ctx, color, fc + o);
      res += color;
    }
  return res / (16.f * 16.f);
}
f32x4 stochasticAA(Context &__ctx, f32x2 fc, f32x4 m) {
  f32x4 res = f32x4(0);
  float st = 1.0f / (16.f - 1.f);
  f32x4 color;
  _mainImage(__ctx, color, fc);
  res += color;
  if (16.f > 1.f) {
    for (float x = 0.f; x < 1.f; x += st) {
      f32x4 color;
      _mainImage(__ctx, color,
                 fc + (hash22(__ctx, (x + iTime + fc) * 25.f) - 0.5f));
      res += color;
    }
  }
  return res / 16.f;
}
f32x3 dither(Context &__ctx, f32x3 color, f32x2 coord, float steps) {
  f32x3 reduce = floor(color * steps) / steps;
  f32x3 error = color - reduce;
  return reduce + step(f32x3(hash21(__ctx, coord)), error * steps) / steps;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = f32x4(
      dither(__ctx, stochasticAA(__ctx, fragCoord, iMouse).swizzle<0, 1, 2>(),
             fragCoord, 256.f),
      1);
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
