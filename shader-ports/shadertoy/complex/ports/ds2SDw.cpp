// Port of https://www.shadertoy.com/view/ds2SDw; author: darkeclipz.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ds2SDw.json.
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
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 rot(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
f32x4 map(Context &__ctx, f32x3 pos, float time) {
  float id = 0.f;
  float sphere = sdSphere(__ctx, pos, 0.5f);
  float nearestDistanceToSphere = 1e20f;
  float G = 0.192f;
  float F = G / dot(pos, pos);
  float plane = (pos.data[1] + F);
  float d = min(sphere, plane);
  nearestDistanceToSphere = min(nearestDistanceToSphere, sphere);
  if (plane < sphere) {
    id = 1.f;
  }
  return f32x4(d, id, F, nearestDistanceToSphere);
}
f32x3 calcNormal(Context &__ctx, f32x3 pos, float t) {
  f32x2 e = f32x2(0.001f, 0.0f);
  return normalize(
      f32x3(map(__ctx, pos + e.swizzle<0, 1, 1>(), t).data[0] -
                map(__ctx, pos - e.swizzle<0, 1, 1>(), t).data[0],
            map(__ctx, pos + e.swizzle<1, 0, 1>(), t).data[0] -
                map(__ctx, pos - e.swizzle<1, 0, 1>(), t).data[0],
            map(__ctx, pos + e.swizzle<1, 1, 0>(), t).data[0] -
                map(__ctx, pos - e.swizzle<1, 1, 0>(), t).data[0]));
}
float castShadow(Context &__ctx, f32x3 ro, f32x3 rd, float time) {
  float res = 1.0f;
  float t = 0.00f;
  for (int i = 0; i < 100; i++) {
    f32x3 pos = ro + t * rd;
    float h = map(__ctx, pos, time).data[0];
    res = min(res, 16.0f * h / t);
    if (res < 0.001f)
      break;
    t += h;
    if (t > 10.0f)
      break;
  }
  return clamp(res, 0.0f, 1.0f);
}
f32x3 castRay(Context &__ctx, f32x3 ro, f32x3 rd, float time) {
  float m = -1.0f;
  float t = 0.00f;
  int i = 0;
  float w = 1e20f;
  for (i = 0; i < 100; i++) {
    float precis = 0.001f;
    f32x3 pos = ro + t * rd;
    f32x4 h = map(__ctx, pos, time);
    m = h.data[1];
    w = min(w, h.data[3]);
    if (h.data[0] < precis)
      break;
    float stepSize = 1.0f / max(1.0f, 1.0f + h.data[2]);
    t += h.data[0] * stepSize;
    if (t > 20.0f)
      break;
  }
  if (t > 20.0f)
    m = -1.0f;
  return f32x3(t, m, w);
}
f32x3 rgb(Context &__ctx, int r, int g, int b) {
  return f32x3(r, g, b) / 255.f;
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 U) {
  float time = iTime / 4.f;
  f32x3 col = f32x3(0);
  f32x3 res = f32x3(0);
  for (float aax = 0.f; aax < 2.f; aax++)
    for (float aay = 0.f; aay < 2.f; aay++) {
      f32x2 p =
          (2.f * (U + f32x2(aax, aay) / 2.f) - iResolution.swizzle<0, 1>()) /
          iResolution.swizzle<0, 1>().data[1];
      f32x3 ta = f32x3(0, 0, 0);
      f32x3 ro = f32x3(0, 1.0f + (-sin(time) * .5f + .5f), -1);
      ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (rot(__ctx, time)));
      f32x3 ww = normalize(ta - ro);
      f32x3 uu = normalize(cross(ww, f32x3(0, 1, 0)));
      f32x3 vv = normalize(cross(uu, ww));
      f32x3 rd = normalize(p.data[0] * uu + p.data[1] * vv + 1.8f * ww);
      f32x3 col = f32x3(0.0f);
      f32x3 tm = castRay(__ctx, ro, rd, time);
      f32x3 starGlowColor = rgb(__ctx, 252, 120, 49);
      f32x3 starColor1 = rgb(__ctx, 254, 235, 164);
      f32x3 starColor2 = rgb(__ctx, 158, 40, 0);
      if (tm.data[0] < 20.f) {
        float t = tm.data[0];
        float id = tm.data[1];
        f32x3 pos = ro + t * rd;
        f32x3 nor = calcNormal(__ctx, pos, time);
        f32x3 mate = f32x3(0.6f);
        if (id < 0.5f) {
          mate = f32x3(.8f);
          f32x3 posn = normalize(pos);
          float u = 0.5f + atan(posn.data[2], posn.data[0]) / (2.f * 3.1415f);
          float v = 0.5f - asin(posn.data[1]) / 3.1415f;
          f32x2 uv = f32x2(u, v), df = fwidth(uv);
          if (df.data[0] > 0.5f)
            df.data[0] = 0.f;
          mate = 4.0f * starGlowColor * exp(-30.0f * tm.data[2]);
          float r = textureLod(__ctx.channel0, uv,
                               log2(max(df.data[0], df.data[1]) * 1024.f))
                        .data[0];
          mate *= mix(starColor1, starColor2, r);
        } else if (id < 1.5f) {
          mate = f32x3(0.02f);
          float s = 12.f;
          float w = 0.05f;
          if (fract(pos.data[0] * s) < w) {
            mate = f32x3(.3f);
          }
          if (fract(pos.data[2] * s) < w) {
            mate = f32x3(.3f);
          }
          mate += 1.5f * starGlowColor * exp(-8.0f * tm.data[2]);
          mate += 2.0f * starGlowColor * exp(-15.0f * tm.data[2]);
          mate += 0.5f * starGlowColor * exp(-30.0f * tm.data[2]);
          mate += 0.25f * starGlowColor * exp(-60.0f * tm.data[2]);
        }
        f32x3 light1 = f32x3(0, 1, 1);
        f32x3 light2 = f32x3(1, 3, 1);
        float key = clamp(1.0f, 0.0f, 1.0f);
        float bac = clamp(0.2f + 0.8f * dot(light2, nor), 0.0f, 1.0f);
        float amb = (0.7f + 0.3f * nor.data[1]);
        float shadow = 1.0f;
        f32x3 brdf = 1.0f * f32x3(0.40f, 0.40f, 0.40f) * amb * shadow;
        brdf += 1.0f * f32x3(1.00f, 1.00f, 1.00f) * key * shadow;
        brdf += 1.0f * f32x3(0.40f, 0.40f, 0.40f) * bac;
        col = mate * brdf * exp(-0.6f * t);
      }
      res += clamp(col, 0.0f, 1.0f);
    }
  col = pow(res / (2.f * 2.f), f32x3(0.4545f));
  f32x2 uv = U / iResolution.swizzle<0, 1>().swizzle<0, 1>();
  uv *= 1.0f - uv.swizzle<1, 0>();
  float v = pow(uv.data[0] * uv.data[1] * 10.0f, 0.25f);
  col *= v;
  O = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
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
