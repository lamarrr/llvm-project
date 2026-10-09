// Port of https://www.shadertoy.com/view/mdjXzc; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdjXzc.json.
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
  Texture3D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float hash(Context &__ctx, float a, float b) {
  int x = floatBitsToInt(a * a / 7.f) ^ floatBitsToInt(a),
      y = floatBitsToInt(b * b / 7.f) ^ floatBitsToInt(b);
  return float((x * x + y) * (y * y - x) - x) / 2.14e9f;
}
f32x3 erot(Context &__ctx, f32x3 p, f32x3 ax, float ro) {
  return mix(dot(ax, p) * ax, p, cos(ro)) + cross(ax, p) * sin(ro);
}
float cc(Context &__ctx, float a, float b) {
  float f = tanh(a * cos(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
float cs(Context &__ctx, float a, float b) {
  float f = tanh(a * sin(b)) / tanh(a);
  return sign(f) * pow(abs(f), 0.25f);
}
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float mlength(Context &__ctx, f32x2 uv) {
  return max(abs(uv.data[0]), abs(uv.data[1]));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
float sfloor(Context &__ctx, float a, float b) {
  return floor(b - .5f) + .5f +
         .5f * tanh(a * (fract(b - .5f) - .5f)) / tanh(.5f * a);
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) - k * h * (1.f - h);
}
float smax(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f - 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) + k * h * (1.f - h);
}
float map(Context &__ctx, f32x3 p) {
  f32x3 q = f32x3(0);
  p *= 0.7f;
  q = texture(__ctx.channel0, 0.25f * p).swizzle<0, 1, 2>();
  q = mix(
      q,
      texture(__ctx.channel0, 0.5f * p.swizzle<1, 2, 0>()).swizzle<0, 1, 2>(),
      0.5f);
  q = mix(q, texture(__ctx.channel0, p.swizzle<2, 1, 0>()).swizzle<0, 1, 2>(),
          0.25f);
  q = mix(q, texture(__ctx.channel0, 2.f * p).swizzle<0, 1, 2>(), 0.125f);
  return clamp(q.data[2], 0.f, 1.f);
}
f32x4 march(Context &__ctx, f32x3 ro, f32x3 rd) {
  float d = 0.f;
  int steps = 180;
  float o = 3.1415926535f / 3.f;
  f32x3 off = tanh(2.f * cos(f32x3(-o, 0, o) + 0.15f * iTime)) / tanh(2.f);
  f32x3 p = ro;
  float inc = 0.005f;
  for (int i = 0; i < 180; i++) {
    p = ro + off * rd * d;
    float dd = map(__ctx, p);
    rd = erot(__ctx, rd, off, 0.01f * cos(8.f * d + 0.2f * iTime));
    d += inc;
    if (1.5f * dd <
        d * (0.99f + 0.02f * hash(__ctx, p.data[0],
                                  hash(__ctx, p.data[1], p.data[2])))) {
      steps = i;
      break;
    }
  }
  return f32x4(p, steps);
}
f32x3 dir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
f32x3 norm(Context &__ctx, f32x3 p) {
  float d = map(__ctx, p);
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(map(__ctx, p - e.swizzle<0, 1, 1>()),
                      map(__ctx, p - e.swizzle<1, 0, 1>()),
                      map(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 off = 0.15f * iTime * f32x3(1, 0.9f, 0.95f);
  f32x3 ro = 1.f - off;
  f32x3 rd = dir(__ctx, uv, ro, -off, 1.f);
  f32x4 m = march(__ctx, ro, rd);
  f32x3 p = m.swizzle<0, 1, 2>();
  float d = length(p - ro);
  f32x3 n = norm(__ctx, p);
  f32x3 ld = normalize(f32x3(1, 2, 3));
  float dif = .9f + .1f * dot(n, ld);
  f32x3 col = .5f + .5f * cos(2.f * 3.1415926535f *
                              (0.25f + 0.4f * mlength(__ctx, p) +
                               0.5f * f32x3(0, 1, 2) / 3.f));
  float mx = m.data[3] / float(180);
  mx = pow(mx, 3.f);
  float v = mix(dif, 1.f, mx);
  v = clamp(v, 0.f, 1.f);
  col -= 0.12f * v;
  col = mix(col, f32x3(.5f, .5f, 1), mx);
  col = mix(col, f32x3(1, 0, 0), max(0.f, 1.f - 2.f * mx));
  col *= 2.f * map(__ctx, p - 0.004f * rd);
  float d2 = mlength(__ctx, uv);
  float k = 1.f / iResolution.data[1];
  float s2 = smoothstep(-k, k, -abs(-d2 + 0.45f) + 2.f * k);
  col += tanh(.5f * iTime) * .8f * s2 * smoothstep(-4.f * k, 4.f * k, d - 0.3f);
  col = clamp(col, 0.f, 1.f);
  col = pow(col, f32x3(1.f / 2.2f));
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_3d<f32x4> channel0,
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
