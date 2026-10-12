// Port of https://www.shadertoy.com/view/fd2fW1; author: SnoopethDuckDuck.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fd2fW1.json.
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
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(6.28318f * (c * t + d));
}
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float mlength(Context &__ctx, f32x2 uv) {
  return max(abs(uv.data[0]), abs(uv.data[1]));
}
float mlength(Context &__ctx, f32x3 uv) {
  return max(max(abs(uv.data[0]), abs(uv.data[1])), abs(uv.data[2]));
}
float smin(Context &__ctx, float a, float b) {
  float k = 0.12f;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
f32x2x2 Rot(Context &__ctx, float a) {
  float c = cos(a), s = sin(a);
  return f32x2x2(c, -s, s, c);
}
f32x2 tabs(Context &__ctx, f32x2 x, float m, float o) {
  return abs(x) + m * tanh(0.01f * cos(2.5f * abs(x) + o)) / tanh(0.01f);
}
float tlength(Context &__ctx, f32x2 uv, float m) {
  return mix(length(uv), mlength(__ctx, uv),
             0.5f + 0.5f * tanh(4.f * cos(m)) / tanh(4.f));
}
void render(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 ouv = uv;
  uv.data[1] += 0.03f * cos(iTime);
  float r = 0.25f;
  float th = 0.01f;
  uv *= 1.5f;
  float t = iTime;
  float d = 1.f;
  float k = 1.f / iResolution.data[1];
  float s = step(abs(d - 2.f * r), th);
  float m = 0.25f;
  float n = 7.f;
  float a = -0.2f * iTime;
  for (float i = 0.f; i < n; i++) {
    float a2 = atan(uv.data[0], uv.data[1]);
    uv *= Rot(__ctx,
              tanh(4000.f * cos(3.f * a2 + a)) / tanh(4000.f) * length(uv) +
                  a2 * 3.f + a);
    uv = abs(uv) - m;
    d = abs(length(uv) - mlength(__ctx, uv)) - 0.25f * m;
    if (d >= r + 0.f * th) {
      s += step(s, 0.f) * step(abs(d - r), th);
    }
    m *= 1.f;
    r = m * d;
  }
  f32x3 col = f32x3(s);
  f32x3 e = f32x3(1);
  f32x3 orange =
      pal(__ctx, 0.2f * iTime - ouv.data[1] * 0.1f + length(uv) * 2.f - 0.08f,
          e, e, e, 0.8f * f32x3(0, 1, 2) / 3.f);
  col += 0.13f;
  float s1 = step(length(ouv), 0.45f);
  float s2 = step(length(abs(ouv) - f32x2(1.5f * 0.44f, 0)), 0.1f);
  col *= s1 + s2;
  col += step(abs(length(ouv) - 0.47f), 1.f / 400.f);
  col +=
      step(abs(length(abs(ouv) - f32x2(1.5f * 0.44f, 0)) - 0.12f), 1.f / 400.f);
  col *= orange;
  col = sqrt(col);
  fragColor = f32x4(col, 1.0f);
}
void mainImage(Context &__ctx, f32x4 &O, f32x2 C) {
  float px = 1.f / 2.0f, i, j;
  f32x4 cl2, cl;
  if (2.0f == 1.f) {
    render(__ctx, cl, C);
    O = cl;
    return;
  }
  for (i = 0.f; i < 2.0f; i++) {
    for (j = 0.f; j < 2.0f; j++) {
      f32x2 C2 = f32x2(C.data[0] + px * i, C.data[1] + px * j);
      render(__ctx, cl2, C2);
      cl += cl2;
    }
  }
  cl /= 2.0f * 2.0f;
  O = cl;
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
