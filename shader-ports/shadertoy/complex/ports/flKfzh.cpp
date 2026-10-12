// Port of https://www.shadertoy.com/view/flKfzh; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/flKfzh.json.
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
const float hf = 0.015f;
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
float tanh_approx(Context &__ctx, float x) {
  float x2 = x * x;
  return clamp(x * (27.0f + x2) / (27.0f + 9.0f * x2), -1.0f, 1.0f);
}
float atan_approx(Context &__ctx, float y, float x) {
  float cosatan2 = x / (abs(x) + abs(y));
  float t = (0.5f * 3.141592654f) - cosatan2 * (0.5f * 3.141592654f);
  return y < 0.0f ? -t : t;
}
float pmin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float pabs(Context &__ctx, float a, float k) { return -pmin(__ctx, a, -a, k); }
float height(Context &__ctx, f32x2 p) {
  float tm = iTime;
  const float xm = 0.5f * 0.005123f;
  float ym = mix(0.125f, 0.25f,
                 0.5f - 0.5f * cos((2.0f * 3.141592654f) * iTime / 600.0f));
  p *= 0.4f;
  float d = length(p);
  float c = 1E6f;
  float x = pow(d, 0.1f);
  float y = (atan(p.data[0], p.data[1]) + 0.05f * tm - 2.0f * d) /
            (2.0f * 3.141592654f);
  for (float i = 0.f; i < 3.f; ++i) {
    float v = length(
        fract(f32x2(x - tm * i * xm, fract(y + i * ym) * .5f) * 20.f) * 2.f -
        1.f);
    c = pmin(__ctx, c, v, 0.125f);
  }
  float h =
      (-hf + hf * (pabs(__ctx,
                        tanh_approx(__ctx, 5.5f * d - 80.f * c * c * d * d *
                                                          (.55f - d)) -
                            0.25f * d,
                        0.25f)));
  return h;
}
f32x3 normal(Context &__ctx, f32x2 p) {
  f32x2 e = f32x2(4.0f / iResolution.data[1], 0);
  f32x3 n;
  n.data[0] = height(__ctx, p + e.swizzle<0, 1>()) -
              height(__ctx, p - e.swizzle<0, 1>());
  n.data[1] = -2.0f * e.data[0];
  n.data[2] = height(__ctx, p + e.swizzle<1, 0>()) -
              height(__ctx, p - e.swizzle<1, 0>());
  return normalize(n);
}
f32x3 color(Context &__ctx, f32x2 p) {
  const float ss = 1.25f;
  const float hh = 1.95f;
  const f32x3 lp1 = -f32x3(1.0f, hh, -1.0f) * f32x3(ss, 1.0f, ss);
  const f32x3 lp2 = -f32x3(-1.0f, hh, -1.0f) * f32x3(ss, 1.0f, ss);
  const f32x3 lcol1 =
      (f32x3(0.30f, 0.35f, 2.0f).data[2] *
       mix(hsv2rgb_K.swizzle<0, 0, 0>(),
           clamp(abs(fract(f32x3(0.30f, 0.35f, 2.0f).swizzle<0, 0, 0>() +
                           hsv2rgb_K.swizzle<0, 1, 2>()) *
                         6.0f -
                     hsv2rgb_K.swizzle<3, 3, 3>()) -
                     hsv2rgb_K.swizzle<0, 0, 0>(),
                 0.0f, 1.0f),
           f32x3(0.30f, 0.35f, 2.0f).data[1]));
  const f32x3 lcol2 =
      (f32x3(0.57f, 0.6f, 2.0f).data[2] *
       mix(hsv2rgb_K.swizzle<0, 0, 0>(),
           clamp(abs(fract(f32x3(0.57f, 0.6f, 2.0f).swizzle<0, 0, 0>() +
                           hsv2rgb_K.swizzle<0, 1, 2>()) *
                         6.0f -
                     hsv2rgb_K.swizzle<3, 3, 3>()) -
                     hsv2rgb_K.swizzle<0, 0, 0>(),
                 0.0f, 1.0f),
           f32x3(0.57f, 0.6f, 2.0f).data[1]));
  const f32x3 mat =
      (f32x3(0.55f, 0.9f, 0.05f).data[2] *
       mix(hsv2rgb_K.swizzle<0, 0, 0>(),
           clamp(abs(fract(f32x3(0.55f, 0.9f, 0.05f).swizzle<0, 0, 0>() +
                           hsv2rgb_K.swizzle<0, 1, 2>()) *
                         6.0f -
                     hsv2rgb_K.swizzle<3, 3, 3>()) -
                     hsv2rgb_K.swizzle<0, 0, 0>(),
                 0.0f, 1.0f),
           f32x3(0.55f, 0.9f, 0.05f).data[1]));
  const float spe = 16.0f;
  float h = height(__ctx, p);
  f32x3 n = normal(__ctx, p);
  f32x3 ro = f32x3(0.0f, 8.0f, 0.0f);
  f32x3 pp = f32x3(p.data[0], 0.0f, p.data[1]);
  f32x3 po = f32x3(p.data[0], 0.0f, p.data[1]);
  f32x3 rd = normalize(ro - po);
  f32x3 ld1 = normalize(lp1 - po);
  f32x3 ld2 = normalize(lp2 - po);
  float diff1 = max(dot(n, ld1), 0.0f);
  float diff2 = max(dot(n, ld2), 0.0f);
  f32x3 rn = n;
  f32x3 ref = reflect(rd, rn);
  float ref1 = max(dot(ref, ld1), 0.0f);
  float ref2 = max(dot(ref, ld2), 0.0f);
  float dm = tanh_approx(__ctx, abs(h) * 120.0f);
  float rm = dm;
  dm *= dm;
  f32x3 lpow1 = dm * mat * lcol1;
  f32x3 lpow2 = dm * mat * lcol2;
  f32x3 col = f32x3(0.0f);
  col += diff1 * diff1 * lpow1;
  col += diff2 * diff2 * lpow2;
  col += rm * pow(ref1, spe) * lcol1;
  col += rm * pow(ref2, spe) * lcol2;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord / iResolution.swizzle<0, 1>();
  f32x2 p = -1.f + 2.f * q;
  p.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 col = color(__ctx, p);
  col = aces_approx(__ctx, col);
  col = sRGB(__ctx, col);
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
