// Port of https://www.shadertoy.com/view/dlBGR1; author: z0rg.
// Dataset license metadata: cc-by-nc-4.0; original retained in
// upstream/dlBGR1.json.
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
float _seed;
float hash11(Context &__ctx, float seed) {
  return fract(sin(seed * 123.456f) * 123.456f);
}
float rand(Context &__ctx) { return hash11(__ctx, _seed++); }
f32x3 gradient(Context &__ctx, float i) {
  f32x3 grad[4];
  grad[0] = f32x3(0.765f, 0.376f, 0.314f);
  grad[1] = f32x3(0.349f, 0.871f, 0.910f);
  grad[2] = f32x3(0.765f, 0.376f, 0.314f);
  grad[3] = f32x3(0.137f, 0.122f, 0.937f) * 2.f;
  float idx = clamp(i, 0.f, 1.f) * 3.0f;
  int prevIdx = int(idx);
  int nextIdx = min(prevIdx + 1, 3);
  f32x3 prev = grad[prevIdx];
  f32x3 next = grad[nextIdx];
  float lrp = idx - float(prevIdx);
  return mix(prev, next, smoothstep(0.f, 1.f, lrp));
}
f32x3 rdr(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.05f);
  f32x2 uv2 = uv;
  float rep = .1f;
  uv2.data[1] += sin(uv2.data[0] * 3.f) * .1f +
                 .2f * sin(uv2.data[1] * 5.f + uv.data[0] * 5.f + iTime * .2f);
  float lines = sin(uv2.data[1] * 280.f) + .9999f;
  float mask = length(uv) - .25f;
  float sharp = iResolution.data[0] * .5f;
  col = mix(col, gradient(__ctx, length(uv2) * 2.f),
            (1.f - clamp(lines, 0.f, 1.f)) *
                (1.f - clamp(mask * sharp, 0.f, 1.f)));
  col = mix(col, gradient(__ctx, length(-uv2) * 2.f).swizzle<2, 0, 1>(),
            1.f - clamp((abs(mask) - .001f) * sharp, 0.f, 1.f));
  col += gradient(__ctx, length(uv2) * 2.f) *
         (1.f - clamp(lines * .5f, 0.f, 1.f)) * .5f *
         (1.f - clamp(length(uv), 0.f, 1.f));
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord.swizzle<0, 1>() - .5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  float stp = .007f;
  uv = floor((uv / stp)) * stp;
  _seed = iTime + texture(__ctx.channel0, uv).data[0];
  f32x3 col = rdr(__ctx, uv);
  col += pow(rand(__ctx), .5f) * .1f;
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
