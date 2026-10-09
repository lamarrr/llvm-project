// Port of https://www.shadertoy.com/view/DsXcWn; author: mrange.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DsXcWn.json.
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
f32x3 palette(Context &__ctx, float t) {
  return (1.0f + cos(f32x3(0.0f, 1.0f, 2.0f) + (2.0f * 3.141592654f) * t)) *
         0.5f;
}
float pmin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float pmax(Context &__ctx, float a, float b, float k) {
  return -pmin(__ctx, -a, -b, k);
}
float pabs(Context &__ctx, float a, float k) { return pmax(__ctx, a, -a, k); }
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
f32x2 toPolar(Context &__ctx, f32x2 p) {
  return f32x2(length(p), atan(p.data[1], p.data[0]));
}
f32x2 toRect(Context &__ctx, f32x2 p) {
  return f32x2(p.data[0] * cos(p.data[1]), p.data[0] * sin(p.data[1]));
}
float modMirror1(Context &__ctx, float &p, float size) {
  float halfsize = size * 0.5f;
  float c = floor((p + halfsize) / size);
  p = mod(p + halfsize, size) - halfsize;
  p *= mod(c, 2.0f) * 2.0f - 1.0f;
  return c;
}
float smoothKaleidoscope(Context &__ctx, f32x2 &p, float sm, float rep) {
  f32x2 hp = p;
  f32x2 hpp = toPolar(__ctx, hp);
  float rn = modMirror1(__ctx, hpp.data[1], (2.0f * 3.141592654f) / rep);
  float sa = 3.141592654f / rep -
             pabs(__ctx, 3.141592654f / rep - abs(hpp.data[1]), sm);
  hpp.data[1] = sign(hpp.data[1]) * (sa);
  hp = toRect(__ctx, hpp);
  p = hp;
  return rn;
}
f32x3 kishimisu(Context &__ctx, f32x3 col, f32x2 p) {
  f32x2 p0 = p;
  f32x3 finalColor = f32x3(0.0f);
  f32x2 p1 = p;
  for (float i = 0.0f; i < 4.0f; i++) {
    p1 = fract(p1 * 2.0f + 0.0125f * iTime) - 0.5f;
    float d = length(p1) * exp(-length(p0));
    f32x3 cc = palette(__ctx, length(p0) + i * .4f + iTime * .2f);
    d = sin(d * 8.f + iTime) / 8.f;
    d = abs(d);
    d = max(d, 0.005f);
    d = (0.0125f / d);
    d *= d;
    col += cc * d;
  }
  return col;
}
f32x3 effect(Context &__ctx, f32x2 p, f32x2 pp) {
  f32x3 col = f32x3(0.0f);
  f32x2x2 rot = f32x2x2(cos(0.025f * iTime), sin(0.025f * iTime),
                        -sin(0.025f * iTime), cos(0.025f * iTime));
  p *= rot;
  f32x2 kp = p;
  float r = iResolution.data[0] / iResolution.data[1];
  kp.data[0] -= r - 0.125f;
  float kl = dot(kp, kp);
  float kn = smoothKaleidoscope(__ctx, kp, 0.05f, 50.0f);
  kp += 0.5f * sin(f32x2(1.0f, sqrt(0.5f)) * iTime * 0.21f);
  kp /= r;
  col = kishimisu(__ctx, col, kp);
  col = clamp(col, 0.0f, 4.0f);
  f32x3 scol = palette(__ctx, 0.125f * iTime);
  col += 0.025f * scol * scol / max(length(kl), 0.001f);
  col -= .0033f * f32x3(1.0f, 2.0f, 3.0f).swizzle<2, 1, 0>() * kl;
  col = aces_approx(__ctx, col);
  col = max(col, 0.0f);
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
