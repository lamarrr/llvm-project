// Port of https://www.shadertoy.com/view/styGzh; author: Plento.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/styGzh.json.
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
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), -sin(a), sin(a), cos(a));
}
float hash13(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * .1031f);
  p3 += dot(p3, p3.swizzle<2, 1, 0>() + 31.32f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float hash12(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * .1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float rbox(Context &__ctx, f32x3 p, f32x3 b, float r) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f) - r;
}
float sub(Context &__ctx, float d1, float d2) { return max(-d1, d2); }
f32x3 getCell(Context &__ctx, f32x3 p) { return floor(p / f32x3(1.2f)); }
f32x3 getCellCoord(Context &__ctx, f32x3 p) {
  return mod(p, f32x3(1.2f)) - f32x3(1.2f) * .5f;
}
float map(Context &__ctx, f32x3 p) {
  float d = 999.f;
  f32x3 id = getCell(__ctx, p);
  f32x3 rup = getCellCoord(__ctx, p);
  float rnd = 2.f * hash13(__ctx, id * 663.f) - 1.f;
  rup.data[1] += rnd * .2f;
  rup.set_swizzle<0, 2>(
      rup.swizzle<0, 2>() *
      (rot(__ctx, rnd * .3f + .3f * iTime * sign(rnd) + id.data[0])));
  rup.set_swizzle<0, 1>(
      rup.swizzle<0, 1>() *
      (rot(__ctx, rnd * .3f + .3f * iTime * sign(rnd) + id.data[1])));
  rup.data[0] += cos(rup.data[2] * 30.f) * .02f;
  rup.data[1] += sin(rup.data[2] * 30.f) * .02f;
  d = min(length(rup) - .45f, d);
  d = sub(__ctx,
          rbox(__ctx, p - f32x3(0.f, iTime * .55f, 0.f), f32x3(2.4f), .01f), d);
  return d;
}
f32x3 normal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(0.002f, -0.002f);
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>()) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>()) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>()) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>()));
}
f32x3 color(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 n, float t) {
  f32x3 p = ro + rd * t;
  f32x3 lp = ro + f32x3(0.f, 0.f, 2.5f);
  f32x3 ld = normalize(lp - p);
  float dd = length(p - lp);
  float dif = max(dot(n, ld), .1f);
  float fal = max(1.f / (dd), .0f);
  float spec = pow(max(dot(reflect(-ld, n), -rd), 0.f), 23.f);
  f32x3 objCol = f32x3(1);
  objCol += spec * .75f;
  objCol *= (dif + .4f);
  objCol *= fal;
  return objCol;
}
void mainImage(Context &__ctx, f32x4 &f, f32x2 u) {
  f32x2 uv = f32x2(u.swizzle<0, 1>() -
                   0.5f * iResolution.swizzle<0, 1>().swizzle<0, 1>()) /
             iResolution.swizzle<0, 1>().data[1];
  f32x3 rd = normalize(f32x3(uv, 1.5f));
  rd = normalize(f32x3(uv, 2.0f - dot(uv, uv) * -0.3f));
  f32x3 ro = f32x3(0);
  if (abs(uv.data[0]) > .75f) {
    f = f32x4(0, 0, 0, 1);
    return;
  }
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (rot(__ctx, .8f)));
  ro.data[1] += iTime * .55f;
  float d = 0.0f, t = 0.f, ns = 0.f;
  f32x3 p, n, col = f32x3(0);
  int nHits = 0;
  int maxHits = 3;
  for (int i = 0; i < 80; i++) {
    d = map(__ctx, ro + rd * t);
    if (nHits >= maxHits || t > 10.f)
      break;
    if (abs(d) < 0.004f) {
      p = ro + rd * t;
      n = normal(__ctx, p);
      if (d > 0.f && nHits == 0) {
        rd = refract(rd, n, 1.02f);
        f32x3 ref = reflect(n, rd);
        col += texture(__ctx.channel0, ref).swizzle<0, 1, 2>() * .4f;
        col.set_swizzle<2, 0, 1>(col.swizzle<2, 0, 1>() + (abs(ref) * .1f));
      }
      col += color(__ctx, ro, rd, n, t);
      nHits++;
      t += .2f;
    }
    t += abs(d) * .7f;
    ns++;
  }
  if (nHits > 0)
    col /= float(nHits) * .6f;
  col *= smoothstep(10.f, 4.f, ns * .1f);
  col = pow(col * 1.4f, f32x3(2.4f));
  col = 1.f - exp(-col);
  col += .12f * hash12(__ctx, u * 42.f + 32.f + iTime);
  f = f32x4(col, 1.0f);
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
