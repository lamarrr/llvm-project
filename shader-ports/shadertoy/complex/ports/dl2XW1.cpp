// Port of https://www.shadertoy.com/view/dl2XW1; author: rsk.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dl2XW1.json.
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
bool isLine(Context &__ctx, f32x2 a, f32x2 b, f32x2 p) {
  a = floor(a);
  b = floor(b);
  b.data[0] += 0.01f;
  p = floor(p);
  f32x2 ab = b - a;
  int i = int(abs(ab.data[0]) > abs(ab.data[1]));
  int xi = 1 - i;
  int yi = i;
  float t = (p[xi] - a[xi]) / ab[xi];
  float y = round(a[yi] + ab[yi] * t);
  float minX = round(min(a[xi], b[xi]));
  float maxX = round(max(a[xi], b[xi]));
  return p[yi] == y && minX <= p[xi] && p[xi] <= maxX;
}
bool seg(Context &__ctx, f32x2 position, float size, float rotation,
         f32x2 fragment) {
  f32x2 cs = f32x2(cos(rotation), sin(rotation));
  f32x2 a = -cs * size / 2.0f + position;
  f32x2 b = cs * size / 2.0f + position;
  return isLine(__ctx, a, b, fragment);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragCoord /= 1.0f;
  const float tau = 6.283185307179586f;
  f32x2 r = iResolution.swizzle<0, 1>() / 1.0f;
  float s = min(r.data[0], r.data[1]) / 2.25f;
  f32x2 p = r / f32x2(3.0f, 2.0f);
  bool lines = seg(__ctx, p, s, 0.0f, fragCoord);
  lines = lines || seg(__ctx, p, s, tau / 4.0f, fragCoord);
  float pxScale = s / 200.0f;
  lines = lines || isLine(__ctx, p + round(f32x2(10.0f) * pxScale),
                          p + round(f32x2(10.0f) * pxScale), fragCoord);
  lines = lines || isLine(__ctx, p + round(f32x2(15.0f) * pxScale),
                          p + round(f32x2(50.0f) * pxScale), fragCoord);
  lines = lines || isLine(__ctx, p + round(f32x2(15.0f) * pxScale),
                          p + round(f32x2(20.0f, 25.0f) * pxScale), fragCoord);
  lines = lines || isLine(__ctx, p + round(f32x2(15.0f) * pxScale),
                          p + round(f32x2(25.0f, 20.0f) * pxScale), fragCoord);
  p += f32x2(p.data[0], 0.0f);
  float t = iTime / 3.0f;
  lines = lines || seg(__ctx, p, s, t, fragCoord);
  float tShifted = iTime / 25.0f + tau / 4.0f;
  lines = lines || seg(__ctx, p, s, tShifted, fragCoord);
  tShifted += tau / 4.0f;
  lines = lines || seg(__ctx, p, s, tShifted, fragCoord);
  const int steps = 20;
  p = r / f32x2(float(steps) + 1.0f, 7.0f);
  float ps = s / 5.5f;
  for (int i = 0; i < steps; i++) {
    lines = lines || seg(__ctx, p + f32x2(p.data[0] * float(i), 0.0f), ps,
                         tau / float(steps - 1) * float(i), fragCoord);
  }
  p = f32x2(r.data[0] / (float(steps) + 1.0f), r.data[1] * 6.0f / 7.0f);
  for (int i = 0; i < steps; i++) {
    lines = lines || seg(__ctx, p + f32x2(p.data[0] * float(i), 0.0f), ps,
                         tau / float(steps - 1) * float(i), fragCoord);
  }
  fragColor = f32x4(f32x3(!lines), 1.0f);
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
