// Port of https://www.shadertoy.com/view/fdGfWR; author: soundmasteraj.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/fdGfWR.json.
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
f32x2x2 mm2(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
float noise(Context &__ctx, float x) {
  return textureLod(__ctx.channel0, f32x2(x * .01f, 1.f), 0.0f).data[0];
}
float hash(Context &__ctx, float n) { return fract(sin(n) * 43758.5453f); }
float noise(Context &__ctx, f32x3 p) {
  f32x3 ip = floor(p);
  f32x3 fp = fract(p);
  fp = fp * fp * (3.0f - 2.0f * fp);
  f32x2 tap = (ip.swizzle<0, 1>() + f32x2(37.0f, 17.0f) * ip.data[2]) +
              fp.swizzle<0, 1>();
  f32x2 rg =
      textureLod(__ctx.channel0, (tap + 0.5f) / 256.0f, 0.0f).swizzle<1, 0>();
  return mix(rg.data[0], rg.data[1], fp.data[2]);
}
f32x3x3 m3 =
    f32x3x3(0.00f, 0.80f, 0.60f, -0.80f, 0.36f, -0.48f, -0.60f, -0.48f, 0.64f);
float flow(Context &__ctx, f32x3 p, float t) {
  float z = 2.f;
  float rz = 0.f;
  f32x3 bp = p;
  for (float i = 1.f; i < 5.f; i++) {
    p += iTime * 1.1f * .1f;
    rz += (sin(noise(__ctx, p + t * 0.8f) * 6.f) * 0.5f + 0.5f) / z;
    p = mix(bp, p, 0.6f);
    z *= 2.f;
    p *= 2.01f;
    p *= m3;
  }
  return rz;
}
float sins(Context &__ctx, float x) {
  float rz = 0.f;
  float z = 2.f;
  for (float i = 0.f; i < 3.f; i++) {
    rz += abs(fract(x * 1.4f) - 0.5f) / z;
    x *= 1.3f;
    z *= 1.15f;
    x -= (sin(iTime / 3.1415f)) + iTime * 1.1f * .65f * z;
  }
  return rz;
}
float segm(Context &__ctx, f32x3 p, f32x3 a, f32x3 b) {
  f32x3 pa = p - a;
  f32x3 ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.f);
  return length(pa - ba * h) * .5f;
}
f32x3 path(Context &__ctx, float i, float d) {
  f32x3 en = f32x3(0.f, 0.f, 1.f);
  float sns2 = sins(__ctx, d + i * 0.5f) * 0.22f;
  float sns = sins(__ctx, d + i * .6f) * 0.21f;
  en.set_swizzle<0, 2>(
      en.swizzle<0, 2>() *
      (mm2(__ctx, (hash(__ctx, i * 10.569f) - .5f) * 6.2f + sns2)));
  en.set_swizzle<0, 1>(
      en.swizzle<0, 1>() *
      (mm2(__ctx, (hash(__ctx, i * 4.732f) - .5f) * 6.2f + sns)));
  return en;
}
f32x2 map(Context &__ctx, f32x3 p, float i) {
  float lp = length(p);
  f32x3 bg = f32x3(0.f);
  f32x3 en = path(__ctx, i, lp);
  float ins = smoothstep(0.11f, .46f, lp);
  float outs = .15f + smoothstep(.0f, .15f, abs(lp - 1.f));
  p *= ins * outs;
  float id = ins * outs;
  float rz = segm(__ctx, p, bg, en) - 0.011f;
  return f32x2(rz, id);
}
float march(Context &__ctx, f32x3 ro, f32x3 rd, float startf, float maxd,
            float j) {
  float precis = 0.001f;
  float h = 0.5f;
  float d = startf;
  for (int i = 0; i < 35; i++) {
    if (abs(h) < precis || d > maxd)
      break;
    d += h * 1.2f;
    float res = map(__ctx, ro + rd * d, j).data[0];
    h = res;
  }
  return d;
}
f32x3 vmarch(Context &__ctx, f32x3 ro, f32x3 rd, float j, f32x3 orig) {
  f32x3 p = ro;
  f32x2 r = f32x2(0.f);
  f32x3 sum = f32x3(0);
  float w = 0.f;
  for (int i = 0; i < 19; i++) {
    r = map(__ctx, p, j);
    p += rd * .03f;
    float lp = length(p);
    f32x3 col =
        sin(f32x3(.05f, 0.5f, 1.052f) * 3.94f + r.data[0]) * .85f + 0.4f;
    col.set_swizzle<0, 1, 2>(col.swizzle<0, 1, 2>() *
                             (smoothstep(.0f, .015f, -r.data[0])));
    col *= smoothstep(0.04f, .2f, abs(lp - 1.1f));
    col *= smoothstep(0.1f, .34f, lp);
    sum +=
        abs(col) * 5.f *
        (1.2f - noise(__ctx, lp * 2.f + j * 13.f + iTime * 1.1f * 5.f) * 1.1f) /
        (log(distance(p, orig) - 2.f) + .75f);
  }
  return sum;
}
f32x2 iSphere2(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 oc = ro;
  float b = dot(oc, rd);
  float c = dot(oc, oc) - 1.f;
  float h = b * b - c;
  if (h < 0.0f)
    return f32x2(-1.f);
  else
    return f32x2((-b - sqrt(h)), (-b + sqrt(h)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - 0.5f;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x2 um = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - .5f;
  f32x3 ro = f32x3(0.f, 0.f, 5.f);
  f32x3 rd = normalize(f32x3(p * .7f, -1.5f));
  f32x2x2 mx = mm2(__ctx, iTime * 1.1f * .4f + um.data[0] * 6.f);
  f32x2x2 my = mm2(__ctx, iTime * 1.1f * 0.3f + um.data[1] * 6.f);
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (mx));
  rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (mx));
  ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() * (my));
  rd.set_swizzle<0, 1>(rd.swizzle<0, 1>() * (my));
  f32x3 bro = ro;
  f32x3 brd = rd;
  f32x3 col = f32x3(0.0125f, 0.f, 0.025f);
  for (float j = 1.f; j < 13.f + 1.f; j++) {
    ro = bro;
    rd = brd;
    f32x2x2 mm =
        mm2(__ctx, (iTime * 1.1f * 0.1f + ((j + 1.f) * 5.1f)) * j * 0.25f);
    ro.set_swizzle<0, 1>(ro.swizzle<0, 1>() * (mm));
    rd.set_swizzle<0, 1>(rd.swizzle<0, 1>() * (mm));
    ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (mm));
    rd.set_swizzle<0, 2>(rd.swizzle<0, 2>() * (mm));
    float rz = march(__ctx, ro, rd, 2.5f, 6.f, j);
    if (rz >= 6.f)
      continue;
    f32x3 pos = ro + rz * rd;
    col = max(col, vmarch(__ctx, pos, rd, j, bro));
  }
  ro = bro;
  rd = brd;
  f32x2 sph = iSphere2(__ctx, ro, rd);
  if (sph.data[0] > 0.f) {
    f32x3 pos = ro + rd * sph.data[0];
    f32x3 pos2 = ro + rd * sph.data[1];
    f32x3 rf = reflect(rd, pos);
    f32x3 rf2 = reflect(rd, pos2);
    float nz = (-log(abs(flow(__ctx, rf * 1.2f, iTime * 1.1f) - .01f)));
    float nz2 = (-log(abs(flow(__ctx, rf2 * 1.2f, -iTime * 1.1f) - .01f)));
    col += (0.1f * nz * nz * f32x3(0.012f, 0.512f, .5f) +
            0.05f * nz2 * nz2 * f32x3(0.0055f, 0.2f, .55f)) *
           0.8f;
  }
  fragColor = f32x4(col * 1.3f, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
