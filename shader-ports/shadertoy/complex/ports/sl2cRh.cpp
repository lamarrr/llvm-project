// Port of https://www.shadertoy.com/view/sl2cRh; author: Karp.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sl2cRh.json.
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
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
f32x2 CenterUV(Context &__ctx, f32x2 fragCoord) {
  f32x2 uv =
      f32x2(fragCoord.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>());
  return uv;
}
f32x2 NormalizeUV(Context &__ctx, f32x2 fragCoord) {
  f32x2 uv = CenterUV(__ctx, fragCoord.swizzle<0, 1>());
  uv = 2.0f * uv.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  return uv;
}
f32x2 AspectRatioUV(Context &__ctx, f32x2 fragCoord) {
  f32x2 uv = CenterUV(__ctx, fragCoord.swizzle<0, 1>());
  uv = 2.0f * uv.swizzle<0, 1>() / iResolution.data[1];
  return uv;
}
f32x2 RotateUV(Context &__ctx, f32x2 uv, float angle) {
  angle = angle * 3.14159265359f / 180.0f;
  uv *f32x2x2(cos(angle), sin(angle), -sin(angle), cos(angle));
  return uv;
}
f32x3 Color255(Context &__ctx, int R, int G, int B) {
  float rat = 1.0f / 255.0f;
  return f32x3(float(R) * rat, float(G) * rat, float(B) * rat);
}
f32x3 HSVcolor(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
float Clamp01(Context &__ctx, float value) { return clamp(value, 0.0f, 1.0f); }
float LineHori(Context &__ctx, f32x2 uv, float height, float thickness,
               float blur) {
  float sdfLine = abs(uv.data[1] - height) - thickness;
  return Clamp01(__ctx, sdfLine * (10.0f / blur));
}
float LineVert(Context &__ctx, f32x2 uv, float height, float thickness,
               float blur) {
  float sdfLine = abs(uv.data[1] - height) - thickness;
  return Clamp01(__ctx, sdfLine * (10.0f / blur));
}
float Disk(Context &__ctx, f32x2 uv, f32x2 center, float radius, float blur) {
  float sdfCircle = length(uv - center) - radius;
  return Clamp01(__ctx, sdfCircle * (10.0f / blur));
}
float Rectangle(Context &__ctx, f32x2 uv, f32x2 center, f32x2 size) {
  float ret;
  float d = 0.005f;
  ret = smoothstep((center.data[0] + (-size.data[0] * .5f)) - d,
                   (center.data[0] + (size.data[0] * .5f)) + d, uv.data[0]);
  ret *= smoothstep((center.data[1] + (size.data[1] * .5f)) - d,
                    (center.data[1] + (-size.data[1] * .5f)) + d, uv.data[1]);
  ret *= 1.0f - smoothstep((center.data[1] + (-size.data[1] * .5f)) - d,
                           (center.data[1] + (size.data[1] * .5f)) + d,
                           uv.data[1]);
  ret *= 1.0f - smoothstep((center.data[0] + (size.data[0] * .5f)) - d,
                           (center.data[0] + (-size.data[0] * .5f)) + d,
                           uv.data[0]);
  return ret;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = AspectRatioUV(__ctx, fragCoord.swizzle<0, 1>());
  uv *= 1.2f;
  float t = iTime * 2.0f;
  f32x3 pix = f32x3(uv, 0);
  pix = f32x3(0.1f);
  for (int i = 0; i < 960; i++) {
    float t =
        (iTime * 1.0f) + abs(sin(float(i) / 9600.0f * 6.28318530717f + iTime));
    f32x2 pos = f32x2(cos(float(i) / 32.0f * 6.28318530717f + t),
                      sin(float(i) / 32.0f * 6.28318530717f + t)) *
                    sin(float(i) / 8.0f * 6.28318530717f + t) +
                f32x2(0.0f, 0.0f);
    float circleMask = Disk(__ctx, uv, pos, 0.075f, 0.25f);
    pix = mix(HSVcolor(__ctx, f32x3(float(i) / 1024.0f, 0.95f, 0.95f)), pix,
              circleMask);
  }
  fragColor = f32x4(pix, 1.0f);
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
