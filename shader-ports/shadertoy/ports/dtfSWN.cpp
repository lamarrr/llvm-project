// Port of https://www.shadertoy.com/view/dtfSWN; author: QuantumSuper.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtfSWN.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float animTime;
f32x4 fft;
float sinRad2(Context &__ctx, float a, float r2) {
  return r2 * (1.f + .25f * fft.data[0] * sin(8.f * a + animTime));
}
float sinRing(Context &__ctx, float d2, float a, float r2, float w) {
  float r2Min = sinRad2(__ctx, a, r2);
  float r2Max = sinRad2(__ctx, a, r2 * w);
  if ((d2 >= r2Min - .001f) && (d2 <= r2Max + .001f))
    return 1.f;
  return .001f / abs(d2 - r2Min) + 0.001f / abs(d2 - r2Max);
}
float sinRing2(Context &__ctx, float d2, float a, float r2, float w) {
  return sinRing(__ctx, d2, a, r2, w) +
         sinRing(__ctx, d2, a + 3.14159265359f / 8.f, r2, w);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 col = f32x3(0);
  f32x2 uv = (2.f * fragCoord - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  animTime = 2.133333f * iTime;
  float a = atan(uv.data[0] / uv.data[1]);
  float d2 = uv.data[0] * uv.data[0] + uv.data[1] * uv.data[1];
  float p, s, amp, pc;
  fft.set_swizzle<0, 1, 2, 3>(f32x4(0));
  for (int n = 0; n < 12; n++)
    fft.data[0] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  for (int n = 12; n < 48; n++)
    fft.data[1] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  for (int n = 48; n < 192; n++)
    fft.data[2] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  for (int n = 192; n < 512; n++)
    fft.data[3] += texelFetch(__ctx.channel0, i32x2(n, 0)).data[0];
  fft /= f32x4(12, 36, 144, 320);
  for (float n = 0.f; n < 8.f; n++) {
    p = .01f * animTime + n * 3.14159265359f / 4.f;
    s = smoothstep(-.3f, 0.f, -cos(p));
    pc = a + p + animTime;
    amp =
        (.9f - .8f * s) * sinRing2(__ctx, d2, (-1.f + 2.f * mod(n, 2.f)) * a,
                                   .9f * (1.f + .999f * sin(p)), 1.f + .5f * s);
    col += amp * (.1f + .9f * fft.swizzle<2, 1, 3>()) *
           f32x3(sin(pc), sin(pc + 3.14159265359f / 3.f),
                 sin(pc + 2.f * 3.14159265359f / 3.f));
  }
  col *= col;
  col += f32x3(.001f / abs(d2 - .0015f) * (.1f + .9f * fft.data[0]));
  col = pow(col, f32x3(.4545f));
  fragColor = f32x4(col, 1.0f);
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
