// Port of https://www.shadertoy.com/view/mtG3zm; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/mtG3zm.json.
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
f32x2x2 r2d(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
f32x3 getCam(Context &__ctx, f32x3 rd, f32x2 uv) {
  f32x3 r = normalize(cross(rd, f32x3(0.f, 1.f, 0.f)));
  f32x3 u = normalize(cross(rd, r));
  return normalize(rd + (r * uv.data[0] + u * uv.data[1]) * .6f);
}
float sdCapsule(Context &__ctx, f32x3 p, f32x3 a, f32x3 b, float r) {
  f32x3 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h) - r;
}
f32x2 _min(Context &__ctx, f32x2 a, f32x2 b) {
  if (a.data[0] < b.data[0])
    return a;
  return b;
}
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 acc = f32x2(1000.f, -1.f);
  float repx = .5f;
  float idx = floor((p.data[0] + repx * .5f) / repx);
  p.data[0] = mod(p.data[0] + repx * .5f, repx) - repx * .5f;
  float repy = 5.f;
  p.data[1] = mod(p.data[1] + repy * .5f, repy) - repy * .5f;
  p.set_swizzle<1, 2>(
      p.swizzle<1, 2>() *
      (r2d(__ctx, length(p.swizzle<1, 2>()) * .5f - iTime * .1f + idx * 5.f -
                      .4f * iTime * abs(sin(idx + .5f)))));
  float an = atan(p.data[1], p.data[2]);
  float rep = acos(-1.f) * 2.f / 12.f;
  float id = floor((an + rep * .5f) / rep);
  p.data[0] += sin(p.data[1] * 20.f - iTime * 4.f) * .02f *
               clamp(length(p.swizzle<1, 2>()), 0.f, 1.f);
  float sector = mod(an + rep * .5f, rep) - rep * .5f;
  p.set_swizzle<1, 2>(f32x2(sin(sector), cos(sector)) *
                      length(p.swizzle<1, 2>()));
  float blob =
      sdCapsule(__ctx, p, f32x3(0.f, 0.f, -0.5f), f32x3(0.f, 0.f, 1.5f), .12f);
  p.data[2] -= 1.5f;
  f32x3 pm = p - f32x3(0.f, 0.f, -.1f);
  f32x3 p2 = p;
  p2.data[0] = abs(p2.data[0]);
  p2.set_swizzle<0, 2>(p2.swizzle<0, 2>() * (r2d(__ctx, .5f)));
  blob = min(blob, length(p2.swizzle<0, 1>()) - .002f);
  blob = max(blob, p.data[2] - .2f);
  f32x3 p3 = p;
  p3.data[0] = abs(p3.data[0]);
  blob = min(blob, length(p3 - f32x3(0.1f, 0.f, .2f)) - .02f);
  p.data[0] = abs(p.data[0]);
  float eyes = length(p.swizzle<0, 2>() - f32x2(.03f, 0.f)) - .02f;
  float mat = clamp(eyes * 400.f, 0.f, 1.f);
  float mouth = length(pm.swizzle<0, 2>()) - .05f;
  mouth = max(mouth, -pm.data[2]);
  mat = mix(mat, 2.f, 1.f - clamp(mouth * 400.f, 0.f, 1.f));
  pm.data[0] += sin(pm.data[2] * 150.f - 7.f * iTime) * .01f;
  float tongue =
      sdSegment(__ctx, pm.swizzle<0, 2>(), f32x2(0.f, 0.f), f32x2(0.f, .025f)) -
      .01f;
  tongue = max(tongue, mouth);
  mat = mix(mat, 3.f, 1.f - clamp(tongue * 4000.f, 0.f, 1.f));
  acc = _min(__ctx, acc, f32x2(blob, mat));
  return acc;
}
f32x3 accCol;
f32x3 trace(Context &__ctx, f32x3 ro, f32x3 rd) {
  accCol = f32x3(0.f);
  f32x3 p = ro;
  for (int i = 0; i < 128 && distance(p, ro) < 10.f; ++i) {
    f32x2 res = map(__ctx, p);
    if (res.data[0] < 0.01f)
      return f32x3(res.data[0], distance(p, ro), res.data[1]);
    p += rd * res.data[0] * .3f;
    f32x3 rgb =
        mix(f32x3(0.996f, 0.290f, 0.165f), f32x3(0.996f, 0.651f, 0.165f),
            sin(iTime + 10.f * p.data[0] + p.data[1]) * .5f + .5f);
    accCol += rgb * (1.f - clamp(res.data[0] / .2f, 0.f, 1.f)) * .025f *
              (1.f + texture(__ctx.channel0, f32x2(0.1f, 0.f)).data[0]);
  }
  return f32x3(-1.f);
}
f32x3 getNorm(Context &__ctx, f32x3 p, float d) {
  f32x2 e = f32x2(0.01f, 0.f);
  return normalize(f32x3(d) -
                   f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.f);
  uv.data[0] += .3f;
  f32x3 ro = f32x3(.5f, 0.5f, -2.f);
  f32x3 ta = f32x3(0.f, -3.2f, 0.f);
  f32x3 rd = normalize(ta - ro);
  rd = getCam(__ctx, rd, uv);
  f32x3 res = trace(__ctx, ro, rd);
  if (res.data[1] > 0.f) {
    f32x3 p = ro + rd * res.data[1];
    f32x3 n = getNorm(__ctx, p, res.data[0]);
    col = n * .5f + .5f;
    col = f32x3(.1f);
    if (res.data[2] == 0.f)
      col = f32x3(1.f);
    if (res.data[2] == 2.f)
      col = f32x3(.7f, 0.1f, 0.f);
    if (res.data[2] == 3.f)
      col = f32x3(1.f, 0.3f, 0.3f);
  }
  col += accCol;
  float beat = 1.f / 2.2f;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  f32x3 col = rdr(__ctx, uv);
  col *= 1.f - clamp((length(uv) - .2f) * 2.f, 0.f, 1.f);
  fragColor = f32x4(col, 1.0f);
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
