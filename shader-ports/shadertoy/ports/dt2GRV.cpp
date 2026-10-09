// Port of https://www.shadertoy.com/view/dt2GRV; author: ersh.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dt2GRV.json.
#include "../support.h"
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
float nonzero(Context &__ctx, float x, float minval) {
  return x >= 0.f ? x + exp(-x) * minval : x - exp(x) * minval;
}
float nonzero(Context &__ctx, float x) { return nonzero(__ctx, x, 1.f); }
float powX(Context &__ctx, float x, float f) {
  return x > 0.f ? pow(x, f) : x < 0.f ? -pow(-x, f) : 0.f;
}
f32x2x2 rotate2d(Context &__ctx, float a) {
  return f32x2x2(cos(a), -sin(a), sin(a), cos(a));
}
float calc(Context &__ctx, f32x2 fragCoord, float size, float angle, float kx,
           float ky, f32x2 offset) {
  f32x2 stretch = fragCoord - iResolution.swizzle<0, 1>() * 1.5f / 2.0f *
                                  sin(iTime / f32x2(11.f, 19.f));
  f32x2 p =
      fragCoord.swizzle<0, 1>() *
      rotate2d(__ctx, angle + kx * stretch.data[0] - ky * stretch.data[1]);
  f32x2 pmod = (min(fract((p + offset) / (size * 2.0f)) / (1.f - (1.f / size)),
                    fract(-(p + offset) / (size * 2.0f)) / (1.f / size)) *
                (size * 2.0f));
  return max(pmod.data[0], pmod.data[1]) / size / 2.f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float size = iResolution.data[0] /
               (25.0f + powX(__ctx, sin(iTime / 5.5f), 9.f) * 10.f);
  float angle = iTime / 10.0f;
  float kx = sin(iTime / 1.f) /
             nonzero(__ctx,
                     iResolution.data[0] * 1.3f *
                         (1.f - powX(__ctx, sin(iTime / 15.f), 5.f)),
                     0.01f);
  float ky = powX(__ctx, sin(iTime / 1.4f), 3.f) /
             nonzero(__ctx,
                     iResolution.data[0] * 1.3f *
                         (1.f - powX(__ctx, sin(iTime / 22.f), 7.f)),
                     0.01f);
  f32x2 offset = iResolution.swizzle<0, 1>() * cos(iTime / 27.f) *
                 f32x2(cos(iTime / 15.f), -sin(iTime / 11.f));
  f32x2 offsetC = iResolution.swizzle<0, 1>() * sin(iTime / 37.f) *
                  f32x2(sin(iTime / 31.f), cos(iTime / 25.f)) / 5.f;
  fragCoord =
      fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>() / 2.0f + offsetC;
  float k = calc(__ctx, fragCoord, size, angle, kx, ky, offset);
  fragColor = f32x4(f32x3(k), 1.0f);
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
