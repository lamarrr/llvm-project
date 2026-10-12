// Port of https://www.shadertoy.com/view/dsG3zw; author: delajor.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dsG3zw.json.
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

f32x3 eval(Context &__ctx, f32x2 uv, f32x2 cen) {
  float sdf = length(uv - cen);
  float c0 = step(sdf, 0.25f);
  float c1 = 1.0f - step(sdf, 0.15f);
  f32x3 col = f32x3(1.0f - c0 * c1);
  return col;
}
float rand(Context &__ctx, float co) {
  return fract(sin(co * (91.3458f)) * 47453.5453f);
}
float rand(Context &__ctx, f32x2 co) {
  return fract(sin(dot(co.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
float rand(Context &__ctx, f32x3 co) {
  return rand(__ctx, co.swizzle<0, 1>() + rand(__ctx, co.data[2]));
}
f32x2 randv2(Context &__ctx, f32x2 co) {
  return f32x2(rand(__ctx, co), rand(__ctx, co + f32x2(7607.0f, 6211.0f))) -
         f32x2(0.5f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 pxy = 1.0f / iResolution.swizzle<0, 1>();
  f32x2 uv = fragCoord * pxy;
  float md = 4.0f;
  f32x2 pxm = f32x2(1.0f, md * iResolution.data[1] / iResolution.data[0]);
  f32x2 uvm = uv;
  uvm.data[0] = md * mod(uvm.data[0], 1.0f / md);
  uvm -= f32x2(0.5f, 0.5f);
  uvm *= pxm;
  const int N = 256;
  if (uv.data[0] < 1.0f / md) {
    f32x2 cen = f32x2(0.0f, 0.0f);
    f32x3 col = eval(__ctx, uvm, cen);
    fragColor = f32x4(col, 1.0f);
  } else if (uv.data[0] < 2.0f / md) {
    f32x2 cen = f32x2(0.0f, 0.0f);
    f32x3 col = f32x3(0.0f);
    f32x2 prnd = uvm;
    for (int i = 0; i < N; i++) {
      f32x2 rpx = randv2(__ctx, prnd);
      f32x2 uvr = uvm + 100.0f * rpx * pxy / pxm;
      prnd = float(i) * uvr;
      col += eval(__ctx, uvr, cen);
    }
    col /= float(N);
    fragColor = f32x4(col, 1.0f);
  } else if (uv.data[0] < 3.0f / md) {
    float c = 0.025f;
    f32x2 center = f32x2(0.0f, 0.0f);
    f32x2 _uv = uvm - center;
    float minDiff = 1.f;
    float index = 0.f;
    for (float i = 0.f; i < float(N); i++) {
      float phi = 0.5f * (1.0f + sqrt(5.0f));
      float angle = 2.0f * 3.14159265f * i * float(N) / phi;
      float radius = c * sqrt(i);
      float x = radius * cos(angle);
      float y = radius * sin(angle);
      f32x2 pos = f32x2(x, y);
      float diff = abs(distance(pos, _uv)) * 60.f;
      if (diff < minDiff) {
        minDiff = diff;
        index = i + 1.f;
      }
    }
    minDiff = smoothstep(0.f, .5f, minDiff);
    minDiff = minDiff * minDiff;
    float r = minDiff * 1.0f;
    float g = minDiff * 1.0f;
    float b = minDiff * 1.0f;
    fragColor = f32x4(r, g, b, 1);
  } else {
    float time = iTime + 1.f;
    f32x2 center = f32x2(0.0f, 0.0f);
    f32x2 _uv = uvm - center;
    float minDiff = 1.f;
    float index = 0.f;
    f32x3 col = f32x3(0.0f);
    float r = 50.0f / sqrt(float(N));
    for (float i = 0.f; i < float(N); i++) {
      float angle = 137.5009f * i + rand(__ctx, uvm);
      float radius = r * sqrt(i);
      float x = radius * cos(angle);
      float y = radius * sin(angle);
      f32x2 pos = f32x2(x, y) * pxy / pxm;
      col += eval(__ctx, uvm + pos, center);
    }
    col /= float(N);
    fragColor = f32x4(col, 1.0f);
  }
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
