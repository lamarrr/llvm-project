// Port of https://www.shadertoy.com/view/DtjGRh; author: foxes.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtjGRh.json.
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
float sinc(Context &__ctx, float x) { return abs(x) > 0.f ? sin(x) / x : 1.f; }
float FX(Context &__ctx, float t) {
  float freq = 22000.f * 2.f / 2000.f;
  float wav = 0.f;
  if (iMouse.data[2] <= 0.0f) {
    wav += (t > 0.f && t < 1.f)
               ? (sin((sqrt(sqrt(t)) * 2.f - 0.5f) * 3.1415f) * 0.5f + 0.5f) *
                     sin(t * freq * 3.1415f)
               : 0.f;
    t -= 0.925f;
    wav += (t > 0.f && t < 1.f)
               ? (sin((sqrt(t) * 2.f - 0.5f) * 3.1415f) * 0.5f + 0.5f) *
                     sin(t * freq * 3.1415f)
               : 0.f;
  } else {
    wav += sin(t * freq * 3.1415f);
  }
  return wav;
}
float restore(Context &__ctx, float t, float ofs, float samp) {
  float val = 0.f;
  for (float i = -50.f; i < 50.f; i += 1.f) {
    float dto = (t + ofs) * samp + 0.5f;
    float dt = floor(dto) + i;
    float x = 3.1415f * ((dto - dt) - 0.5f);
    dt = dt / samp;
    val += FX(__ctx, dt - ofs) * sinc(__ctx, x);
  }
  return val;
}
float discret(Context &__ctx, float t, float ofs, float samp) {
  return FX(__ctx, floor((t + ofs) * samp + 0.5f) / samp - ofs);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float t = uv.data[0] * 3.f - 1.0f;
  float sign = FX(__ctx, t);
  float s = 44100.f / 2000.f;
  float of = fract(iTime) / s;
  float disc = discret(__ctx, t, of, s);
  float rest = restore(__ctx, t, of, s);
  rest = clamp((1.f - abs(rest * 0.25f + 0.5f - uv.data[1]) * 5.0f /
                          (0.01f + abs(ddx(rest)))),
               0.f, 1.f);
  sign = clamp((1.f - abs(sign * 0.25f + 0.5f - uv.data[1]) * 5.0f /
                          (0.01f + abs(ddx(sign)))),
               0.f, 1.f);
  float dto = (t + of) * s + 0.5f;
  float dt = floor(dto);
  float x = ((dto - dt) - 0.5f);
  disc = disc * 0.25f + 0.5f;
  disc = abs(disc - uv.data[1]) > 0.005f || abs(x) > 0.007f * s ? 0.f : 1.f;
  f32x3 vawe = f32x3(1.f, 1.f, 0.5f) * sign;
  f32x3 rez = f32x3(0.7f, 0.7f, 1.0f) * rest;
  f32x3 dots = f32x3(disc);
  f32x3 col = clamp(dots + vawe + rez, f32x3(0.f), f32x3(1.f));
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
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
