// Port of https://www.shadertoy.com/view/stscWj; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/stscWj.json.
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
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
float hash21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a, f32x2(21.23f, 41.232f))) * 4123.2323f);
}
float noise(Context &__ctx, f32x2 p) {
  f32x2 i = floor(p);
  f32x2 f = fract(p);
  f32x2 u = f * f * (3.f - 2.f * f);
  return mix(mix(hash21(__ctx, i + f32x2(0, 0)), hash21(__ctx, i + f32x2(1, 0)),
                 u.data[0]),
             mix(hash21(__ctx, i + f32x2(0, 1)), hash21(__ctx, i + f32x2(1, 1)),
                 u.data[0]),
             u.data[1]);
}
float box(Context &__ctx, f32x3 p, f32x3 d) {
  p = abs(p) - d;
  return length(max(p, 0.f)) +
         min(0.f, max(max(p.data[0], p.data[1]), p.data[2]));
}
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float height_map(Context &__ctx, f32x2 p) {
  float height = noise(__ctx, p * .5f) * 4.0f;
  height = floor(height / .25f) * .25f;
  return height;
}
float time, clr;
f32x2x2 rx, ry;
f32x2 map(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() + (iTime * .5f));
  f32x2 res = f32x2(100.f, -1.f);
  float height = height_map(__ctx, p.swizzle<0, 2>());
  clr = height;
  float d = (p.data[1] + 3.f) - height;
  if (d < res.data[0])
    res = f32x2(d * .5f, 2.f);
  return res;
}
f32x3 normal(Context &__ctx, f32x3 p, float t, float md) {
  t *= md;
  float d = map(__ctx, p).data[0];
  f32x2 e = f32x2(t, 0);
  f32x3 n = d - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                      map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]);
  return normalize(n);
}
f32x3 shade(Context &__ctx, f32x3 p, f32x3 rd, float d, float m) {
  f32x3 n = normal(__ctx, p, d, 1.f);
  f32x3 lpos = f32x3(-6, 10, 6);
  clr -= .5f;
  f32x3 c = f32x3(1.f - (clr * .5f), 0, (clr * .6f));
  f32x3 l = normalize(lpos - p);
  float shdw = 1.0f;
  for (float t = .01f; t < 14.f;) {
    float h = map(__ctx, p + l * t).data[0];
    if (h < .0001f) {
      shdw = 0.f;
      break;
    }
    shdw = min(shdw, 24.f * h / t);
    t += h * .95f;
    if (shdw < .0001f || t > 42.f)
      break;
  }
  return mix(c, c * shdw, .75f);
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  f32x2 uv = (-iResolution.swizzle<0, 1>() + 2.0f * F) /
             max(iResolution.data[0], iResolution.data[1]);
  f32x3 C = f32x3(.0f);
  float z = iMouse.swizzle<0, 1>() == f32x2(0)
                ? 0.f
                : -(iMouse.data[1] / iResolution.data[1] * 6.f - 3.f) *
                      3.14159265358f;
  float y = iMouse.swizzle<0, 1>() == f32x2(0)
                ? 0.f
                : -(iMouse.data[0] / iResolution.data[0] * 1.f - .5f) *
                      3.14159265358f;
  float zoom = 10.f + z;
  f32x3 ro = f32x3(uv * zoom, -zoom);
  f32x3 rd = f32x3(0.f, 0.f, 1.f);
  if (uv.data[1] < .4f && uv.data[1] > -.4f) {
    rx = rot(__ctx, -.785f);
    ry = rot(__ctx, -.785f + y);
    ro.set_swizzle<1, 2>(ro.swizzle<1, 2>() * (rx));
    ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (ry));
    rd.set_swizzle<1, 2>(rd.swizzle<1, 2>() * (rx));
    rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (ry));
    f32x3 p = ro;
    for (int i = 0; i < 125; i++) {
      f32x2 ray = map(__ctx, p);
      float d = i < 32 ? ray.data[0] * .5f : ray.data[0];
      float m = ray.data[1];
      p += rd * d;
      if (d * d < 1e-8f) {
        C = shade(__ctx, p, rd, d, ray.data[1]);
        break;
      }
      if (distance(p, rd) > 75.f) {
        break;
      }
    }
  } else {
    uv.data[0] += uv.data[1] > .0f ? iTime * .5f : -iTime * .5f;
    float px = fwidth(uv.data[0]);
    f32x2 f = fract(uv * 55.f) - .5f;
    f32x2 fid = floor(uv * 55.f) + .5f;
    float ht = noise(__ctx, fid + iTime * .25f);
    f32x3 ledh = f32x3(1, 0, 0);
    f32x3 ledc = f32x3(0, 0, 1);
    ht *= .85f;
    float ff = box(__ctx, f, f32x2(.3f, .3f)) - .075f;
    ff = smoothstep(.075f + px, -px, ff);
    float avg = (fid.data[1] * .1f) + 3.f;
    float avx = (fid.data[1] * .1f) - 2.3f;
    if (ht > avg && ht < avg + .1f)
      C = mix(C, ledh, ff);
    if (ht > avx && ht < avx + .1f)
      C = mix(C, ledh, ff);
    if (ht > avg + .25f && ht < avg + 1.1f)
      C = mix(C, ledc, ff);
    if (ht > avx + .25f && ht < avx + 1.1f)
      C = mix(C, ledc, ff);
  }
  C = pow(C, f32x3(0.4545f));
  O = f32x4(C, 1.0f);
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
