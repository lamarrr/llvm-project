// Port of https://www.shadertoy.com/view/DlsyRj; author: BackwardsCap.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlsyRj.json.
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
  TextureCube channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x3 cam(Context &__ctx, f32x3 ro, f32x3 ta, f32x2 uv) {
  f32x3 cf = normalize(ta - ro);
  f32x3 cu = normalize(cross(cf, f32x3(0, 1, 0)));
  f32x3 cr = normalize(cross(cu, cf));
  return normalize(uv.data[0] * cu + uv.data[1] * cr + 2.f * cf);
}
float sphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float displacement(Context &__ctx, f32x3 p) {
  f32x2 d1 = p.swizzle<0, 1>() * 0.3f;
  f32x2 d2 = d1 * 0.2f;
  d1.data[0] += d1.data[1] * 2.f;
  d1.data[0] += iTime * 0.05f;
  d2.data[0] += iTime * 0.1f;
  d2.data[1] -= iTime * 0.025f;
  d2.data[0] += d2.data[1] * 2.f;
  float h1 = texture(__ctx.channel0, d2).data[0];
  float h2 = texture(__ctx.channel0, d1).data[0];
  return h1 * .6f + h2 * 0.1f * (1.0f - h1);
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 r;
  float d = displacement(__ctx, p);
  float warp = cos(p.data[1] * .3f * cos(iTime / 10.f + 1.56f));
  float column1 = sphere(__ctx, p * warp, 3.0f) + d;
  r = f32x2(column1, 1.0f);
  return r;
}
f32x2 ray(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.f, m;
  for (int i = 0; i < 128; i++) {
    f32x3 p = ro + rd * t;
    f32x2 s = map(__ctx, p);
    m = s.data[1];
    if (s.data[0] < 0.001f)
      break;
    t += s.data[0];
    if (t > 60.0f) {
      t = -1.f;
      break;
    }
  }
  return f32x2(t, m);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(0.01f, 0);
  return normalize(f32x3(map(__ctx, p + e.swizzle<0, 1, 1>()).data[0] -
                             map(__ctx, p - e.swizzle<0, 1, 1>()).data[0],
                         map(__ctx, p + e.swizzle<1, 0, 1>()).data[0] -
                             map(__ctx, p - e.swizzle<1, 0, 1>()).data[0],
                         map(__ctx, p + e.swizzle<1, 1, 0>()).data[0] -
                             map(__ctx, p - e.swizzle<1, 1, 0>()).data[0]));
}
float fresnel(Context &__ctx, f32x3 n, f32x3 dir, float power) {
  return pow(1.0f + dot(dir, n), power);
}
float shine(Context &__ctx, f32x3 p, f32x3 n) {
  p.data[1] = 0.f;
  float d = (dot(normalize(p), n));
  float r = 0.0f;
  if (d >= .99f)
    r += 0.2f * d;
  return r;
}
f32x3 down(Context &__ctx, f32x3 c, float s) {
  c.set_swizzle<0, 1>(c.swizzle<0, 1>() * (s));
  c.set_swizzle<0, 1>(floor(c.swizzle<0, 1>()));
  c.set_swizzle<0, 1>(c.swizzle<0, 1>() / (s));
  return c;
}
void mainImage(Context &__ctx, f32x4 &c, f32x2 f) {
  f32x2 uv = (2.f * f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(0.067f, 0.502f, 0.933f) *
              (((cos(uv.data[0]) + cos(uv.data[1])))) / 2.75f;
  col = down(__ctx, col, 60.f);
  f32x3 bg = col;
  float t = cos(iTime / 5.f) / 3.f;
  float d = 9.5f + (1.0f + cos(iTime / 2.f));
  f32x3 ro = f32x3(0, sin(t / .1f), cos(t) * d);
  f32x3 ta = f32x3(0);
  f32x3 rd = cam(__ctx, ro, ta, uv);
  f32x2 r = ray(__ctx, ro, rd);
  if (r.data[0] > 0.0f) {
    f32x3 p = ro + rd * r.data[0];
    f32x3 n = normal(__ctx, p);
    f32x3 mate = f32x3(.0f, .5f, 1.0f);
    f32x3 sun = normalize(f32x3(0.5f, .5f, .5f));
    f32x3 backlight = normalize(f32x3(-0.5f, -.5f, -.5f));
    float dif = clamp(dot(sun, n), 0.f, 1.f);
    float bdif = clamp(dot(backlight, n), 0.f, 1.f);
    float sky = clamp(dot(n, f32x3(0, 1, 0)), 0.f, 1.f);
    float bou = clamp(dot(n, f32x3(0, -1, 0)), 0.f, 1.f);
    float sha = clamp(ray(__ctx, p + n * .001f, sun).data[0], 0.f, 1.f);
    col = dif * mate;
    col += bdif * mate;
    col += sky * mate;
    col += bou * mate;
    col += fresnel(__ctx, n, rd, 3.f) * f32x3(0.6f, 0.9f, 1.0f);
    f32x3 ref = texture(__ctx.channel1, n).swizzle<0, 1, 2>();
    float s = shine(__ctx, p, n);
    col = mix(col, ref, 0.5f + s);
    col = mix(col, bg, fresnel(__ctx, n, rd, 2.f) * 1.5f);
    col = down(__ctx, col, 8.f);
    col = mix(col, bg, clamp(r.data[0] / 60.0f, 0.f, 1.f));
  }
  c.set_swizzle<0, 1, 2>(col);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_cube<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
