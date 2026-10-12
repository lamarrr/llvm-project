// Port of https://www.shadertoy.com/view/7lGczy; author: Behzod.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7lGczy.json.
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
const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0f;
const float MAX_DIST = 100.0f;
const float PRECISION = 0.001f;
const f32x3 COLOR_BACKGROUND = f32x3(0.835f, 1, 1);
const float roadW = 9.0f;
const float rumbleW = roadW / 3.0f;
float sdSphere(Context &__ctx, f32x3 p, float r) {
  float d = length(p) - r;
  return d;
}
float road(Context &__ctx, f32x3 p) {
  float d = p.data[1] + 1.f;
  float left = p.data[0] + roadW / 2.f;
  d = max(d, -left);
  float right = -p.data[0] + roadW / 2.f;
  d = max(d, -right);
  return d;
}
float rumble(Context &__ctx, f32x3 p) {
  float d = p.data[1] + 0.7f;
  float rw = rumbleW;
  float left = p.data[0] + roadW / 2.f + rw;
  float right = -p.data[0] + roadW / 2.f + rw;
  d = min(max(max(d, -left), left - rw), max(max(d, -right), right - rw));
  return d;
}
float grass(Context &__ctx, f32x3 p) {
  float d = p.data[1] + 1.0f;
  float rw = rumbleW;
  float left = p.data[0] + roadW / 2.f + rw;
  float right = -p.data[0] + roadW / 2.f + rw;
  return min(max(d, left), max(d, right));
}
f32x2 opU(Context &__ctx, f32x2 d1, f32x2 d2) {
  return (d1.data[0] < d2.data[0]) ? d1 : d2;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 res = f32x2(1e10f, 0.f);
  f32x2 flooring = f32x2(road(__ctx, p), 0.5f);
  f32x2 sphereLeft = f32x2(sdSphere(__ctx, p - f32x3(-2.5f, 0, -2), 1.f), 1.5f);
  f32x2 sphereRight = f32x2(sdSphere(__ctx, p - f32x3(2.5f, 0, -2), 1.f), 2.5f);
  f32x2 rumbling = f32x2(rumble(__ctx, p), 3.5f);
  f32x2 grassing = f32x2(grass(__ctx, p), 4.5f);
  res = opU(__ctx, res, flooring);
  res = opU(__ctx, res, sphereLeft);
  res = opU(__ctx, res, sphereRight);
  res = opU(__ctx, res, rumbling);
  res = opU(__ctx, res, grassing);
  return res;
}
f32x2 rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float depth = MIN_DIST;
  f32x2 res = f32x2(0.0f);
  float id = 0.f;
  for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
    f32x3 p = ro + depth * rd;
    res = map(__ctx, p);
    depth += res.data[0];
    id = res.data[1];
    if (res.data[0] < PRECISION || depth > MAX_DIST)
      break;
  }
  return f32x2(depth, id);
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.0005f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, p + e.swizzle<0, 1, 1>()).data[0] +
      e.swizzle<1, 1, 0>() * map(__ctx, p + e.swizzle<1, 1, 0>()).data[0] +
      e.swizzle<1, 0, 1>() * map(__ctx, p + e.swizzle<1, 0, 1>()).data[0] +
      e.swizzle<0, 0, 0>() * map(__ctx, p + e.swizzle<0, 0, 0>()).data[0]);
}
f32x3 render(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x3 col = COLOR_BACKGROUND;
  f32x2 res = rayMarch(__ctx, ro, rd);
  float d = res.data[0];
  if (d > MAX_DIST)
    return col;
  float id = res.data[1];
  f32x3 p = ro + rd * d;
  f32x3 normal = calcNormal(__ctx, p);
  f32x3 lightPosition = f32x3(2, 2, 7);
  f32x3 lightDirection = normalize(lightPosition - p);
  float dif = clamp(dot(normal, lightDirection), 0.3f, 1.f);
  if (id > 0.f)
    col = dif * f32x3(0.7f * mod(floor(p.data[0]) + floor(p.data[2]), 2.0f),
                      .5f, .5f);
  if (id > 1.f)
    col = dif * f32x3(0, .8f, .8f);
  if (id > 2.f)
    col = dif * f32x3(1, 0.58f, 0.29f);
  if (id > 3.f)
    col = dif *
          f32x3(1.f + 0.7f * mod(floor(p.data[0]) + floor(p.data[2]), 2.0f));
  if (id > 4.f)
    col = dif * f32x3(0, .7f, .2f);
  col += COLOR_BACKGROUND * 0.2f;
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - .5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = f32x3(10.f * sin(iTime), 0, 3);
  f32x3 rd = normalize(f32x3(uv, -1));
  f32x3 col = render(__ctx, ro, rd);
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
