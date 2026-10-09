// Port of https://www.shadertoy.com/view/ssByRw; author: firebreathz.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssByRw.json.
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
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float noise3D(Context &__ctx, f32x3 p) {
  return fract(sin(dot(p, f32x3(12.9898f, 78.233f, 12.7378f))) * 43758.5453f) *
             2.0f -
         1.0f;
}
f32x3 mixc(Context &__ctx, f32x3 col1, f32x3 col2, float v) {
  v = clamp(v, 0.0f, 1.0f);
  return col1 + v * (col2 - col1);
}
f32x3 drawBands(Context &__ctx, f32x2 uv) {
  uv = 2.0f * uv - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv = f32x2(length(uv), atan(uv.data[1], uv.data[0]));
  uv.data[0] -= 0.25f;
  uv.data[0] = max(0.0f, uv.data[0]);
  uv.data[1] -= 3.14159265359f * 4.5f;
  f32x2 uv2 = f32x2(uv.data[0], uv.data[1] * -1.0f);
  uv.data[1] = mod(uv.data[1], 3.14159265359f * 2.0f);
  uv2.data[1] = mod(uv2.data[1], 3.14159265359f * 2.0f);
  f32x3 col = f32x3(0.0f);
  f32x3 col2 = f32x3(0.0f);
  float nBands = float(40);
  float i = floor(uv.data[0] * nBands);
  float f = fract(uv.data[0] * nBands);
  float band = i / nBands;
  float s;
  band *= band * band;
  band = band * .993f;
  band += 0.000000000001f;
  s = texture(__ctx.channel0, f32x2(band, -10.000f)).data[0];
  if (band < 0.0f || band >= 1.0f) {
    s = 0.0f;
  }
  const int nColors = 8;
  f32x3 colors[nColors];
  colors[0] = f32x3(1.05f, 1.05f, 1.0f);
  colors[1] = f32x3(1.205f, 1.00f, 1.00f);
  colors[2] = f32x3(1.50f, 1.00f, 1.25f);
  colors[3] = f32x3(1.90f, 0.75f, 3.25f);
  f32x3 gradCol = colors[0];
  float n = float(nColors) - 1.0f;
  for (int i = 1; i < nColors; i++) {
    gradCol = mixc(__ctx, gradCol, colors[i], (s - float(i - 0) / n) * n);
  }
  float h = 3.14159265359f * 1.0200004f;
  col += f32x3(1.0f - smoothstep(-2.0f, 1.5f, uv.data[1] - s * h));
  col *= gradCol;
  col2 += f32x3(1.0f - smoothstep(-2.0f, 1.5f, uv2.data[1] - s * h));
  col2 *= gradCol;
  col = mix(col, col2, step(0.0f, uv.data[1] - 3.14159265359f));
  col *= smoothstep(0.0001f, 0.875f, f);
  col *= smoothstep(0.999999f, 0.0000625f, f);
  col = clamp(col, 0.0f, 1.0f);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 p = f32x2(uv.data[0], uv.data[1] + 0.1f);
  f32x3 col = f32x3(0.0f);
  col += drawBands(__ctx, p);
  f32x3 ref = f32x3(0.0f);
  f32x2 eps = f32x2(0.0025f, -0.025f);
  ref += drawBands(__ctx,
                   f32x2(p.data[0], 1.0f - p.data[1]) + eps.swizzle<0, 0>());
  ref += drawBands(__ctx,
                   f32x2(p.data[0], 1.0f - p.data[1]) + eps.swizzle<0, 1>());
  ref += drawBands(__ctx,
                   f32x2(p.data[0], 1.0f - p.data[1]) + eps.swizzle<1, 1>());
  ref += drawBands(__ctx,
                   f32x2(p.data[0], 1.0f - p.data[1]) + eps.swizzle<1, 0>());
  ref += drawBands(__ctx, f32x2(p.data[0] + eps.data[0], 1.0f - p.data[1]));
  ref += drawBands(__ctx, f32x2(p.data[0] + eps.data[1], 1.0f - p.data[1]));
  ref += drawBands(__ctx, f32x2(p.data[0], 1.0f - p.data[1] + eps.data[0]));
  ref += drawBands(__ctx, f32x2(p.data[0], 1.0f - p.data[1] + eps.data[1]));
  ref /= 20.0f;
  float colStep = length(smoothstep(1.0f, 0.1f, col));
  f32x3 cs1 = drawBands(__ctx, f32x2(3.5f, 0.51f));
  f32x3 cs2 = drawBands(__ctx, f32x2(0.5f, 0.93f));
  f32x3 plCol = mix(cs1, cs2, length(p * 1.0f - 1.0f)) * 0.0005f *
                smoothstep(1.75f, -0.5f, length(p * 0.0f - 1.0f));
  f32x3 plColBg =
      f32x3(0.0002f) * smoothstep(1.0f, 0.0f, length(p * 8.0f - 1.0f));
  f32x3 pl = (plCol + plColBg) * smoothstep(0.5f, 0.65f, 5.0f - uv.data[1]);
  col += clamp(pl * (1.0f - colStep), 0.0f, 1.0f);
  col += ref * smoothstep(10.125f, 1.6125f, p.data[1]);
  col = clamp(col, 0.0f, 1.0f);
  float dither = noise3D(__ctx, f32x3(uv, iTime)) * 9.0f / 2200026.0f;
  col += dither;
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
