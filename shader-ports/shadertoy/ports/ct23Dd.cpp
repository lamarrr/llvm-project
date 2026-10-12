// Port of https://www.shadertoy.com/view/ct23Dd; author: byt3_m3chanic.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/ct23Dd.json.
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
float hash21(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(23.43f, 84.21f))) * 4832.3234f);
}
f32x2x2 rot(Context &__ctx, float a) {
  return f32x2x2(cos(a), sin(a), -sin(a), cos(a));
}
f32x3 hue(Context &__ctx, float t) {
  t += iTime * .06f;
  return .55f + .75f * cos(6.28318530718f * t *
                           (f32x3(1.f, .99f, .95f) + f32x3(.1f, .34f, .27f)));
}
float line(Context &__ctx, f32x2 p) {
  f32x2 a = f32x2(.2f, .5f), b = f32x2(.8f, .5f);
  f32x2 ba = b - a, pa = p - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.f, 1.f);
  float d = length(pa - h * ba);
  return d - .1f;
}
const float scale = 8.f;
void mainImage(Context &__ctx, f32x4 &O, f32x2 F) {
  f32x2 uv = (2.f * F.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
             max(iResolution.data[0], iResolution.data[1]);
  uv.data[0] -= iTime * .05f;
  f32x3 h1 = hue(__ctx, uv.data[1] * .04f + iTime * .01f);
  f32x3 h2 = hue(__ctx, (15.f + uv.data[1]) * .04f + iTime * .05f);
  f32x3 h3 = hue(__ctx, uv.data[0] * .04f + iTime * .01f);
  f32x3 C = hue(__ctx, (10.f - uv.data[0]) * .04f + iTime * .1f);
  f32x2 id = floor(uv * scale);
  f32x2 gv = fract(uv * scale) - .5f;
  float px = scale / iResolution.data[0];
  float check = mod(id.data[1] + id.data[0], 2.f) * 2.f - 1.f;
  float rnd = hash21(__ctx, id);
  if (rnd > .5f)
    gv.data[0] *= -1.f;
  f32x2 d2 = f32x2(length(gv - .5f), length(gv + .5f));
  f32x2 g2 = d2.data[0] < d2.data[1] ? gv - .5f : gv + .5f;
  float ft = .2f + .1f * sin(uv.data[1] * 5.2f + iTime);
  float d = length(g2) - .5f;
  float dl = smoothstep(px, -px, abs(abs(d) - ft) - .02f);
  d = smoothstep(-px, px, abs(d) - ft);
  C = mix(C, h3, d);
  C = mix(C, f32x3(.8f), dl);
  f32x2 arc = gv - sign(gv.data[0] + gv.data[1] + .001f) * .5f;
  float angle = atan(arc.data[0], arc.data[1]);
  d = length(arc);
  float x = fract(3.f * check * angle / 3.14159265359f + iTime * 1.15f);
  float y = (d / .5f) - .5f;
  if (rnd < .5f ^ ^check > 0.f)
    y = 1.f - y;
  f32x2 tuv = f32x2(x, y);
  float ts = length(tuv - f32x2(.5f)) - .25f;
  ts = smoothstep(.05f + px, -px, abs(ts) - .125f);
  C = mix(C, f32x3(.01f), ts);
  float t = length(tuv - f32x2(.5f)) - .25f;
  t = smoothstep(px, -px, abs(t) - .075f);
  C = mix(C, h2, t);
  ts = min(line(__ctx, tuv - f32x2(.5f, 0)), line(__ctx, tuv + f32x2(.5f, 0)));
  ts = smoothstep(.05f + px, -px, abs(ts) - .05f);
  C = mix(C, f32x3(.01f), ts);
  t = min(line(__ctx, tuv - f32x2(.5f, 0)), line(__ctx, tuv + f32x2(.5f, 0)));
  t = smoothstep(px, -px, t);
  C = mix(C, h1, t);
  C = pow(C, f32x3(.4545f));
  O = f32x4(C, 1.f);
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
