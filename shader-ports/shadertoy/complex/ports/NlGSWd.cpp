// Port of https://www.shadertoy.com/view/NlGSWd; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/NlGSWd.json.
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
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
f32x2 _min(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] < b.data[0])
    return a;
  return b;
}
f32x2 map(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (r2d(__ctx, .25f)));
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (r2d(__ctx, .25f)));
  p -= f32x3(1.f, 1.2f, 0.f);
  f32x2 acc = f32x2(10000.f, -1.f);
  float shape = length(p) - 1.f;
  shape = max(shape, -(length(p.swizzle<0, 1>()) - .5f));
  shape = max(shape, -(length(p) - .8f));
  float mat = 0.0f;
  if (length(p) > .9f) {
    mat = 1.f;
    float th = 0.04f;
    if (abs(length(p.swizzle<0, 2>()) - .2f) - th < 0.f)
      mat = 3.f;
    if (abs(length(p.swizzle<1, 2>()) - .2f) - th < 0.f)
      mat = 3.f;
  }
  if (length(p.swizzle<0, 1>()) < .51f)
    mat = 2.f;
  acc = _min(__ctx, acc, f32x2(shape, mat));
  float antena = max(max(length(p.swizzle<0, 2>()) - .015f, p.data[1] + .5f),
                     -p.data[1] - 2.f);
  antena = min(antena, length(p - f32x3(0.f, -2.f, 0.f)) - .1f);
  acc = _min(__ctx, acc, f32x2(antena, 0.f));
  return acc;
}
float grass(Context &__ctx, f32x2 uv) {
  uv.data[0] += sin(uv.data[1] * 20.f + iTime) * .02f;
  float h = mix(.01f, .02f, clamp(sin(uv.data[0]) * .5f + .5f, 0.f, 1.f));
  return uv.data[1] -
         h * asin(sin(uv.data[0] * 250.f + sin(uv.data[1] * 2.f) * 20.f +
                      clamp(3.f + uv.data[1] * 15.f, 0.f, 1.f) * iTime));
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
f32x3 trace(Context &__ctx, f32x3 ro, f32x3 rd, int steps) {
  f32x3 p = ro;
  for (int i = 0; i < steps; ++i) {
    f32x2 res = map(__ctx, p);
    if (res.data[0] < 0.01f)
      return f32x3(res.data[0], distance(p, ro), res.data[1]);
    p += rd * res.data[0];
  }
  return f32x3(-1.f);
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x2 buv = uv;
  float baseT = iTime * .1f;
  float t = sin(baseT) * .5f;
  f32x3 col = mix(f32x3(0.694f, 0.827f, 0.824f), f32x3(0.796f, 0.882f, 0.871f),
                  clamp(-uv.data[1], 0.f, 1.f));
  float xA = uv.data[0] + t * .125f;
  float mountA =
      uv.data[1] + .1f - 0.05f * sin(xA * 5.f) + 0.01f * asin(sin(xA * 10.f));
  f32x3 mountACol = mix(
      f32x3(0.627f, 0.710f, 0.690f) * .8f, f32x3(0.702f, 0.784f, 0.765f),
      clamp(
          sin(14.6f * length(f32x2(xA, uv.data[1]) - f32x2(0.2f, 1.f)) * 10.f) *
              sin(length(f32x2(xA, uv.data[1]) - f32x2(0.f, 1.f)) * 50.f) *
              1000.f,
          0.f, 1.f));
  col = mix(col, mountACol, 1.f - clamp(mountA * 40000.f, 0.f, 1.f));
  float xB = uv.data[0] + t * .25f;
  float mountB =
      uv.data[1] + .1f - 0.02f * sin(xB * 10.f) + 0.01f * asin(sin(xB * 10.f));
  col = mix(col, f32x3(0.729f, 0.553f, 0.541f),
            1.f - clamp(mountB * 40000.f, 0.f, 1.f));
  f32x3 ro = f32x3(sin(t) * 10.f, 0.f, 10.f * cos(t));
  f32x3 ta = f32x3(t * 4.f, 0.f, 0.f);
  f32x3 rd = normalize(ta - ro);
  rd = getCam(__ctx, rd, buv);
  f32x3 res = trace(__ctx, ro, rd, 128);
  if (res.data[1] > 0.f) {
    f32x3 p = ro + rd * res.data[1];
    f32x3 n = getNorm(__ctx, p, res.data[0]);
    f32x3 ldir = normalize(f32x3(1.f, -1.f, 1.f));
    col = n * .5f + .5f;
    if (res.data[2] == 0.f)
      col = f32x3(.1f);
    if (res.data[2] == 1.f || res.data[2] == 3.f)
      col = mix(f32x3(.8f), f32x3(.92f), clamp(dot(n, ldir) * 100.f, 0.f, 1.f));
    if (res.data[2] == 2.f)
      col = f32x3(.4f);
    if (res.data[2] == 3.f)
      col *= .75f;
  }
  float xC = uv.data[0] + t * .5f;
  float mountC =
      uv.data[1] + .15f - 0.02f * sin(xC * 7.f) + 0.01f * asin(sin(xC * 10.f));
  col = mix(col, f32x3(0.824f, 0.392f, 0.388f),
            1.f - clamp(mountC * 40000.f, 0.f, 1.f));
  float xD = uv.data[0] + t;
  float mountD = uv.data[1] + .17f - 0.02f * sin(-xD * 10.f + 4.f) +
                 0.015f * asin(sin(xD * 7.f));
  mountD += .2f + grass(__ctx, uv);
  col = mix(col, f32x3(0.776f, 0.314f, 0.314f),
            1.f - clamp(mountD * 40000.f, 0.f, 1.f));
  float xE = uv.data[0] + t * 2.f;
  float mountE = uv.data[1] + .3f - 0.04f * sin(-xE * 10.f + 4.f) +
                 0.015f * asin(sin(xE * 7.f));
  mountE += .2f + grass(__ctx, uv);
  col = mix(col, f32x3(0.541f, 0.173f, 0.173f),
            1.f - clamp(mountE * 40000.f, 0.f, 1.f));
  uv.data[1] -= .2f;
  float xCloud = uv.data[0] + t * .0125f;
  float cloud =
      max(uv.data[1] - asin(sin(xCloud * 5.f)) * .02f -
              sin(xCloud * 5.f - iTime * .25f) * .03f,
          -(uv.data[1] + .03f - .02f * (sin(xCloud * 5.f + iTime * .25f))));
  col = mix(col, f32x3(0.776f, 0.875f, 0.863f),
            1.f - clamp(cloud * 40000.f, 0.f, 1.f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  float stp = .005f;
  uv = floor(uv / stp) * stp;
  f32x3 col = rdr(__ctx, uv);
  fragColor = f32x4(col, 1.0f);
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
