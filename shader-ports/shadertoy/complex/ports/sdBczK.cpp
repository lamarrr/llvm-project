// Port of https://www.shadertoy.com/view/sdBczK; author: davidar.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdBczK.json.
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
float h21(Context &__ctx, f32x2 a) {
  return fract(sin(dot(a.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453123f);
}
float h11(Context &__ctx, float a) {
  return fract(sin((a) * 12.9898f) * 43758.5453123f);
}
f32x3 pal(Context &__ctx, float t, f32x3 a, f32x3 b, f32x3 c, f32x3 d) {
  return a + b * cos(2.f * 3.1415926535f * (c * t + d));
}
float box(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return max(d.data[0], d.data[1]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 R = iResolution.swizzle<0, 1>();
  f32x2 uv = (fragCoord - 0.5f * R.swizzle<0, 1>()) / R.data[1];
  f32x3 col = f32x3(0);
  float t = mod(iTime * 1.2f, 6000.f);
  float px = 1.f / iResolution.data[1];
  f32x2 xRange = f32x2(-0.5f, 0.5f) * R.data[0] / R.data[1];
  f32x2 yRange = f32x2(-0.5f, 0.5f);
  float id = 0.f;
  float seed = floor(t / 6.f);
  float a;
  float minSize = 0.015f;
  float iters = 10.f;
  float borderSize = -0.002f;
  float minIters = 1.f;
  for (float i = 0.f; i < iters; i++) {
    float xLength = xRange.data[1] - xRange.data[0];
    float yLength = yRange.data[1] - yRange.data[0];
    float dividex =
        h21(__ctx, f32x2(i + id, seed)) * (xLength) + xRange.data[0];
    float dividey =
        h21(__ctx, f32x2(i + id, seed)) * (yLength) + yRange.data[0];
    dividex =
        clamp(dividex, xRange.data[0] + minSize, xRange.data[1] - minSize);
    dividey =
        clamp(dividey, yRange.data[0] + minSize, yRange.data[1] - minSize);
    float mn =
        min(length(xRange.data[0] - dividex), length(xRange.data[1] - dividex));
    mn = min(mn, min(length(yRange.data[0] - dividey),
                     length(yRange.data[1] - dividey)));
    if (mn < minSize && i - 1.f > minIters)
      break;
    f32x2 diff = f32x2(0);
    if (uv.data[0] < dividex) {
      xRange = f32x2(xRange.data[0], dividex);
      diff.data[0] += dividex;
    } else {
      xRange = f32x2(dividex, xRange.data[1]);
      diff.data[0] -= dividey;
    }
    if (uv.data[1] < dividey) {
      yRange = f32x2(yRange.data[0], dividey);
      diff.data[1] -= dividex;
    } else {
      yRange = f32x2(dividey, yRange.data[1]);
      diff.data[1] += dividey;
    }
    id = length(diff + 10.0f);
    xLength = xRange.data[1] - xRange.data[0];
    yLength = yRange.data[1] - yRange.data[0];
    f32x2 center = f32x2((xRange.data[0] + xRange.data[1]) / 2.f,
                         (yRange.data[0] + yRange.data[1]) / 2.f);
    f32x2 m =
        (.5f + .5f * f32x2(cos(iTime), sin(iTime * 1.7f))) * R.swizzle<0, 1>();
    if (iMouse.data[2] > 0.f)
      m = iMouse.swizzle<0, 1>();
    float scale = clamp(
        length(center - (m - 0.5f * R.swizzle<0, 1>()) / R.data[1]) / 0.3f, 0.f,
        1.f);
    xLength *= scale;
    yLength *= scale;
    a = box(__ctx, uv - center, f32x2(xLength, yLength) * 0.5f);
  }
  id = h11(__ctx, id) * 1000.0f;
  f32x3 e = f32x3(0.5f);
  f32x3 al =
      pal(__ctx, id * 0.1f, e * 1.2f, e, e * 2.0f, f32x3(0, 0.33f, 0.66f));
  col = clamp(al, 0.f, 1.f);
  col = f32x3(.95f);
  col -= .8f * smoothstep(-px, px, a + borderSize);
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
