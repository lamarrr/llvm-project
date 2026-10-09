// Port of https://www.shadertoy.com/view/mtjGWw; author: shaofun.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mtjGWw.json.
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
f32x2x2 mm2(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float map(Context &__ctx, f32x3 p) {
  p.data[0] += sin(p.data[2] * 5.f + sin(p.data[1] * 5.f)) * 0.3f;
  return (length(p) - 1.f) * 0.7f;
}
float march(Context &__ctx, f32x3 ro, f32x3 rd) {
  float precis = 0.001f;
  float h = precis * 2.0f;
  float d = 0.f;
  for (int i = 0; i < 80; i++) {
    if (abs(h) < precis || d > 10.f)
      break;
    d += h;
    float res = map(__ctx, ro + rd * d);
    h = res;
  }
  return d;
}
f32x3 normal(Context &__ctx, const f32x3 p) {
  f32x2 e = f32x2(-1.f, 1.f) * 0.005f;
  return normalize(e.swizzle<1, 0, 0>() * map(__ctx, p + e.swizzle<1, 0, 0>()) +
                   e.swizzle<0, 0, 1>() * map(__ctx, p + e.swizzle<0, 0, 1>()) +
                   e.swizzle<0, 1, 0>() * map(__ctx, p + e.swizzle<0, 1, 0>()) +
                   e.swizzle<1, 1, 1>() * map(__ctx, p + e.swizzle<1, 1, 1>()));
}
void TBN(Context &__ctx, f32x3 N, f32x3 &T, f32x3 &B) {
  if (N.data[2] < -0.999999f) {
    T = f32x3(0, -1, 0);
    B = f32x3(-1, 0, 0);
  } else {
    float a = 1.0f / (1.0f + N.data[2]);
    float b = -N.data[0] * N.data[1] * a;
    T = f32x3(1.0f - N.data[0] * N.data[0] * a, b, -N.data[0]);
    B = f32x3(b, 1.0f - N.data[1] * N.data[1] * a, -N.data[1]);
  }
}
f32x3 dLine(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 a, f32x3 b) {
  f32x3 ba = b - a;
  f32x3 oa = ro - a;
  float oad = dot(oa, rd);
  float dba = dot(rd, ba);
  float baba = dot(ba, ba);
  float oaba = dot(oa, ba);
  f32x2 th =
      f32x2(-oad * baba + dba * oaba, oaba - oad * dba) / (baba - dba * dba);
  th.data[0] = max(th.data[0], 0.f);
  th.data[1] = clamp(th.data[1], 0.f, 1.f);
  f32x3 p = a + ba * th.data[1];
  f32x3 q = ro + rd * th.data[0];
  return f32x3(length(p - q), th);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 mo = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - .5f;
  mo = (mo == f32x2(-.5f)) ? mo = f32x2(0.8f, 0.1f) : mo;
  mo.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 ro = f32x3(0.f, 0.f, 4.5f);
  f32x3 rd = normalize(f32x3(p, -1.5f));
  f32x3 rd2 = f32x3(0, 0.f, -1);
  f32x2x2 mx = mm2(__ctx, mo.data[0] * 6.f);
  f32x2x2 my = mm2(__ctx, mo.data[1] * 6.f);
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (mx));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (mx));
  rd2.set_swizzle<0, 2>(rd2.swizzle<0, 2>() * (mx));
  ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() * (my));
  rd.set_swizzle<0, 1>(rd.swizzle<0, 1>() * (my));
  rd2.set_swizzle<0, 1>(rd2.swizzle<0, 1>() * (my));
  f32x3 ligt = normalize(f32x3(-.5f, 0.2f, -0.2f));
  f32x3 lcol = f32x3(1.00f, 0.90f, 0.75f);
  float rdl = clamp(dot(rd, ligt), 0.f, 1.f);
  f32x3 col = lcol * pow(rdl, 50.f) + f32x3(0.1f, 0.2f, 0.3f) * 0.5f;
  float rz = march(__ctx, ro, rd);
  if (rz < 10.f) {
    f32x3 pos = ro + rz * rd;
    f32x3 nor = normal(__ctx, pos);
    float dif = clamp(dot(nor, ligt), 0.f, 1.f);
    float bac = clamp(dot(nor, -ligt), 0.f, 1.f);
    float spe = pow(clamp(dot(reflect(rd, nor), ligt), 0.f, 1.f), 100.f);
    float fre = 0.6f * pow(clamp(1.f + dot(nor, rd), 0.0f, 1.0f), 2.f);
    f32x3 brdf = 1.0f * f32x3(0.10f, 0.11f, 0.13f);
    brdf += 2.f * bac * f32x3(0.15f, 0.15f, 0.15f);
    brdf += 1.50f * dif * lcol;
    col = f32x3(0.3f, 0.3f, 0.3f);
    col = col * brdf + col * spe + fre * col;
  }
  f32x3 pdir = f32x3(0.5773f);
  pdir.set_swizzle<0, 2>(pdir.swizzle<0, 2>() * (mm2(__ctx, iTime * 0.3f)));
  pdir.set_swizzle<2, 1>(pdir.swizzle<2, 1>() * (mm2(__ctx, iTime * .44f)));
  float rz2 = march(__ctx, pdir * 3.f, -pdir);
  f32x3 bpos = pdir * 3.f + rz2 * -pdir;
  f32x3 nor = normal(__ctx, bpos);
  f32x3 r = f32x3(0);
  f32x3 f = f32x3(0);
  for (int i = 0; i < 20000; i++) {
    TBN(__ctx, nor, f, r);
  }
  f32x3 g = dLine(__ctx, ro, rd, bpos, bpos + nor);
  float occ = step(g.data[1], rz) * 0.5f + 0.5f;
  col = max(col, smoothstep(-2.5f, 1.f, sin(g.data[2] * 120.f)) * occ *
                     f32x3(.3f, .3f, 1) *
                     (1.f - smoothstep(0.0f, .03f, g.data[0])));
  g = dLine(__ctx, ro, rd, bpos, bpos + f);
  occ = step(g.data[1], rz) * 0.5f + 0.5f;
  col = max(col, smoothstep(-2.5f, 1.f, sin(g.data[2] * 120.f)) * occ *
                     f32x3(1.f, .2f, .2f) *
                     (1.f - smoothstep(0.0f, .03f, g.data[0])));
  g = dLine(__ctx, ro, rd, bpos, bpos + r);
  occ = step(g.data[1], rz) * 0.5f + 0.5f;
  col = max(col, smoothstep(-2.5f, 1.f, sin(g.data[2] * 120.f)) * occ *
                     f32x3(.2f, 1, .2f) *
                     (1.f - smoothstep(0.0f, .03f, g.data[0])));
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
