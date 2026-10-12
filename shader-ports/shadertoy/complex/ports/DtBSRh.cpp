// Port of https://www.shadertoy.com/view/DtBSRh; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtBSRh.json.
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
const float planeDist = 1.0f - 0.82f;
const f32x4 hsv2rgb_K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 p =
      abs(fract(c.swizzle<0, 0, 0>() + hsv2rgb_K.swizzle<0, 1, 2>()) * 6.0f -
          hsv2rgb_K.swizzle<3, 3, 3>());
  return c.data[2] * mix(hsv2rgb_K.swizzle<0, 0, 0>(),
                         clamp(p - hsv2rgb_K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
f32x3 sRGB(Context &__ctx, f32x3 t) {
  return mix(1.055f * pow(t, f32x3(1.f / 2.4f)) - 0.055f, 12.92f * t,
             step(t, f32x3(0.0031308f)));
}
f32x3 aces_approx(Context &__ctx, f32x3 v) {
  v = max(v, 0.0f);
  v *= 0.6f;
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
}
float hash3(Context &__ctx, f32x3 r) {
  return fract(sin(dot(r.swizzle<0, 1>(), f32x2(1.38984f * sin(r.data[2]),
                                                1.13233f * cos(r.data[2])))) *
               653758.5453f);
}
f32x2 mod2(Context &__ctx, f32x2 &p, f32x2 size) {
  f32x2 c = floor((p + size * 0.5f) / size);
  p = mod(p + size * 0.5f, size) - size * 0.5f;
  return c;
}
f32x2 soff(Context &__ctx, float h0) {
  float h1 = fract(h0 * 8677.0f);
  return -1.0f + 2.0f * f32x2(h0, h1);
}
f32x3 offset(Context &__ctx, float z) {
  float a = z;
  f32x2 p = -mix(0.0f, 0.1f, smoothstep(40.0f, 50.0f, iTime)) *
            (f32x2(cos(a), sin(a * sqrt(2.0f))) +
             f32x2(cos(a * sqrt(0.75f)), sin(a * sqrt(0.5f))));
  return f32x3(p, z);
}
f32x3 doffset(Context &__ctx, float z) {
  float eps = 0.05f;
  return (offset(__ctx, z + eps) - offset(__ctx, z - eps)) / (2.0f * eps);
}
f32x3 ddoffset(Context &__ctx, float z) {
  float eps = 0.05f;
  return (doffset(__ctx, z + eps) - doffset(__ctx, z - eps)) / (2.0f * eps);
}
f32x3 skyColor(Context &__ctx, f32x3 ro, f32x3 rd) {
  const f32x3 gcol =
      (f32x3(0.55f, 0.9f, 0.035f).data[2] *
       mix(hsv2rgb_K.swizzle<0, 0, 0>(),
           clamp(abs(fract(f32x3(0.55f, 0.9f, 0.035f).swizzle<0, 0, 0>() +
                           hsv2rgb_K.swizzle<0, 1, 2>()) *
                         6.0f -
                     hsv2rgb_K.swizzle<3, 3, 3>()) -
                     hsv2rgb_K.swizzle<0, 0, 0>(),
                 0.0f, 1.0f),
           f32x3(0.55f, 0.9f, 0.035f).data[1]));
  f32x2 pp = rd.swizzle<0, 1>();
  return gcol / dot(pp, pp);
}
f32x3 plane(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 pp, f32x3 off, float aa,
            float n) {
  float l = distance(ro, pp);
  f32x3 hn;
  f32x2 p = (pp - off * f32x3(1.0f, 1.0f, 0.0f)).swizzle<0, 1>();
  p *= 1.0f + l * l;
  const float csz = 0.15f;
  const float co = 0.33f * csz;
  f32x3 col = f32x3(0.0f);
  const float cnt = 5.0f;
  const float icnt = 1.0f / cnt;
  for (float i = 0.0f; i < 1.0f; i += icnt) {
    f32x2 cp = p;
    f32x2 cn = mod2(__ctx, cp, f32x2(csz));
    float h0 = hash3(__ctx, f32x3(cn, n) + 123.4f + i);
    float h1 = fract(3677.0f * h0);
    cp += soff(__ctx, h0) * co;
    float cl = length(cp);
    float d = (cl - 0.0005f);
    d = max(d, 0.0001f);
    f32x3 bcol =
        0.000005f *
        (1.0f + sin(f32x3(0.0f, 1.0f, 2.0f) + (2.0f * 3.141592654f) * h1));
    bcol *= smoothstep(0.17f * csz, 0.05f * csz, cl) * 0.15f /
            (d * d * (l * l + 0.01f));
    col += bcol;
    p += icnt;
    p *= f32x2x2(cos(1.0f), sin(1.0f), -sin(1.0f), cos(1.0f));
  }
  return col;
}
f32x3 color(Context &__ctx, f32x3 ww, f32x3 uu, f32x3 vv, f32x3 ro, f32x2 p) {
  float lp = length(p);
  f32x2 np = p + 1.0f / iResolution.swizzle<0, 1>();
  float rdd = 3.0f;
  f32x3 rd = normalize(-p.data[0] * uu + p.data[1] * vv + rdd * ww);
  f32x3 nrd = normalize(-np.data[0] * uu + np.data[1] * vv + rdd * ww);
  const int furthest = 9;
  const int fadeFrom = max(furthest - 2, 0);
  const float fadeDist = planeDist * float(furthest - fadeFrom);
  float nz = floor(ro.data[2] / planeDist);
  f32x3 col = skyColor(__ctx, ro, rd);
  for (int i = 1; i <= furthest; ++i) {
    float pz = planeDist * nz + planeDist * float(i);
    float pd = (pz - ro.data[2]) / rd.data[2];
    if (pd > 0.0f) {
      f32x3 pp = ro + rd * pd;
      f32x3 npp = ro + nrd * pd;
      float aa = 3.0f * length(pp - npp);
      f32x3 off = offset(__ctx, pp.data[2]);
      f32x3 pcol = plane(__ctx, ro, rd, pp, off, aa, nz + float(i));
      float nz = pp.data[2] - ro.data[2];
      float fadeIn = smoothstep(planeDist * float(furthest),
                                planeDist * float(fadeFrom), nz);
      float fadeOut = smoothstep(0.0f, planeDist * 0.1f, nz);
      pcol *= fadeOut * fadeIn;
      col += pcol;
    }
  }
  return col;
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 pp) {
  float tm = planeDist * iTime * (145.0f * 0.5f) / 60.0f;
  f32x3 ro = offset(__ctx, tm);
  f32x3 dro = doffset(__ctx, tm);
  f32x3 ddro = ddoffset(__ctx, tm);
  f32x3 ww = normalize(dro);
  f32x3 uu = normalize(cross(normalize(f32x3(0.0f, 1.0f, 0.0f) + ddro), ww));
  f32x3 vv = cross(ww, uu);
  f32x3 col = color(__ctx, ww, uu, vv, ro, p);
  col -= .1f * f32x3(0.0f, 1.0f, 2.0f).swizzle<2, 1, 0>() * length(pp);
  col *= smoothstep(1.5f, 0.5f, length(pp));
  col *= smoothstep(1.0f, 15.0f, iTime);
  col = aces_approx(__ctx, col);
  col = sRGB(__ctx, col);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  f32x2 pp = p;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = effect(__ctx, p, pp);
  fragColor = f32x4(col, 1.0f);
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
