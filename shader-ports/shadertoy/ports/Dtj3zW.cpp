// Port of https://www.shadertoy.com/view/Dtj3zW; author: kishimisu.
// Dataset license metadata: cc-by-nc-sa-4.0; original retained in
// upstream/Dtj3zW.json.
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
float logisticAmp(Context &__ctx, float amp) {
  float c = .88f, a = 20.f;
  return ((1.f / (exp(-a * (amp - c)) + 1.f)) -
          (1.f / (exp(-a * (0.0f - c)) + 1.f))) /
         ((1.f / (exp(-a * (1.0f - c)) + 1.f)) -
          (1.f / (exp(-a * (0.0f - c)) + 1.f)));
}
float getPitch(Context &__ctx, float freq, float octave) {
  freq = pow(2.f, freq) * 261.f;
  freq = pow(2.f, octave) * freq / 12000.f;
  return logisticAmp(
      __ctx, (texelFetch(__ctx.channel0, i32x2(int(freq * 512.f), 0)).data[0]));
}
float getVol(Context &__ctx, float samples) {
  float avg = 0.f;
  for (float i = 0.f; i < samples; i++)
    avg += (texelFetch(__ctx.channel0, i32x2(int(i / samples * 512.f), 0))
                .data[0]);
  return avg / samples;
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float hash13(Context &__ctx, f32x3 p3) {
  p3 = fract(p3 * .1031f);
  p3 += dot(p3, p3.swizzle<2, 1, 0>() + 31.32f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(.1f, .0f, .14f);
  float vol = getVol(__ctx, 8.f);
  f32x3 ro = f32x3(0, 8, 12) * (1.f + vol * .3f);
  ro.set_swizzle<2, 0>(ro.swizzle<2, 0>() *
                       (f32x2x2(cos(iTime * .4f + f32x4(0, 33, 11, 0)))));
  f32x3 f = normalize(-ro), r = normalize(cross(f32x3(0, 1, 0), f));
  f32x3 rd = normalize(f + uv.data[0] * r + uv.data[1] * cross(f, r));
  float hasSound = 1.f;
  if (iChannelTime[0] <= 0.f)
    hasSound = .4f;
  for (float i = 0.f, t = 0.f; i < 30.f; i++) {
    f32x3 p = ro + t * rd;
    f32x2 cen = floor(p.swizzle<0, 2>()) + .5f;
    f32x3 id = abs(f32x3(cen.data[0], 0, cen.data[1]));
    float d = length(id);
    float freq =
        smoothstep(0.f, 20.f, d) * 3.f * hasSound + hash13(__ctx, id) * 2.f;
    float pitch = getPitch(__ctx, freq, .7f);
    float v = vol * smoothstep(2.f, 0.f, d);
    float h = d * .2f * (1.f + pitch * 1.5f) + v * 2.f;
    float me = sdBox(__ctx, p - f32x3(cen.data[0], -50.f, cen.data[1]),
                     f32x3(.3f, 50.f + h, .3f) + pitch) -
               .05f;
    col += mix(mix(f32x3(.8f, .2f, .4f), f32x3(0, 1, 0), min(v * 2.f, 1.f)),
               f32x3(.5f, .3f, 1.2f), smoothstep(10.f, 30.f, d)) *
           (cos(id) + 1.5f) * (pitch * d * .08f + v) * 1.f /
           (1.f + pow(abs(me * 20.f), 1.3f)) * (1.f + vol * 2.f);
    t += me;
  }
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
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
