// Port of https://www.shadertoy.com/view/sdfcW7; author: Del.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdfcW7.json.
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
float dot2(Context &__ctx, f32x2 v) { return dot(v, v); }
float sdCappedCone(Context &__ctx, f32x3 p, float h, float r1, float r2) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()), p.data[1]);
  f32x2 k1 = f32x2(r2, h);
  f32x2 k2 = f32x2(r2 - r1, 2.0f * h);
  f32x2 ca = f32x2(q.data[0] - min(q.data[0], (q.data[1] < 0.0f) ? r1 : r2),
                   abs(q.data[1]) - h);
  f32x2 cb = q - k1 + k2 * clamp(dot(k1 - q, k2) / dot2(__ctx, k2), 0.0f, 1.0f);
  float s = (cb.data[0] < 0.0f && ca.data[1] < 0.0f) ? -1.0f : 1.0f;
  return s * sqrt(min(dot2(__ctx, ca), dot2(__ctx, cb)));
}
float smax(Context &__ctx, float a, float b, float k) {
  k = -k;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
f32x2x2 rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, s, -s, c);
}
float sdBox2D(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float sdHelix(Context &__ctx, f32x3 p, float radius, float height, float coils,
              float pd, float ph) {
  coils = coils / (height / 3.14159265f);
  f32x3 pp = f32x3(normalize(p.swizzle<0, 1>()) * radius,
                   clamp(p.data[2], -height * 0.5f, height * 0.5f));
  float d1 = distance(p, pp);
  float d2 =
      asin(sin(p.data[2] * coils + 0.5f * atan(p.data[0], p.data[1]))) / coils;
  float d = sdBox2D(__ctx, f32x2(d1, d2), f32x2(pd, ph));
  return d;
}
float sdTurd(Context &__ctx, f32x3 pos) {
  float radius = 2.5f;
  float height = 4.0f;
  float pd = radius - 0.35f;
  float ph = 0.01f;
  float coils = 4.0f;
  float d1 =
      sdHelix(__ctx, pos.swizzle<0, 2, 1>(), radius, height, coils, pd, ph) -
      0.02f;
  float d2 = sdCappedCone(__ctx, pos, (height) * 0.5f, radius, 0.0f) - 0.03f;
  float d = smax(__ctx, d2, -d1, 0.45f);
  return d * 0.95f;
}
float map(Context &__ctx, f32x3 pos) {
  pos.set_swizzle<0, 2>(pos.swizzle<0, 2>() *
                        (rot(__ctx, fract(iTime * 0.24f) * 6.28f)));
  return sdTurd(__ctx, pos);
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(1.0f, -1.0f) * 0.5773f;
  const float eps = 0.001f;
  return normalize(
      e.swizzle<0, 1, 1>() * map(__ctx, pos + e.swizzle<0, 1, 1>() * eps) +
      e.swizzle<1, 1, 0>() * map(__ctx, pos + e.swizzle<1, 1, 0>() * eps) +
      e.swizzle<1, 0, 1>() * map(__ctx, pos + e.swizzle<1, 0, 1>() * eps) +
      e.swizzle<0, 0, 0>() * map(__ctx, pos + e.swizzle<0, 0, 0>() * eps));
}
f32x3x3 lookAtMatrix(Context &__ctx, f32x3 from, f32x3 to) {
  f32x3 forward = normalize(to - from);
  f32x3 right = normalize(cross(forward, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 up = cross(right, forward);
  return f32x3x3(right, up, forward);
}
float raycast(Context &__ctx, f32x3 rayOrigin, f32x3 rayDirection) {
  float maximumDistance = 30.0f;
  float t = 0.0f;
  for (int i = 0; i < 150; i++) {
    if (t > maximumDistance) {
      break;
    }
    f32x3 currentPosition = rayOrigin + rayDirection * t;
    float d = map(__ctx, currentPosition);
    if (d < 0.0001f) {
      return t;
    }
    t += d;
  }
  return 0.0f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float angle = 3.14159265f * 0.4f;
  f32x3 rayOrigin = f32x3(6.0f * cos(angle), 3.0f + (sin(iTime * 0.1f) * 1.5f),
                          6.0f * sin(angle));
  f32x3 targetPosition = f32x3(0.0f);
  f32x3x3 cameraTransform = lookAtMatrix(__ctx, rayOrigin, targetPosition);
  i32x2 sampleCount = i32x2(1.0f, 1.0f);
  f32x2 uv = fragCoord;
  uv = uv / iResolution.swizzle<0, 1>();
  uv = (uv * 2.0f) - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 rayDirection = normalize(f32x3(uv, 1.5f));
  rayDirection = cameraTransform * rayDirection;
  float t = raycast(__ctx, rayOrigin, rayDirection);
  f32x3 color = f32x3((1.0f - abs(uv.data[1])) * 0.1f);
  if (t > 0.0f) {
    f32x3 position = rayOrigin + rayDirection * t;
    f32x3 n = calcNormal(__ctx, position);
    f32x3 mycol = f32x3(0.48f, 0.21f, 0.04f);
    f32x3 pos = rayOrigin + t * rayDirection;
    f32x3 nor = n;
    f32x3 dir = normalize(f32x3(1.0f, 0.7f, 0.0f));
    f32x3 ref = reflect(rayDirection, nor);
    float spe = max(dot(ref, dir), 0.0f);
    f32x3 spec = f32x3(1.0f) * pow(spe, 64.f);
    float dif = clamp(dot(nor, dir), 0.05f, 1.0f);
    color = mycol * dif;
    color += spec;
  }
  fragColor = f32x4(sqrt(color), 1.0f);
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
