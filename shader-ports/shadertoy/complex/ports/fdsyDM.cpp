// Port of https://www.shadertoy.com/view/fdsyDM; author: Del.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/fdsyDM.json.
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
f32x2x2 rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, s, -s, c);
}
float sdBox2D(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float remap(Context &__ctx, float s, float a1, float a2, float b1, float b2) {
  return b1 + (s - a1) * (b2 - b1) / (a2 - a1);
}
float smax(Context &__ctx, float a, float b, float k) {
  k = -k;
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float sdHelixSpiral(Context &__ctx, f32x3 p, float rad1, float rad2,
                    float height, float coils, float pd, float ph) {
  float rounding = 0.1f;
  float dir = sign(coils) * 0.5f;
  coils = abs(coils);
  coils = coils / (height / 3.14159265f);
  float vv = clamp(p.data[2], -height * 0.5f, height * 0.5f);
  vv = remap(__ctx, vv, -height * 0.5f, height * 0.5f, rad2, rad1);
  f32x3 pp = f32x3(normalize(p.swizzle<0, 1>()) * vv, p.data[2]);
  float d1 = distance(p, pp);
  float d2 =
      asin(sin(p.data[2] * coils + dir * atan(p.data[0], p.data[1]))) / coils;
  float d = sdBox2D(__ctx, f32x2(d1, d2), f32x2(pd, ph)) - rounding;
  float t = abs(p.data[2]) - height * 0.5f;
  d = smax(__ctx, d, t, 0.5f);
  return d;
}
float sdCappedCylinder(Context &__ctx, f32x3 p, float h, float r) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 2>()), p.data[1])) - f32x2(h, r);
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
float map(Context &__ctx, f32x3 pos) {
  pos.set_swizzle<0, 2>(pos.swizzle<0, 2>() *
                        (rot(__ctx, fract(iTime * 0.24f) * 6.28f)));
  float ns1 = 0.5f + sin(iTime * 1.1f) * 0.5f;
  float ns2 = 0.5f + sin(iTime * 0.7f) * 0.5f;
  float rad1 = 1.0f + ns1 * 2.0f;
  float rad2 = 4.0f;
  float height = 4.0f + (3.0f * ns2);
  float pd = 0.25f;
  float ph = 0.075f;
  float coils = 5.0f;
  float d1 = sdHelixSpiral(__ctx, pos.swizzle<0, 2, 1>(), rad1, rad2, height,
                           coils, pd, ph);
  return d1 * 0.7f;
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
  f32x3 rayOrigin = f32x3(9.0f * cos(angle), 6.0f, 9.0f * sin(angle));
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
    f32x3 mycol = f32x3(0.58f, 0.41f, 0.24f);
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
