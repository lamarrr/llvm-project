// Port of https://www.shadertoy.com/view/DslSRS; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DslSRS.json.
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
float cc(Context &__ctx, float a, float b) {
  float f = clamp(a * cos(b) * (27.f + a * cos(b) * a * cos(b)) /
                      (27.f + 9.f * a * cos(b) * a * cos(b)),
                  -1.f, 1.f) /
            clamp(a * (27.f + a * a) / (27.f + 9.f * a * a), -1.f, 1.f);
  return sign(f) * pow(abs(f), 0.25f);
}
float cs(Context &__ctx, float a, float b) {
  float f = clamp(a * sin(b) * (27.f + a * sin(b) * a * sin(b)) /
                      (27.f + 9.f * a * sin(b) * a * sin(b)),
                  -1.f, 1.f) /
            clamp(a * (27.f + a * a) / (27.f + 9.f * a * a), -1.f, 1.f);
  return sign(f) * pow(abs(f), 0.25f);
}
f32x3 pal(Context &__ctx, float t, f32x3 d) {
  return 0.5f + 0.5f * cos(2.f * 3.14159f * (0.5f * t + d));
}
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(2.f * 3.14159f * (c * t + d));
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
  return floor(b) + 0.5f + 0.5f * tanh(a * (fract(b) - 0.5f)) / tanh(0.5f * a);
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) - k * h * (1.f - h);
}
float smax(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f - 0.5f * (b - a) / k, 0.f, 1.f);
  return mix(b, a, h) + k * h * (1.f - h);
}
f32x3 ACESFilm(Context &__ctx, f32x3 x) {
  return clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f),
               0.0f, 1.0f);
}
f32x3 rayOri(Context &__ctx) {
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 ro = f32x3(0, 6.f * cos(0.25f * iTime), 10);
  ro.set_swizzle<0, 2>(
      ro.swizzle<0, 2>() *
      (f32x2x2(cos(3.14159f / 12.f *
                   clamp(2.f * cos(0.5f * iTime) *
                             (27.f + 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)) /
                             (27.f + 9.f * 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)),
                         -1.f, 1.f) /
                   clamp(2.f * (27.f + 2.f * 2.f) / (27.f + 9.f * 2.f * 2.f),
                         -1.f, 1.f)),
               -sin(3.14159f / 12.f *
                    clamp(2.f * cos(0.5f * iTime) *
                              (27.f + 2.f * cos(0.5f * iTime) * 2.f *
                                          cos(0.5f * iTime)) /
                              (27.f + 9.f * 2.f * cos(0.5f * iTime) * 2.f *
                                          cos(0.5f * iTime)),
                          -1.f, 1.f) /
                    clamp(2.f * (27.f + 2.f * 2.f) / (27.f + 9.f * 2.f * 2.f),
                          -1.f, 1.f)),
               sin(3.14159f / 12.f *
                   clamp(2.f * cos(0.5f * iTime) *
                             (27.f + 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)) /
                             (27.f + 9.f * 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)),
                         -1.f, 1.f) /
                   clamp(2.f * (27.f + 2.f * 2.f) / (27.f + 9.f * 2.f * 2.f),
                         -1.f, 1.f)),
               cos(3.14159f / 12.f *
                   clamp(2.f * cos(0.5f * iTime) *
                             (27.f + 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)) /
                             (27.f + 9.f * 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)),
                         -1.f, 1.f) /
                   clamp(2.f * (27.f + 2.f * 2.f) / (27.f + 9.f * 2.f * 2.f),
                         -1.f, 1.f)))));
  return ro;
}
float dist(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 2>(
      p.swizzle<0, 2>() *
      (f32x2x2(cos(3.14159f / 12.f *
                   clamp(2.f * cos(0.5f * iTime) *
                             (27.f + 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)) /
                             (27.f + 9.f * 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)),
                         -1.f, 1.f) /
                   clamp(2.f * (27.f + 2.f * 2.f) / (27.f + 9.f * 2.f * 2.f),
                         -1.f, 1.f)),
               -sin(3.14159f / 12.f *
                    clamp(2.f * cos(0.5f * iTime) *
                              (27.f + 2.f * cos(0.5f * iTime) * 2.f *
                                          cos(0.5f * iTime)) /
                              (27.f + 9.f * 2.f * cos(0.5f * iTime) * 2.f *
                                          cos(0.5f * iTime)),
                          -1.f, 1.f) /
                    clamp(2.f * (27.f + 2.f * 2.f) / (27.f + 9.f * 2.f * 2.f),
                          -1.f, 1.f)),
               sin(3.14159f / 12.f *
                   clamp(2.f * cos(0.5f * iTime) *
                             (27.f + 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)) /
                             (27.f + 9.f * 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)),
                         -1.f, 1.f) /
                   clamp(2.f * (27.f + 2.f * 2.f) / (27.f + 9.f * 2.f * 2.f),
                         -1.f, 1.f)),
               cos(3.14159f / 12.f *
                   clamp(2.f * cos(0.5f * iTime) *
                             (27.f + 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)) /
                             (27.f + 9.f * 2.f * cos(0.5f * iTime) * 2.f *
                                         cos(0.5f * iTime)),
                         -1.f, 1.f) /
                   clamp(2.f * (27.f + 2.f * 2.f) / (27.f + 9.f * 2.f * 2.f),
                         -1.f, 1.f)))));
  float pd = 2.5f * p.data[2] + 7.f + 0.5f - abs(p.data[0]);
  float d = 100.f;
  float osc = 0.5f + 0.25f * cos(3.14159f * exp(-length(p)) - iTime);
  float m = 1.f - osc;
  float th = tanh(0.04f * iTime);
  for (float i = 0.f; i < 4.f; i++) {
    float io = 3.14159f * i / 4.f;
    p = sqrt(p * p + 1e-3f) - sqrt(2.f) * m;
    p.set_swizzle<0, 1>(
        p.swizzle<0, 1>() *
        (f32x2x2(cos(io + 0.5f * iTime), -sin(io + 0.5f * iTime),
                 sin(io + 0.5f * iTime), cos(io + 0.5f * iTime))));
    p.set_swizzle<1, 2>(
        p.swizzle<1, 2>() *
        (f32x2x2(cos(io + 0.25f * iTime), -sin(io + 0.25f * iTime),
                 sin(io + 0.25f * iTime), cos(io + 0.25f * iTime))));
    d = smin(__ctx, d,
             smax(__ctx,
                  min(length(p.swizzle<0, 1>()),
                      min(mlength(__ctx, p.swizzle<0, 2>()),
                          length(p.swizzle<1, 2>()))),
                  length(p) -
                      (1.f + th *
                                 clamp(2.f * sin(io + iTime) *
                                           (27.f + 2.f * sin(io + iTime) * 2.f *
                                                       sin(io + iTime)) /
                                           (27.f + 9.f * 2.f * sin(io + iTime) *
                                                       2.f * sin(io + iTime)),
                                       -1.f, 1.f) /
                                 clamp(2.f * (27.f + 2.f * 2.f) /
                                           (27.f + 9.f * 2.f * 2.f),
                                       -1.f, 1.f)) *
                          m,
                  0.02f) -
                 0.75f * m,
             0.1f);
    m *= osc;
  }
  return min(pd, d);
}
float march(Context &__ctx, f32x3 ro, f32x3 rd, float z) {
  float dO = 0.f;
  float s = sign(z);
  for (int i = 0; i < 400; i++) {
    f32x3 p = ro + rd * dO;
    float dS = dist(__ctx, p);
    if (s != sign(dS)) {
      z *= 0.5f;
      s = sign(dS);
    }
    if (abs(dS) < .001f || dO > 100.f)
      break;
    dO += dS * z;
  }
  return min(dO, 100.f);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  float d = dist(__ctx, p);
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(dist(__ctx, p - e.swizzle<0, 1, 1>()),
                      dist(__ctx, p - e.swizzle<1, 0, 1>()),
                      dist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 rayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = rayOri(__ctx);
  float th = tanh(0.1f * iTime);
  f32x3 rd = rayDir(__ctx, uv, ro, f32x3(0), 0.75f + 1.25f * th);
  f32x3 col = f32x3(0);
  float d = march(__ctx, ro, rd, 1.f);
  f32x3 p = ro + rd * d;
  if (d < 100.f) {
    f32x3 n = normal(__ctx, p);
    f32x3 r = reflect(rd, n);
    f32x3 ld = normalize(f32x3(1, 3, 3));
    float dif = 0.5f + 0.5f * dot(n, ld);
    float spec = pow(dif, 32.f);
    f32x3 q = p + 2.f * .001f * n;
    float d2 = march(__ctx, q, ld, 1.f);
    col = spec * f32x3(clamp(0.15f * d2, 0.f, 1.f));
    f32x3 q2 = p + mix(50.f, 150.f, th) * .001f * r;
    float d3 = march(__ctx, q2, rd, 1.f);
    f32x3 p2 = q2 + rd * d3;
    f32x3 c1 = texture(__ctx.channel0, p2.swizzle<0, 1>()).swizzle<0, 1, 2>();
    f32x3 c2 = texture(__ctx.channel0, p2.swizzle<1, 2>()).swizzle<0, 1, 2>();
    f32x3 c3 = texture(__ctx.channel0, p2.swizzle<2, 0>()).swizzle<0, 1, 2>();
    n = abs(n);
    f32x3 c = n.data[2] * c1 + n.data[0] * c2 + n.data[1] * c3;
    col *= c;
    f32x3 tx3 = texture(__ctx.channel1, r).swizzle<0, 1, 2>();
    col *=
        2.f *
        (1.f + clamp(4.f * cos(3.14159f * 1.1f + 4.f * tx3) *
                         (27.f + 4.f * cos(3.14159f * 1.1f + 4.f * tx3) * 4.f *
                                     cos(3.14159f * 1.1f + 4.f * tx3)) /
                         (27.f + 9.f * 4.f * cos(3.14159f * 1.1f + 4.f * tx3) *
                                     4.f * cos(3.14159f * 1.1f + 4.f * tx3)),
                     -1.f, 1.f) /
                   clamp(4.f * (27.f + 4.f * 4.f) / (27.f + 9.f * 4.f * 4.f),
                         -1.f, 1.f)) *
        tx3;
    col = clamp(col, 0.f, 1.f);
    col = mix(col, f32x3(1.f - c), clamp(0.25f * d3, 0.f, 1.f));
    f32x3 fogCol = mix(f32x3(0), f32x3(234, 201, 103) / 255.f, 1.f - th);
    col = mix(col, fogCol, 1.f - 1.f / cosh(.06f * length(p.swizzle<0, 1>())));
    col = ACESFilm(__ctx, col);
  }
  col = pow(col, f32x3(1.f / 2.6f));
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_cube<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
