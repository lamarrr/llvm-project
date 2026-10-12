// Port of https://www.shadertoy.com/view/cdjBzc; author: TinyTexel.
// Dataset license metadata: cc0-1.0; original retained in upstream/cdjBzc.json.
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
float capsule_aa(Context &__ctx, f32x2 tc, float r, float s, f32x2 a, f32x2 b) {
  f32x2 vec = b - a;
  tc -= a;
  float tt = dot(vec, vec);
  float rcptt = 1.0f / tt;
  f32x2 p = vec * (tt == 0.0f ? 0.0f : clamp(dot(tc, vec * rcptt), 0.0f, 1.0f));
  float l = length(tc - p);
  float v = clamp((r - l) * s * 0.5f, 0.0f, 1.0f);
  v = v * v * (3.0f - 2.0f * v);
  return v;
}
float lineseg_mb(Context &__ctx, float x, float r, float t) {
  float u = r + t * 0.5f;
  return clamp(u - abs(-x - r + u), 0.0f, min(2.0f * r, t)) * (1.0f / t);
}
float disk_mb(Context &__ctx, f32x2 tc, float r, float t) {
  float r0 = sqrt(max(0.0f, r * r - tc.data[1] * tc.data[1]));
  return lineseg_mb(__ctx, tc.data[0], r0, t);
}
float disk_mb(Context &__ctx, f32x2 tc, float r, f32x2 a, f32x2 b) {
  float aa_mask = capsule_aa(__ctx, tc, r, iResolution.data[1], a, b);
  f32x2 vec = b - a;
  tc -= a;
  float tt = dot(vec, vec);
  float rcptt = 1.0f / tt;
  f32x2 dirX = vec * rcptt;
  f32x2 dirY = f32x2(-dirX.data[1], dirX.data[0]);
  float x = dot(tc, dirX);
  float y = dot(tc, dirY);
  float r0 = sqrt(max(0.0f, r * r * rcptt - y * y));
  float u = r0 + 0.5f;
  float v = clamp(u - abs(-x - r0 + u), 0.0f, min(2.0f * r0, 1.0f));
  if (tt == 0.0f)
    v = dot(tc, tc) < r * r ? 1.0f : 0.0f;
  v *= aa_mask;
  return v;
}
float disk_mb_aa(Context &__ctx, f32x2 tc, float r, float s, f32x2 a, f32x2 b) {
  f32x2 vec = b - a;
  tc -= a;
  float tt = dot(vec, vec);
  float rcptt = 1.0f / tt;
  f32x2 dirX = vec * rcptt;
  f32x2 dirY = f32x2(-dirX.data[1], dirX.data[0]);
  float x = dot(tc, dirX);
  float y = dot(tc, dirY);
  float r0 = sqrt(max(0.0f, r * r * rcptt - y * y));
  float u = r0 + 0.5f;
  float v = clamp(u - abs(-x - r0 + u), 0.0f, min(2.0f * r0, 1.0f));
  if (tt == 0.0f)
    v = dot(tc, tc) < r * r ? 1.0f : 0.0f;
  {
    f32x2 p = vec * (tt == 0.0f ? 0.0f : clamp(x, 0.0f, 1.0f));
    float l = length(tc - p);
    float m = clamp((r - l) * s * 0.5f, 0.0f, 1.0f);
    v *= m * m * (3.0f - 2.0f * m);
  }
  return v;
}
void mainImage(Context &__ctx, f32x4 &outCol, f32x2 uv0) {
  f32x2 tc =
      (uv0 - iResolution.swizzle<0, 1>() * 0.5f) / iResolution.swizzle<1, 1>();
  f32x3 col = f32x3(0.0f);
  float v = 0.0f;
  float r = 0.25f;
  float t = 0.5f;
  v = disk_mb(__ctx, tc, r, t);
  v = disk_mb(__ctx, tc, r, f32x2(-0.0f, -0.f), f32x2(0.f, 0.f));
  v = disk_mb_aa(__ctx, tc, r, iResolution.data[1], f32x2(-0.2f, -0.f),
                 f32x2(0.2f, 0.f));
  col = f32x3(v);
  outCol = f32x4(pow(col, f32x3(1.0f / 2.2f)), 0.0f);
  outCol = f32x4(col, 0.0f);
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
