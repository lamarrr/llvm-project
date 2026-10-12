// Port of https://www.shadertoy.com/view/ss2cRz; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ss2cRz.json.
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
  TextureCube channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float thc(Context &__ctx, float a, float b) {
  return tanh(a * cos(b)) / tanh(a);
}
float ths(Context &__ctx, float a, float b) {
  return tanh(a * sin(b)) / tanh(a);
}
f32x2 thc(Context &__ctx, float a, f32x2 b) {
  return tanh(a * cos(b)) / tanh(a);
}
f32x2 ths(Context &__ctx, float a, f32x2 b) {
  return tanh(a * sin(b)) / tanh(a);
}
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
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
float smin(Context &__ctx, float a, float b) {
  float k = 0.12f;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
f32x2x2 Rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 s) {
  p = abs(p) - s;
  return length(max(p, 0.f)) +
         min(max(p.data[0], max(p.data[1], p.data[2])), 0.f);
}
float GetDist(Context &__ctx, f32x3 p) {
  f32x2 uv = p.swizzle<0, 2>();
  uv *= Rot(__ctx,
            0.18f * p.data[1] + 0.18f * cos(1.5f * iTime + 2.f * p.data[1] +
                                            0.2f * dot(uv, uv)));
  uv = abs(uv) - 1.6f;
  float time = iTime + 2.f * p.data[1];
  float sc = 2.f;
  float r = 0.05f;
  float sp = 1.f;
  float n = 7.f;
  float d = 100.f;
  for (float i = 0.f; i < n; i++) {
    float e = (i / n) * 3.14159f;
    f32x2 p = 0.5f * cos(time + e + 0.25f * 3.14159f * cos(iTime + 2.f * e)) *
              f32x2(cos(e), sin(e));
    d = smin(__ctx, d,
             length(uv - p) - 0.1f - 0.05f * cos(iTime + 10.f * p.data[1] + e));
  }
  return 0.7f * d;
}
float RayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float dO = 0.f;
  for (int i = 0; i < 400; i++) {
    f32x3 p = ro + rd * dO;
    float dS = GetDist(__ctx, p);
    dO += dS;
    if (dO > 20.f || abs(dS) < .001f)
      break;
  }
  return dO;
}
f32x3 GetNormal(Context &__ctx, f32x3 p) {
  float d = GetDist(__ctx, p);
  f32x2 e = f32x2(.001f, 0);
  f32x3 n = d - f32x3(GetDist(__ctx, p - e.swizzle<0, 1, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 0, 1>()),
                      GetDist(__ctx, p - e.swizzle<1, 1, 0>()));
  return normalize(n);
}
f32x3 GetRayDir(Context &__ctx, f32x2 uv, f32x3 p, f32x3 l, float z) {
  f32x3 f = normalize(l - p), r = normalize(cross(f32x3(0, 1, 0), f)),
        u = cross(f, r), c = f * z, i = c + uv.data[0] * r + uv.data[1] * u,
        d = normalize(i);
  return d;
}
float GetLight(Context &__ctx, f32x3 p, f32x3 lightPos) {
  f32x3 l = normalize(lightPos - p);
  f32x3 n = GetNormal(__ctx, p);
  float dif = clamp(dot(n, l), 0.f, 1.f);
  float d = RayMarch(__ctx, p + n * .001f * 2.f, l);
  dif = 0.2f + 0.8f * dif * step(length(lightPos - p), d);
  return dif;
}
f32x3 Bg(Context &__ctx, f32x3 rd) {
  float k = 0.5f + 0.5f * rd.data[1];
  f32x3 e = f32x3(1.f);
  f32x3 col = pal(__ctx, 0.9f * rd.data[1] - 0.08f, f32x3(k), e, f32x3(k),
                  1.15f * f32x3(0.f, 0.33f, 0.66f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float l = 0.5f + 0.5f * thc(__ctx, 3.f, 0.5f * iTime);
  float r = 5.f;
  float time = 0.25f * iTime;
  f32x3 ro = f32x3(r * cos(time), 0.f * iTime, r * sin(time));
  f32x3 rd = GetRayDir(__ctx, uv, ro, f32x3(0), 1.2f);
  f32x3 col = f32x3(0);
  float d = RayMarch(__ctx, ro, rd);
  if (d < 20.f) {
    f32x3 p = ro + rd * d;
    f32x3 n = GetNormal(__ctx, p);
    f32x3 r = reflect(rd, n);
    float ambient = .3f;
    float difPower = .4f;
    float k = 0.5f;
    col = f32x3(smoothstep(-k, k, r.data[1] - n.data[1]));
    f32x3 e = f32x3(1.f);
    col *= pal(__ctx, r.data[1] * 0.25f, col, e, col,
               1.15f * f32x3(0.f, 0.33f, 0.66f));
    col *= 0.9f + 0.1f * h21(__ctx, p.swizzle<0, 2>());
  } else {
  }
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
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
