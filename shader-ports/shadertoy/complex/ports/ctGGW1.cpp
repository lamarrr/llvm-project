// Port of https://www.shadertoy.com/view/ctGGW1; author: fenix.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/ctGGW1.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
void mainImage(Context &__ctx, f32x4 &O, f32x2 u) {
  f32x3 R = iResolution, h;
  float t = iTime / 8.f, m = abs(fract(t) - .5f), d = m,
        M = smoothstep(.5f, 0.f, d), r = fract(t * .5f) > .5f ? 1.f : 0.f,
        z = 4.f - 2.f * cos(t);
  f32x2 P, D;
  u = z * (u + u - R.swizzle<0, 1>()) / R.data[1] + r * .5f;
  z *= 3.f / R.data[1];
  d = 1.f;
  for (int k = 0; k < 9; k++)
    P = f32x2(k % 3, k / 3) - .5f,
    h = fract(1e4f * sin(trunc(f32x3(u + P + .5f, t)) *
                         f32x3x3(R + 17.f, R + 51.f, 57.f - R))) -
        .5f,
    D = abs((fract(u) - P +
             h.swizzle<0, 1>() * f32x2x2(cos(t * 3.14f + f32x4(0, 11, 33, 0))) *
                 M) *
            f32x2x2(cos(.8f -
                        trunc(h.data[2] * 12.f + sign(h.data[2])) * fract(t) *
                            3.14f +
                        f32x4(0, 11, 33, 0)))) -
        .354f,
    d = min(d, length(max(D, 0.f)) + min(max(D.data[0], D.data[1]), 0.f));
  O = (f32x4(.3f, .6f, 1, 1) - r * f32x4(-.2f, .5f, .5f, 0)) *
      (1.f - .5f * smoothstep(M * .1f + z, 0.f, d));
  d = m - .25f, u += f32x2(.2f, -.2f) * smoothstep(.25f, 0.f, d);
  r = 1.f - r;
  d = 1.f;
  for (int k = 0; k < 9; k++)
    P = f32x2(k % 3, k / 3) - .5f,
    h = fract(1e4f * sin(trunc(f32x3(u + P + .5f, t)) *
                         f32x3x3(R + 17.f, R + 51.f, 57.f - R))) -
        .5f,
    D = abs((fract(u) - P +
             h.swizzle<0, 1>() * f32x2x2(cos(t * 3.14f + f32x4(0, 11, 33, 0))) *
                 M) *
            f32x2x2(cos(.8f -
                        trunc(h.data[2] * 12.f + sign(h.data[2])) * fract(t) *
                            3.14f +
                        f32x4(0, 11, 33, 0)))) -
        .354f,
    d = min(d, length(max(D, 0.f)) + min(max(D.data[0], D.data[1]), 0.f));
  O += ((f32x4(.3f, .6f, 1, 1) - r * f32x4(-.2f, .5f, .5f, 0)) - O) *
       smoothstep(z, 0.f, d);
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
