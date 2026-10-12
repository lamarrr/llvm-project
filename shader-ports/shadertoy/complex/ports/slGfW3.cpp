// Port of https://www.shadertoy.com/view/slGfW3; author: jorge2017a2.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slGfW3.json.
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
float Speed = .03f;
f32x3 Light = f32x3(.6f, .2f, .8f);
float Map(Context &__ctx, f32x3 Position) {
  f32x3 P = (Position * 0.5f +
             texture(__ctx.channel0, Position * 2.f + iTime * Speed * .2f)
                     .swizzle<0, 1, 2>() *
                 .02f);
  float C = texture(__ctx.channel0, P).data[0];
  C *= texture(__ctx.channel0, P * f32x3(.5f, 1, .5f)).data[1];
  C = C * .9f + .1f * pow(texture(__ctx.channel0, P * 5.1f).data[3], 2.f);
  return max((C - .3f) * sqrt((Position.data[2] - .1f) / .3f), 0.f) / .5f;
}
f32x3 NubesXor(Context &__ctx, f32x2 Coord, float pz) {
  f32x4 Color;
  f32x3 R = f32x3(
      (Coord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1], 1.0f);
  f32x3 P = f32x3(0.f + sin(iTime * 0.5f), iTime * Speed, pz);
  f32x4 C = f32x4(0);
  for (float I = .2f; I < .5f; I += .01f) {
    float M1 = Map(__ctx, P + R * I);
    float M2 = Map(__ctx, P + R * I + Light * .01f);
    C += f32x4((.6f + f32x3(.6f, .5f, .4f) * (exp(-M2 * 10.f) - M1)), 1) * M1 *
         (1.f - C.data[3]);
    if (C.data[3] > .99f)
      break;
  }
  Color =
      C + f32x4(f32x3(.5f, .7f, .9f) - R.data[1] * .4f, 1) * (1.f - C.data[3]);
  return Color.swizzle<0, 1, 2>();
}
f32x3 DrawFigBorde(Context &__ctx, f32x3 pColObj, f32x3 colOut, float distObj) {
  colOut = mix(
      colOut, pColObj,
      smoothstep(8.0f * 1.5f / min(iResolution.data[1], iResolution.data[0]),
                 0.f, distObj - (0.0f)));
  colOut = mix(
      colOut, f32x3(0.0f),
      smoothstep(8.0f * 1.5f / min(iResolution.data[1], iResolution.data[0]),
                 0.f, abs(distObj) - 0.005f - (0.0f)));
  return colOut;
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
f32x3 Nave01(Context &__ctx, f32x2 p, f32x3 colOut) {
  f32x2 p0 = p;
  float d1 = sdBox(__ctx, p, f32x2(0.1f, 1.0f));
  float d2 = sdCircle(__ctx, p, 0.5f);
  p.data[1] = abs(p.data[1]) - 1.0f;
  float d3 = sdBox(__ctx, p, f32x2(1.0f, 0.1f));
  f32x3 col = DrawFigBorde(__ctx, f32x3(1.0f, 0.0f, 0.0f), colOut, d1);
  col = DrawFigBorde(__ctx, f32x3(1.0f, 0.0f, 0.0f), col, d2);
  col = DrawFigBorde(__ctx, f32x3(1.0f, 0.0f, 0.0f), col, d3);
  return col;
}
f32x3 Nave02(Context &__ctx, f32x2 p, f32x3 colOut) {
  f32x2 p0 = p;
  p.data[0] = abs(p.data[0]) - 1.0f;
  float d3 = sdBox(__ctx, p, f32x2(0.1f, 1.0f));
  p = p0;
  float d2 = sdCircle(__ctx, p, 0.5f);
  float d1 = sdBox(__ctx, p, f32x2(1.0f, 0.1f));
  f32x3 col = DrawFigBorde(__ctx, f32x3(1.0f, 0.0f, 0.0f), colOut, d1);
  col = DrawFigBorde(__ctx, f32x3(1.0f, 0.0f, 0.0f), col, d2);
  col = DrawFigBorde(__ctx, f32x3(1.0f, 0.0f, 0.0f), col, d3);
  return col;
}
f32x3 lineat(Context &__ctx, f32x2 p, f32x3 colOut) {
  f32x2 p0 = p;
  p.data[0] = mod(p.data[0] + 0.5f, 2.0f);
  float d1 = sdBox(__ctx, p, f32x2(1.0f, 0.1f));
  p = p0;
  p.data[1] = mod(p.data[1] + 0.5f, 2.0f);
  float d2 = sdBox(__ctx, p, f32x2(0.1f, 1.0f));
  f32x3 col = DrawFigBorde(__ctx, f32x3(0.0f, 0.0f, 0.0f), colOut, d1);
  col = DrawFigBorde(__ctx, f32x3(0.0f, 0.0f, 0.0f), col, d2);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      -1.0f + 2.0f * fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv -= f32x2(0.0f, -0.5f);
  float esc = 8.0f;
  uv *= esc;
  f32x2 uv0 = uv;
  f32x3 col = f32x3(1.0f);
  float t = iTime;
  col = NubesXor(__ctx, fragCoord, 1.0f + 1.0f * sin(t * 0.05f));
  col = lineat(__ctx, uv - f32x2(0.0f + sin(t * 0.5f), 1.0f + 2.0f * sin(t)),
               col);
  col = Nave01(__ctx, uv - f32x2(0.0f, 4.0f), col);
  col = Nave02(__ctx, uv - f32x2(3.0f, 2.0f), col);
  col = Nave02(__ctx, uv - f32x2(-3.0f, 2.0f), col);
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_3d<f32x4> channel0,
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
