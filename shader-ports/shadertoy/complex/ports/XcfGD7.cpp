// Port of https://www.shadertoy.com/view/XcfGD7; author: sancho1509.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/XcfGD7.json.
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
float sdSphere(Context &__ctx, f32x3 p, float s) { return length(p) - s; }
float sdTorus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
float sdOctahedron(Context &__ctx, f32x3 p, float s) {
  p = abs(p);
  float m = p.data[0] + p.data[1] + p.data[2] - s;
  f32x3 q;
  if (3.0f * p.data[0] < m)
    q = p.swizzle<0, 1, 2>();
  else if (3.0f * p.data[1] < m)
    q = p.swizzle<1, 2, 0>();
  else if (3.0f * p.data[2] < m)
    q = p.swizzle<2, 0, 1>();
  else
    return m * 0.57735027f;
  float k = clamp(0.5f * (q.data[2] - q.data[1] + s), 0.0f, s);
  return length(f32x3(q.data[0], q.data[1] - s + k, q.data[2] - k));
}
float sdPyramid(Context &__ctx, f32x3 p, float h) {
  float m2 = h * h + 0.25f;
  p.set_swizzle<0, 2>(abs(p.swizzle<0, 2>()));
  p.set_swizzle<0, 2>((p.data[2] > p.data[0]) ? p.swizzle<2, 0>()
                                              : p.swizzle<0, 2>());
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() - (0.5f));
  f32x3 q = f32x3(p.data[2], h * p.data[1] - 0.5f * p.data[0],
                  h * p.data[0] + 0.5f * p.data[1]);
  float s = max(-q.data[0], 0.0f);
  float t = clamp((q.data[1] - 0.5f * p.data[2]) / (m2 + 0.25f), 0.0f, 1.0f);
  float a = m2 * (q.data[0] + s) * (q.data[0] + s) + q.data[1] * q.data[1];
  float b = m2 * (q.data[0] + 0.5f * t) * (q.data[0] + 0.5f * t) +
            (q.data[1] - m2 * t) * (q.data[1] - m2 * t);
  float d2 = min(q.data[1], -q.data[0] * m2 - q.data[1] * 0.5f) > 0.0f
                 ? 0.0f
                 : min(a, b);
  return sqrt((d2 + q.data[2] * q.data[2]) / m2) *
         sign(max(q.data[2], -p.data[1]));
}
float sdEllipsoid(Context &__ctx, f32x3 p, f32x3 r) {
  float k0 = length(p / r);
  float k1 = length(p / (r * r));
  return k0 * (k0 - 1.0f) / k1;
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = max(k - abs(a - b), 0.0f) / k;
  return min(a, b) - h * h * h * k * (1.0f / 6.0f);
}
f32x3 rot3D(Context &__ctx, f32x3 p, f32x3 axis, float angle) {
  return mix(dot(axis, p) * axis, p, cos(angle)) + cross(axis, p) * sin(angle);
}
f32x2x2 rot2D(Context &__ctx, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return f32x2x2(c, -s, s, c);
}
f32x3 palette(Context &__ctx, float t) {
  float time = iTime * 0.05f;
  time += 230.f;
  f32x3 vector = f32x3(
      clamp(smoothstep(0.2f, 0.8f, 0.8f + 0.5f * sin(0.6f * time)), 0.3f, .7f),
      clamp(smoothstep(0.2f, 0.8f, 0.5f + 0.5f * cos(0.8f * time + 2.0f)), 0.5f,
            .6f),
      clamp(smoothstep(0.2f, 0.8f, 0.5f + 0.5f * sin(1.5f * time + 4.0f)), 0.2f,
            .4f));
  return .5f + .5f * cos(6.28318f * (t + vector));
}
float movementCurve(Context &__ctx, float x) {
  return -2.0f / (1.0f + exp(-10.0f * (0.5f * x - 0.5f))) + 1.0f;
}
float PI = 3.1416f;
float fovMult(Context &__ctx, float x, float a, float d) {
  return a + d + sin(PI * x - sqrt(PI)) * a;
}
float map(Context &__ctx, f32x3 p) {
  f32x3 q = p;
  q.data[2] += iTime * 0.8f;
  q.set_swizzle<0, 1>(fract(f32x2(q.data[0], q.data[1])) - .5f);
  q.data[2] = mod(q.data[2], .3f) - .125f;
  float oct = sdEllipsoid(
      __ctx, q, f32x3(fovMult(__ctx, iTime * 0.1f, 0.6f, 0.01f) * .3f));
  return oct;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (iMouse.swizzle<0, 1>() * 2.f - iResolution.swizzle<0, 1>()) /
            iResolution.data[1];
  if (iMouse.data[2] < 0.f)
    m = f32x2(cos(iTime * .05f), sin(iTime * .25f));
  f32x3 ro = f32x3(0, 0, -5);
  f32x3 rd = normalize(
      f32x3(uv / (5.f * fovMult(__ctx, iTime * 0.01f, 0.57f, 0.03f)), 1));
  f32x3 col = f32x3(0);
  float t = 0.f;
  int i = 0;
  for (; i < 130; i++) {
    f32x3 p = ro + rd * t;
    int tx = int((t / 10000.f) * 512.0f);
    float fft = texelFetch(__ctx.channel0, i32x2(tx, 0)).data[0];
    float wave = texelFetch(__ctx.channel0, i32x2(tx, 1)).data[0];
    p.data[1] += 0.5f * sin(t * (m.data[1] + 1.f) * .5f) * .7f;
    p.set_swizzle<0, 1>(p.swizzle<0, 1>() *
                        (rot2D(__ctx, t * .2f * m.data[0])));
    p.data[1] +=
        sin(cos(tan(cos(t * 6.f * fovMult(__ctx, -iTime * 0.02f, 0.5f, 0.3f))) *
                fft) /
            10.f);
    p.data[1] += 0.5f * cos(t * (m.data[1] + 1.f) * .5f) * .7f;
    float d = map(__ctx, p);
    t += d;
    if (t >= 10000.f || d <= 0.003f)
      break;
  }
  col = palette(__ctx, t * 0.01f + float(i) * 0.01f);
  fragColor = f32x4(col, 1);
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
