// Port of https://www.shadertoy.com/view/DsSyDG; author: electroglyph.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DsSyDG.json.
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

const float depthCull = .1f;
const float zoom = 3.5f;
const float bailout = 10.f;
const f32x2 offset = f32x2(.3f, .45f);
const int iterations = 9;
f32x3 hsv(Context &__ctx, const float h, const float s, const float v) {
  return mix(f32x3(1.0f),
             clamp((abs(fract(h + f32x3(3.f, 2.f, 1.f) / 3.0f) * 6.f - 3.f) -
                    1.f),
                   0.f, 1.0f),
             s) *
         v;
}
f32x3 permute(Context &__ctx, f32x3 x) {
  return mod((34.0f * x + 1.0f) * x, 289.0f);
}
f32x2 cellular(Context &__ctx, f32x2 P) {
  f32x2 Pi = mod(floor(P), 289.0f);
  f32x2 Pf = fract(P);
  f32x3 oi = f32x3(-1.0f, 0.0f, 1.0f);
  f32x3 of = f32x3(-0.5f, 0.5f, 1.5f);
  f32x3 px = permute(__ctx, Pi.data[0] + oi);
  f32x3 p = permute(__ctx, px.data[0] + Pi.data[1] + oi);
  f32x3 ox = fract(p * 0.142857142857f) - 0.428571428571f;
  f32x3 oy =
      mod(floor(p * 0.142857142857f), 7.0f) * 0.142857142857f - 0.428571428571f;
  f32x3 dx = Pf.data[0] + 0.5f + 1.0f * ox;
  f32x3 dy = Pf.data[1] - of + 1.0f * oy;
  f32x3 d1 = dx * dx + dy * dy;
  p = permute(__ctx, px.data[1] + Pi.data[1] + oi);
  ox = fract(p * 0.142857142857f) - 0.428571428571f;
  oy =
      mod(floor(p * 0.142857142857f), 7.0f) * 0.142857142857f - 0.428571428571f;
  dx = Pf.data[0] - 0.5f + 1.0f * ox;
  dy = Pf.data[1] - of + 1.0f * oy;
  f32x3 d2 = dx * dx + dy * dy;
  p = permute(__ctx, px.data[2] + Pi.data[1] + oi);
  ox = fract(p * 0.142857142857f) - 0.428571428571f;
  oy =
      mod(floor(p * 0.142857142857f), 7.0f) * 0.142857142857f - 0.428571428571f;
  dx = Pf.data[0] - 1.5f + 1.0f * ox;
  dy = Pf.data[1] - of + 1.0f * oy;
  f32x3 d3 = dx * dx + dy * dy;
  f32x3 d1a = min(d1, d2);
  d2 = max(d1, d2);
  d2 = min(d2, d3);
  d1 = min(d1a, d2);
  d2 = max(d1a, d2);
  d1.set_swizzle<0, 1>((d1.data[0] < d1.data[1]) ? d1.swizzle<0, 1>()
                                                 : d1.swizzle<1, 0>());
  d1.set_swizzle<0, 2>((d1.data[0] < d1.data[2]) ? d1.swizzle<0, 2>()
                                                 : d1.swizzle<2, 0>());
  d1.set_swizzle<1, 2>(min(d1.swizzle<1, 2>(), d2.swizzle<1, 2>()));
  d1.data[1] = min(d1.data[1], d1.data[2]);
  d1.data[1] = min(d1.data[1], d2.data[0]);
  return sqrt(d1.swizzle<0, 1>());
}
f32x2 cmult(Context &__ctx, f32x2 a, f32x2 b) {
  f32x2 p;
  p[0] = a[0] * b[0] - a[1] * b[1];
  p[1] = a[0] * b[1] + a[1] * b[0];
  return p;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float time = iTime;
  f32x2 offset = f32x2(offset.data[0] + sin(time * .1f) * .05f,
                       offset.data[1] + cos(time * .1f) * .05f);
  f32x2 position =
      gl_FragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - offset;
  position = position * (zoom - .7f);
  f32x2 perturb =
      f32x2(-.7f + (sin(time * .11f) * .1f) + (sin(time * .2f) * .4f),
            0.45f + cos(time * .03f) * .05f) +
      (sin(time * .12f) * .3f);
  f32x2 c, c0, d;
  float v;
  c = f32x2(position);
  c0 = perturb;
  c += (cellular(__ctx, c * 4.f)) * .025f;
  f32x2 f = position.swizzle<0, 1>();
  for (int i = 0; i < iterations; i++) {
    d = cmult(__ctx, c, c);
    c = d + c0;
    v = abs((c.data[0] * c.data[0]) + sin(.1f * time + c.data[1] * c.data[1])) /
        sqrt(.1f * time + c.data[0] * c.data[0]);
    if (v > bailout)
      break;
  }
  f32x2 c1 = c + cellular(__ctx, .1f * time + c * 3.f);
  f32x2 c2 = cellular(__ctx, -.1f * f32x2(sin(.4f * time + c.data[1] * .2f),
                                          sin(.02f * time + c.data[0] * .5f)));
  f32x2 c3 = cellular(__ctx, c * .01f);
  float rand = mod(fract(sin(dot(2.5f * gl_FragCoord.swizzle<0, 1>() /
                                     iResolution.swizzle<0, 1>(),
                                 f32x2(12.9898f, 100.233f))) *
                         43758.5453f),
                   .4f);
  float col = (pow(v, -.23f) * pow(sin(c2.data[0]), .99f));
  float col2 = (pow(sin(c1.data[1]) + 1.75f, .99f));
  if (v > 10.f) {
    fragColor = f32x4(hsv(__ctx, .8f + sin(c3.data[0] + col) * .3f,
                          .5f - c3.data[0], .1f + col * .75f),
                      1.f);
  } else
    fragColor = f32x4(hsv(__ctx, .8f + sin(col2) * .3f,
                          .5f - col + sin(time * .4f) * .3f, col2 * .3f),
                      1.f);
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
