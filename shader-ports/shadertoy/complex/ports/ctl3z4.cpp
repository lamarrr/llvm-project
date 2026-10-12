// Port of https://www.shadertoy.com/view/ctl3z4; author: kishimisu.
// Dataset license metadata: cc-by-nc-sa-4.0; original retained in
// upstream/ctl3z4.json.
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
float logisticAmp(Context &__ctx, float amp) {
  const float c = 0.8f, a = 20.f;
  return ((1.f / (exp(-a * (amp - c)) + 1.f)) -
          (1.f / (exp(-a * (0.0f - c)) + 1.f))) /
         ((1.f / (exp(-a * (1.0f - c)) + 1.f)) -
          (1.f / (exp(-a * (0.0f - c)) + 1.f)));
}
float getPitch(Context &__ctx, float smin, float smax, float freq,
               float octave) {
  freq = pow(2.f, freq) * 261.f;
  freq = pow(2.f, octave) * freq / 12000.f;
  return smoothstep(
      smin, smax,
      logisticAmp(
          __ctx,
          (texelFetch(__ctx.channel0, i32x2(int(freq * 512.f), 0)).data[0])));
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
float map(Context &__ctx, f32x3 p, f32x3 &lgt, float v, float fbeat,
          float fbeat2, float fbass, float fhigh, float fball) {
  float d = length(p - f32x3(0, 0, 8.f + 2.f * sin(iTime))) - .5f - v * .8f;
  lgt += mix(f32x3(.1f, .8f, .6f), f32x3(.2f, .6f, 1.f), fbeat2) * 1.f /
         (1.f + pow(abs(d * 10.f), 1.3f)) * fball *
         smoothstep(19.f, 20.f, iTime);
  f32x3 q = p;
  f32x3 id = floor(p);
  q.set_swizzle<0, 2>(fract(p.swizzle<0, 2>()) - .5f);
  q.data[1] = abs(q.data[1]);
  float h = .3f * sin(id.data[0] + id.data[2] + iTime) +
            max(0.f, (length(id) - 1.f)) * .05f;
  float d2 = sdBox(__ctx, q - f32x3(0, 3, 0), f32x3(.1f, 1.5f + v + h, .1f));
  d = min(d, d2);
  d2 = sdBox(__ctx, p - f32x3(0, 0, 22), f32x3(18.f, 18.f, 10));
  d = min(d, d2);
  lgt += f32x3(2.f, .1f, .0f) * 1.f / (1.f + pow(abs(d2 * 20.f), 1.3f)) * fbeat;
  float g = -sdBox(__ctx, p, f32x3(14.f, 8.f, 14));
  lgt += mix(f32x3(.55f, .55f, 1.f), f32x3(.2f, .8f, .8f), fbass) * 1.f /
         (1.f + pow(abs(g * 10.f), 1.3f)) * (fbass + fhigh * 2.5f);
  d = min(d, g);
  return d;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col;
  if (abs(uv.data[0]) > .9f || abs(uv.data[1]) > .9f) {
    fragColor = f32x4(0.f);
    return;
  }
  float vol = getVol(__ctx, 8.f),
        fbeat =
            getPitch(__ctx, mix(.05f, 0.f, smoothstep(17.f, 20.f, iTime)),
                     mix(.8f, .1f, smoothstep(17.f, 20.f, iTime)), 1.215f, 1.f),
        fbeat2 = getPitch(__ctx, 0.f, .8f, -.8f, 1.f),
        fbass = getPitch(__ctx, 0.f, .8f, -.615f, 1.f),
        fhigh = getPitch(__ctx, 0.f, .8f, 2.26f, 1.f),
        fball = getPitch(__ctx, 0.f, .8f, -1.f, 1.f);
  if (iChannelTime[0] <= 0.f)
    fbeat = fhigh = fball = .5f;
  for (float i = 0.f, t = 0.f; i < 50.f; i++) {
    f32x3 p = t * normalize(f32x3(uv, 1.f));
    t += map(__ctx, p, col, vol, fbeat, fbeat2, fbass, fhigh, fball);
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
