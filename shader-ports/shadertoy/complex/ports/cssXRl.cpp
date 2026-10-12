// Port of https://www.shadertoy.com/view/cssXRl; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cssXRl.json.
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
float seg(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - ba * h);
}
float dist(Context &__ctx, f32x2 ipos, f32x2 fpos, f32x2 uv, float io,
           float t) {
  float r = 0.25f + 0.25f * cos(4.f * length(uv) - t);
  f32x2 p = r * f32x2(cos(io + t), sin(io + 2.f * t));
  float c = 0.5f + 0.5f * cos(10.f * length(uv) - 2.f * t);
  return seg(__ctx, fpos, c * uv, c * p);
}
float dist2(Context &__ctx, f32x2 ipos, f32x2 fpos, f32x2 uv, float io,
            float t) {
  float d = 1e5f;
  for (float i = -1.f; i <= 1.f; i++)
    for (float j = -1.f; j <= 1.f; j++)
      d = min(d, dist(__ctx, ipos, fpos - f32x2(i, j), uv, io, t));
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 res = iResolution.swizzle<0, 1>();
  f32x2 ouv = (fragCoord - 0.5f * res) / res.data[1];
  float t = iTime;
  float th = tanh(0.25f * t);
  float s = 0.f;
  float n = 40.f;
  for (float i = n - 1.f; i >= 0.f; i--) {
    float io = 2.f * 3.14159f * i / n;
    float xp = 1.f - exp(-0.12f * i);
    float c = 0.5f + 0.5f * cos(io - t);
    f32x2 uv1 = ouv * f32x2x2(cos(0.2f * t + io), -sin(0.2f * t + io),
                              sin(0.2f * t + io), cos(0.2f * t + io));
    f32x2 uv2 =
        ouv * f32x2x2(cos(0.4f * t + io * 10.f), -sin(0.4f * t + io * 10.f),
                      sin(0.4f * t + io * 10.f), cos(0.4f * t + io * 10.f));
    float c2 = 0.5f - 0.5f * tanh(3.f * cos(0.1f * io - 0.25f * t)) / tanh(3.f);
    f32x2 uv = (1.f + 0.2f * c * i) * mix(uv1, uv2, c2);
    float sc = 1.f + 0.4f * sqrt(i);
    f32x2 ipos = floor(sc * uv) + 0.5f;
    f32x2 fpos = sc * uv - ipos;
    float d = dist2(__ctx, ipos, fpos, uv, io, t);
    float r = 0.13f * (0.75f + 0.25f * cos(io + t));
    float k = (1.f + 3.f * xp) * sc / res.data[1];
    float r2 = 0.05f * cos(10.f * d + uv.data[0] * (20.f + 3.f * i));
    float s2 = smoothstep(-k, k, -abs(-d + r) + r2);
    s *= 1.f - s2;
    float fog = exp(0.02f - 0.5f * dot(uv, uv));
    s = mix(s, fog, (0.5f - 0.4f * th) * xp);
  }
  s = clamp(s, 0.f, 1.f);
  f32x3 col = f32x3(s);
  col *= .5f + .5f * cos(2.f * 3.14159f *
                         (0.15f * length(ouv) + 0.5f * f32x3(0, 1, 2) / 3.f));
  col = pow(col, f32x3(1.f / 2.2f));
  col *= step(abs(ouv.data[0]), 0.52f * res.data[0] / res.data[1] * th);
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
