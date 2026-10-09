// Port of https://www.shadertoy.com/view/DtGGRd; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtGGRd.json.
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
const float bstart = 0.0f;
const float bpm = 150.0f;
const float bhz = bpm / 60.0f;
const float bperiod = 8.0f / bhz;
float beatCount(Context &__ctx, float tm) { return (tm - bstart) * bhz; }
float beat(Context &__ctx, float tm) {
  return smoothstep(0.25f, 1.0f,
                    cos((2.0f * 3.141592654f) * beatCount(__ctx, tm)));
}
const f32x4 hsv2rgb_K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 p =
      abs(fract(c.swizzle<0, 0, 0>() + hsv2rgb_K.swizzle<0, 1, 2>()) * 6.0f -
          hsv2rgb_K.swizzle<3, 3, 3>());
  return c.data[2] * mix(hsv2rgb_K.swizzle<0, 0, 0>(),
                         clamp(p - hsv2rgb_K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
float hash(Context &__ctx, float co) {
  return fract(sin(co * 12.9898f) * 13758.5453f);
}
f32x3 skyColor(Context &__ctx, f32x3 ro, f32x3 rd) {
  const f32x3 gcol0 =
      (f32x3(0.55f, 0.9f, 0.035f * 0.5f).data[2] *
       mix(hsv2rgb_K.swizzle<0, 0, 0>(),
           clamp(
               abs(fract(f32x3(0.55f, 0.9f, 0.035f * 0.5f).swizzle<0, 0, 0>() +
                         hsv2rgb_K.swizzle<0, 1, 2>()) *
                       6.0f -
                   hsv2rgb_K.swizzle<3, 3, 3>()) -
                   hsv2rgb_K.swizzle<0, 0, 0>(),
               0.0f, 1.0f),
           f32x3(0.55f, 0.9f, 0.035f * 0.5f).data[1]));
  const f32x3 gcol1 =
      (f32x3(0.75f, 0.85f, 0.035f * 2.0f).data[2] *
       mix(hsv2rgb_K.swizzle<0, 0, 0>(),
           clamp(
               abs(fract(f32x3(0.75f, 0.85f, 0.035f * 2.0f).swizzle<0, 0, 0>() +
                         hsv2rgb_K.swizzle<0, 1, 2>()) *
                       6.0f -
                   hsv2rgb_K.swizzle<3, 3, 3>()) -
                   hsv2rgb_K.swizzle<0, 0, 0>(),
               0.0f, 1.0f),
           f32x3(0.75f, 0.85f, 0.035f * 2.0f).data[1]));
  float b = beat(__ctx, iTime);
  f32x2 pp = rd.swizzle<0, 1>();
  b *= step(((6.0f) * bperiod + bstart), iTime);
  return mix(gcol0, gcol1, b) / max(dot(pp, pp), 0.0001f);
}
float mod1(Context &__ctx, float &p, float size) {
  float halfsize = size * 0.5f;
  float c = floor((p + halfsize) / size);
  p = mod(p + halfsize, size) - halfsize;
  return c;
}
f32x2 rayCylinder(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 cb, f32x3 ca,
                  float cr) {
  f32x3 oc = ro - cb;
  float card = dot(ca, rd);
  float caoc = dot(ca, oc);
  float a = 1.0f - card * card;
  float b = dot(oc, rd) - caoc * card;
  float c = dot(oc, oc) - caoc * caoc - cr * cr;
  float h = b * b - a * c;
  if (h < 0.0f)
    return f32x2(-1.0f);
  h = sqrt(h);
  return f32x2(-b - h, -b + h) / a;
}
f32x3 color(Context &__ctx, f32x3 ww, f32x3 uu, f32x3 vv, f32x3 ro, f32x2 p) {
  const float rdd = 2.0f;
  const float mm = 5.0f;
  const float rep = 16.0f;
  f32x3 rd = normalize(-p.data[0] * uu + p.data[1] * vv + rdd * ww);
  f32x3 skyCol = skyColor(__ctx, ro, rd);
  f32x2 etc = rayCylinder(__ctx, ro, rd, ro, f32x3(0.0f, 0.0f, 1.0f), 1.0f);
  f32x3 etcp = ro + rd * etc.data[1];
  rd.set_swizzle<1, 0>(
      rd.swizzle<1, 0>() *
      (f32x2x2(cos(0.3f * etcp.data[2]), sin(0.3f * etcp.data[2]),
               -sin(0.3f * etcp.data[2]), cos(0.3f * etcp.data[2]))));
  f32x3 col = skyCol;
  float fi = smoothstep(((1.0f) * bperiod + bstart),
                        ((3.0f) * bperiod + bstart), iTime);
  float a = atan(rd.data[1], rd.data[0]);
  for (float i = 0.0f; i < mm; ++i) {
    float ma = a;
    float sz = rep + i * 9.0f;
    float slices = (2.0f * 3.141592654f) / sz;
    float na = mod1(__ctx, ma, slices);
    float h1 = hash(__ctx, na + 13.0f * i + 123.4f);
    float h2 = fract(h1 * 3677.0f);
    float h3 = fract(h1 * 8677.0f);
    float tr = mix(0.5f, 3.0f, h1);
    f32x2 tc = rayCylinder(__ctx, ro, rd, ro, f32x3(0.0f, 0.0f, 1.0f), tr);
    f32x3 tcp = ro + tc.data[1] * rd;
    f32x2 tcp2 = f32x2(tcp.data[2], a);
    float zz = mix(0.025f, 0.05f, sqrt(h1)) * rep / sz;
    float tnpy = mod1(__ctx, tcp2.data[1], slices);
    float fo = smoothstep(0.5f * slices, 0.25f * slices, abs(tcp2.data[1]));
    tcp2.data[0] += h2 * iTime;
    tcp2.data[1] *= tr * (3.141592654f / 3.0f);
    f32x2 tcp3 = tcp2;
    float w = mix(.2f, 1.0f, h2);
    float tnpx = mod1(__ctx, tcp3.data[0], w);
    float h4 = hash(__ctx, tnpx + 123.4f);
    tcp2 /= zz;
    tcp3 /= zz;
    float d1 = abs(tcp2.data[1]);
    float d2 = length(tcp3) - 2.0f * h4;
    d2 = abs(d2) - 1.0f * h4;
    d2 = abs(d2) - 0.5f * h4;
    float d = mix(d1, d2, fi * (0.5f + 0.5f * sin(0.01f * tcp2.data[0])));
    d *= zz;
    f32x3 bcol =
        (1.0f + cos(f32x3(0.0f, 1.0f, 2.0f) + (2.0f * 3.141592654f) * h3 +
                    0.5f * h2 * h2 * tcp.data[2])) *
        0.00005f;
    bcol /= max(d * d, 0.00001f + 5E-7f * tc.data[1] * tc.data[1]);
    bcol *= exp(-0.04f * tc.data[1] * tc.data[1]);
    bcol *= smoothstep(-0.5f, 1.0f, sin(mix(0.125f, 1.0f, h2) * tcp.data[2]));
    bcol *= fo;
    col += bcol;
  }
  return col;
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
f32x3 effect(Context &__ctx, f32x2 p, f32x2 pp) {
  float tm = 3.0f * iTime;
  f32x3 ro = f32x3(0.0f, 0.0f, tm);
  f32x3 dro = normalize(f32x3(1.0f, 0.0f, 3.0f));
  dro.set_swizzle<0, 2>(
      dro.swizzle<0, 2>() *
      (f32x2x2(cos(0.2f * sin(0.05f * tm)), sin(0.2f * sin(0.05f * tm)),
               -sin(0.2f * sin(0.05f * tm)), cos(0.2f * sin(0.05f * tm)))));
  dro.set_swizzle<1, 2>(dro.swizzle<1, 2>() *
                        (f32x2x2(cos(0.2f * sin(0.05f * tm * sqrt(0.5f))),
                                 sin(0.2f * sin(0.05f * tm * sqrt(0.5f))),
                                 -sin(0.2f * sin(0.05f * tm * sqrt(0.5f))),
                                 cos(0.2f * sin(0.05f * tm * sqrt(0.5f))))));
  const f32x3 up = f32x3(0.0f, 1.0f, 0.0f);
  f32x3 ww = normalize(dro);
  f32x3 uu = normalize(cross(up, ww));
  f32x3 vv = (cross(ww, uu));
  f32x3 col = color(__ctx, ww, uu, vv, ro, p);
  col -= 0.125f * f32x3(0.0f, 1.0f, 2.0f).swizzle<1, 2, 0>() * length(pp);
  col = aces_approx(__ctx, col);
  col *= smoothstep(((0.0f) * bperiod + bstart), ((0.25f) * bperiod + bstart),
                    iTime);
  col = sqrt(col);
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
