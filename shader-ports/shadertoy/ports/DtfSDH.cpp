// Port of https://www.shadertoy.com/view/DtfSDH; author: Spheroidon.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtfSDH.json.
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
float audio_freq(Context &__ctx, sampler2D channel, float f) {
  return texture(channel, f32x2(f, 0.25f)).data[0];
}
float audio_ampl(Context &__ctx, sampler2D channel, float t) {
  return texture(channel, f32x2(t, 0.75f)).data[0];
}
f32x3 B2_spline(Context &__ctx, f32x3 x) {
  f32x3 t = 3.0f * x;
  f32x3 b0 = step(0.0f, t) * step(0.0f, 1.0f - t);
  f32x3 b1 = step(0.0f, t - 1.0f) * step(0.0f, 2.0f - t);
  f32x3 b2 = step(0.0f, t - 2.0f) * step(0.0f, 3.0f - t);
  return 0.5f * (b0 * pow(t, f32x3(2.0f)) +
                 b1 * (-2.0f * pow(t, f32x3(2.0f)) + 6.0f * t - 3.0f) +
                 b2 * pow(3.0f - t, f32x3(2.0f)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 centered = 2.0f * uv - 1.0f;
  centered.data[0] *= iResolution.data[0] / iResolution.data[1];
  float dist2 = dot(centered, centered);
  float clamped_dist = smoothstep(0.0f, 1.0f, dist2);
  float arclength =
      abs(atan(centered.data[1], centered.data[0]) / radians(360.0f)) + 0.01f;
  float t = iTime / 100.0f;
  float polychrome = (1.0f + sin(t * 10.0f)) / 2.0f;
  f32x3 spline_args = fract(f32x3(polychrome * uv.data[0] - t) +
                            f32x3(0.0f, -1.0f / 3.0f, -2.0f / 3.0f));
  f32x3 spline = B2_spline(__ctx, spline_args);
  float f = abs(centered.data[1]);
  f32x3 base_color = f32x3(1.0f, 1.0f, 1.0f) - f * spline;
  f32x3 flame_color = pow(base_color, f32x3(3.0f));
  f32x3 disc_color = 0.20f * base_color;
  f32x3 wave_color = 0.10f * base_color;
  f32x3 flash_color = 0.05f * base_color;
  float sample1 =
      audio_freq(__ctx, __ctx.channel0, abs((uv.data[0] - .5f) / 1.0f) + 0.01f);
  float sample2 = audio_ampl(__ctx, __ctx.channel0, clamped_dist);
  float sample3 = audio_ampl(__ctx, __ctx.channel0, arclength);
  float disp_dist = smoothstep(-0.2f, -0.1f, sample3 - dist2);
  disp_dist *= (1.0f - disp_dist);
  f32x3 color = f32x3(0.0f);
  float v = abs(uv.data[1] - 0.5f);
  color += flame_color * smoothstep(v, v * 8.0f, sample1);
  color += disc_color * smoothstep(0.5f, 1.0f, sample2) * (1.0f - clamped_dist);
  color += flash_color * smoothstep(0.5f, 1.0f, sample3) * clamped_dist;
  color += wave_color * disp_dist;
  color = pow(color, f32x3(0.4545f));
  fragColor = f32x4(color, 1.0f);
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
