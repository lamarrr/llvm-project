// Port of https://www.shadertoy.com/view/DtBGD3; author: megamaz.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtBGD3.json.
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

float lerp(Context &__ctx, float factor, float a, float b) {
  return a + (b - a) * factor;
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x4 K = f32x4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
  f32x3 p = abs(fract(c.swizzle<0, 0, 0>() + K.swizzle<0, 1, 2>()) * 6.0f -
                K.swizzle<3, 3, 3>());
  return c.data[2] * mix(K.swizzle<0, 0, 0>(),
                         clamp(p - K.swizzle<0, 0, 0>(), 0.0f, 1.0f),
                         c.data[1]);
}
f32x3 calcMandelbrot(Context &__ctx, f32x2 fragCoord, float iter) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float real = uv[0];
  float imag = uv[1];
  float col = 0.0f;
  real = lerp(__ctx, uv[0], -2.2f, 1.0f);
  imag = lerp(__ctx, uv[1], -1.0f, 1.0f);
  float creal = real;
  float cimag = imag;
  for (float i = 0.0f; i < iter; i++) {
    float realtemp = real;
    real = (real * real) - (imag * imag) + creal;
    imag = 2.0f * (realtemp * imag) + cimag;
    if (sqrt((real * real) + (imag * imag)) > 4.0f) {
      col = i / iter;
    }
  }
  f32x3 color = f32x3(0.0f, 0.0f, 0.0f);
  if (col != 0.0f) {
    color = hsv2rgb(__ctx, f32x3(col * 2.0f, 1.0f, 1.0f));
  }
  return color;
}
f32x3 calcJulia(Context &__ctx, f32x2 fragCoord, f32x2 p, float iter) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float real = uv[0];
  float imag = uv[1];
  float col = 0.0f;
  real = lerp(__ctx, uv[0], -2.2f, 1.0f);
  imag = lerp(__ctx, uv[1], -1.0f, 1.0f);
  float creal = p[0];
  float cimag = p[1];
  for (float i = 0.0f; i < iter; i++) {
    float realtemp = real;
    real = (real * real) - (imag * imag) + creal;
    imag = 2.0f * (realtemp * imag) + cimag;
    if (sqrt((real * real) + (imag * imag)) > 4.0f) {
      col = i / iter;
    }
  }
  f32x3 color = f32x3(0.0f, 0.0f, 0.0f);
  if (col != 0.0f) {
    color = hsv2rgb(__ctx, f32x3(col * 2.0f, 1.0f, 1.0f));
  }
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float iter = 100.0f;
  f32x2 mousepos = f32x2(lerp(__ctx, iMouse[0] / iResolution[0], -2.2f, 1.0f),
                         lerp(__ctx, iMouse[1] / iResolution[1], -1.0f, 1.0f));
  f32x3 mcolor = calcMandelbrot(__ctx, fragCoord, iter);
  f32x3 jcolor = calcJulia(__ctx, fragCoord, mousepos, iter);
  f32x3 color = (mcolor + jcolor) / f32x3(2.0f, 2.0f, 2.0f);
  fragColor = f32x4(color, 1.0f);
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
