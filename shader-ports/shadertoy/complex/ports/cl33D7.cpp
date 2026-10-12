// Port of https://www.shadertoy.com/view/cl33D7; author: becanchi.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cl33D7.json.
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
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const int iters = 150;
int fractal(Context &__ctx, f32x2 p, f32x2 point) {
  f32x2 so = (-1.0f + 2.0f * point) * 0.3f;
  f32x2 seed = f32x2(0.098386255f + so.data[0], 0.6387662f + so.data[1]);
  for (int i = 0; i < iters; i++) {
    if (length(p) > 2.0f) {
      return i;
    }
    f32x2 r = p;
    p = f32x2(p.data[0] * p.data[0] - p.data[1] * p.data[1],
              2.0f * p.data[0] * p.data[1]);
    p = f32x2(p.data[0] * r.data[0] - p.data[1] * r.data[1] + seed.data[0],
              r.data[0] * p.data[1] + p.data[0] * r.data[1] + seed.data[1]);
  }
  return 0;
}
f32x3 color(Context &__ctx, int i) {
  float f = float(i) / float(iters) * 2.0f;
  f = f * f * 2.f;
  return f32x3((sin(f * 2.0f)), (sin(f * 3.0f)), abs(sin(f * 7.0f)));
}
float sampleMusicA(Context &__ctx) {
  return 0.6f * (texture(__ctx.channel1, f32x2(0.15f, 0.25f)).data[0] +
                 texture(__ctx.channel1, f32x2(0.30f, 0.25f)).data[0]);
}
f32x2x2 rotateX(Context &__ctx, float rad) {
  float c = cos(rad);
  float s = sin(rad);
  return f32x2x2(c, -s, s, c);
}
f32x4 fractalImg(Context &__ctx, f32x2 uv, f32x2 fragCoord) {
  f32x2 mouse = f32x2(iMouse.data[0] / iResolution.data[0],
                      iMouse.data[1] / iResolution.data[1]);
  f32x2 position =
      3.f * (-0.5f + fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>()) +
      f32x2(0.2f * abs(cos(iTime * 0.21313f)) *
                sin(sin(iTime) + cos(iTime) * .242f),
            0.1f * abs(cos(iTime * 0.1323f)));
  position.data[0] *= iResolution.data[0] / iResolution.data[1];
  position *=
      rotateX(__ctx, mod(iTime * .54f, 6.3f) + abs(.2f * sin(iTime / 5.3f)));
  f32x2 iFC = f32x2(iResolution.data[0] - fragCoord.data[0],
                    iResolution.data[1] - fragCoord.data[1]);
  f32x2 pos2 =
      2.f * (-0.5f + iFC.swizzle<0, 1>() / iResolution.swizzle<0, 1>());
  pos2.data[0] *= iResolution.data[0] / iResolution.data[1];
  pos2 *=
      rotateX(__ctx, mod(iTime * -.4235f, 6.3f) + abs(.2f * sin(iTime / 5.3f)));
  f32x4 t3 = texture(__ctx.channel0, f32x2(length(position) / 14.0f, 0.1f));
  float pulse = .2f + sampleMusicA(__ctx) * 1.8f;
  f32x3 invFract = color(
      __ctx,
      fractal(__ctx, pos2,
              f32x2(0.55f + sin(iTime / 18.f + 0.5f) / 2.0f, pulse * .9f)));
  f32x3 fract4 = color(
      __ctx,
      fractal(__ctx, position / 1.6f,
              f32x2(0.6f + cos(iTime / 20.f + 0.5f) / 2.0f, pulse * .8f)));
  f32x3 c =
      color(__ctx, fractal(__ctx, position,
                           f32x2(10.1f + sin(iTime / 3.f) / 12.0f, pulse)));
  t3 = abs(f32x4(0.5f, 0.1f, 0.5f, 1.f) - t3) * .8f;
  f32x4 fract01 = f32x4(c, 1.0f);
  f32x4 salida;
  salida = fract01 / t3 + fract01 * t3 + f32x4(invFract, 0.6f) +
           f32x4(fract4, 0.25f);
  return mix(salida * 2.f,
             texture(__ctx.channel1,
                     fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>()),
             0.3f);
}
const float sensitivity = 1.0f / 100.0f;
float bw(Context &__ctx, f32x2 coords) {
  f32x4 lm = texture(__ctx.channel0, coords) * f32x4(0.21f, 0.71f, 0.07f, 1);
  return lm.data[0] + lm.data[1] + lm.data[2];
}
float outline(Context &__ctx, f32x2 uv) {
  f32x2 of = f32x2(sensitivity * pow(sampleMusicA(__ctx), 2.f), 0);
  return sqrt(
      pow(abs(bw(__ctx, uv) - bw(__ctx, uv + of.swizzle<0, 0>())), 2.0f) +
      pow(abs(bw(__ctx, uv + of.swizzle<0, 1>()) -
              bw(__ctx, uv + of.swizzle<1, 0>())),
          2.0f));
}
f32x2 distort(Context &__ctx, f32x2 uv) {
  bool autoScale = true;
  float strength = sampleMusicA(__ctx) * sign(sin(iTime / 5.f)) * 0.25f;
  float zoom = (autoScale ? abs(strength) + 1.0f : 1.0f);
  f32x2 p = (uv - 0.5f) * 2.0f;
  float theta = atan(p.data[1], p.data[0]);
  float rd = length(p);
  float ru = rd * (1.0f + strength * rd * rd) / zoom;
  return f32x2(cos(theta), sin(theta)) * ru / 2.0f + 0.5f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv.data[0] = 1.0f - uv.data[0];
  f32x4 imgColor = texture(__ctx.channel0, uv);
  uv = distort(__ctx, uv);
  float bwColor = outline(__ctx, uv) * 2.f;
  bwColor = min(round(bwColor * 8.f), 2.f) * bwColor;
  fragColor = imgColor + bwColor * (fractalImg(__ctx, uv, fragCoord) +
                                    fractalImg(__ctx, -uv, fragCoord));
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_2d<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
