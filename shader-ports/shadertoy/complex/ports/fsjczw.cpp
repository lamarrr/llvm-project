// Port of https://www.shadertoy.com/view/fsjczw; author: ErinZ.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fsjczw.json.
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
float circle(Context &__ctx, f32x2 uv, f32x2 o, float r, float blur) {
  float d = length(uv - o);
  float c = smoothstep(r, r - blur, d);
  return c;
}
float lineVer(Context &__ctx, f32x2 uv, f32x2 p, float width, float blur) {
  width /= 2.f;
  uv -= p;
  return smoothstep(-width - blur, -width, uv.data[0]) *
         smoothstep(width + blur, width, uv.data[0]);
}
float line(Context &__ctx, f32x2 uv, f32x2 p, float k, float width,
           float blur) {
  width /= 2.f;
  uv -= p;
  float d = abs(k * uv.data[0] - uv.data[1]) / sqrt(k * k + 1.f);
  return smoothstep(-width - blur, -width, d) *
         smoothstep(width + blur, width, d);
}
float line(Context &__ctx, f32x2 uv, f32x2 p1, f32x2 p2, float width,
           float blur) {
  width /= 2.f;
  blur /= 2.f;
  f32x2 pa = uv - p1, ba = p2 - p1;
  float h = dot(pa, ba) / dot(ba, ba);
  float d = length(pa - ba * h);
  return smoothstep(width + blur, width - blur, d);
}
float segment(Context &__ctx, f32x2 uv, f32x2 p1, f32x2 p2, float width,
              float blur) {
  width /= 2.f;
  blur /= 2.f;
  f32x2 pa = uv - p1, ba = p2 - p1;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.f, 1.f);
  float d = length(pa - ba * h);
  return smoothstep(width + blur, width - blur, d);
}
float arc(Context &__ctx, f32x2 uv, f32x2 o, float r, float a1, float a2,
          float width, float blur) {
  uv -= o;
  width /= 2.f;
  blur /= 2.f;
  float ap = (uv.data[0] > 0.f) ? acos(uv.data[1] / length(uv))
                                : acos(-uv.data[1] / length(uv)) + 3.14f;
  f32x2 p1 = f32x2(sin(a1) * r, cos(a1) * r);
  f32x2 p2 = f32x2(sin(a2) * r, cos(a2) * r);
  float d = (ap > a1 && ap < a2) ? (abs(length(uv) - r))
                                 : min(length(uv - p1), length(uv - p2));
  return smoothstep(width + blur, width - blur, d);
}
float triangle(Context &__ctx, f32x2 uv, f32x2 o, float a, float blur) {
  uv -= o;
  uv.data[0] = abs(uv.data[0]) - a;
  const float k = sqrt(3.f);
  uv.data[1] += 1.f / k * a;
  if (uv.data[0] + k * uv.data[1] > 0.f)
    uv = f32x2(uv.data[0] - k * uv.data[1], -k * uv.data[0] - uv.data[1]) / 2.f;
  uv.data[0] -= clamp(uv.data[0], -2.0f * a, 0.0f);
  float d = -length(uv) * sign(uv.data[1]);
  return smoothstep(0.f + blur, 0.f, d);
}
float rectangle(Context &__ctx, f32x2 uv, f32x2 p1, f32x2 p2, float blur) {
  blur /= 2.f;
  return smoothstep(p1.data[0] - blur, p1.data[0] + blur, uv.data[0]) *
         smoothstep(p2.data[0] + blur, p2.data[0] - blur, uv.data[0]) *
         smoothstep(p1.data[1] + blur, p1.data[1] - blur, uv.data[1]) *
         smoothstep(p2.data[1] - blur, p2.data[1] + blur, uv.data[1]);
}
f32x2 rotate(Context &__ctx, f32x2 uv, f32x2 p, float a) {
  uv -= p;
  f32x2 q;
  q.data[0] = cos(a) * uv.data[0] + sin(a) * uv.data[1];
  q.data[1] = -sin(a) * uv.data[0] + cos(a) * uv.data[1];
  q += p;
  return q;
}
f32x2 bevel(Context &__ctx, f32x2 p, float x, float y) {
  f32x2 q;
  q.data[0] = p.data[0] + p.data[1] * x;
  q.data[1] = p.data[1] + p.data[0] * y;
  return q;
}
f32x3 coordSystem(Context &__ctx, f32x2 p) {
  f32x3 col = f32x3(0);
  float c;
  c = lineVer(__ctx, p, f32x2(0.f, 0.f), 0.003f, 0.005f);
  c += line(__ctx, p, f32x2(0.f, 0.f), 0.f, 0.003f, 0.005f);
  c += lineVer(__ctx, p, f32x2(1.f, 0.f), 0.003f, 0.005f) * .3f;
  c += lineVer(__ctx, p, f32x2(-1.f, 0.f), 0.003f, 0.005f) * .3f;
  col += c * f32x3(.5f, .8f, .9f);
  col += f32x3(circle(__ctx, p, f32x2(0.f, 0.f), 0.02f, 0.005f));
  col = mix(col, f32x3(1, 0.16f, 0.6f),
            f32x3(circle(__ctx, p, f32x2(0.f, 0.f), 0.015f, 0.005f)));
  return col;
}
f32x3 animate(Context &__ctx, f32x2 p, float time) {
  f32x3 col = f32x3(0, 0, 1);
  float tt = mod(time, 2.f) / 2.f;
  float ss = pow(tt, .2f) * 0.5f + 0.5f;
  ss = ss * .8f * sin(tt * 6.28f * 3.0f + p.data[1] * 0.5f) * exp(-tt * 4.f);
  col += circle(__ctx, p, f32x2(-.5f, ss * sin(time * 12.0f) * 0.7f + .2f),
                0.1f, 0.005f);
  col +=
      circle(__ctx, p, f32x2(.5f, ss * sin(time * 12.0f) * 0.7f), 0.1f, 0.005f);
  p = bevel(__ctx, p, ss * (p.data[1] + 0.5f), 0.f);
  col += rectangle(__ctx, p, f32x2(-.3f, .5f), f32x2(.3f, -.5f), 0.005f);
  col -= circle(__ctx, p, f32x2(-.06f, .3f), .05f, 0.005f * 2.f) * 2.f;
  col -= circle(__ctx, p, f32x2(.06f, .3f), .05f, 0.005f * 2.f) * 2.f;
  col -= segment(__ctx, p, f32x2(-.05f, .3f), f32x2(.05f, .3f), .01f,
                 0.005f * 2.f) *
         2.f;
  col = clamp(col, 0.f, 1.f);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 col = f32x3(0.f);
  float amount = 32.f;
  float timescale = .1f;
  f32x3 blurcol = f32x3(0.f);
  for (int i = 0; i < int(amount); i++) {
    blurcol +=
        animate(__ctx, p, iTime - float(i) * timescale / amount) / amount;
  }
  col += blurcol;
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output) {
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
