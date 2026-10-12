// Port of https://www.shadertoy.com/view/dlScDy; author: awahab.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlScDy.json.
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
f32x2 points[2];
const float speed = -0.5f;
const float len = 0.25f;
const float scale = 0.012f;
float intensity = 1.3f;
float radius = 0.015f;
float sdBezier(Context &__ctx, f32x2 pos, f32x2 A, f32x2 B, f32x2 C) {
  f32x2 a = B - A;
  f32x2 b = A - 2.0f * B + C;
  f32x2 c = a * 2.0f;
  f32x2 d = A - pos;
  float kk = 1.0f / dot(b, b);
  float kx = kk * dot(a, b);
  float ky = kk * (2.0f * dot(a, a) + dot(d, b)) / 3.0f;
  float kz = kk * dot(d, a);
  float res = 0.0f;
  float p = ky - kx * kx;
  float p3 = p * p * p;
  float q = kx * (2.0f * kx * kx - 3.0f * ky) + kz;
  float h = q * q + 4.0f * p3;
  if (h >= 0.0f) {
    h = sqrt(h);
    f32x2 x = (f32x2(h, -h) - q) / 2.0f;
    f32x2 uv = sign(x) * pow(abs(x), f32x2(1.0f / 3.0f));
    float t = uv.data[0] + uv.data[1] - kx;
    t = clamp(t, 0.0f, 1.0f);
    f32x2 qos = d + (c + b * t) * t;
    res = length(qos);
  } else {
    float z = sqrt(-p);
    float v = acos(q / (p * z * 2.0f)) / 3.0f;
    float m = cos(v);
    float n = sin(v) * 1.732050808f;
    f32x3 t = f32x3(m + m, -n - m, n - m) * z - kx;
    t = clamp(t, 0.0f, 1.0f);
    f32x2 qos = d + (c + b * t.data[0]) * t.data[0];
    float dis = dot(qos, qos);
    res = dis;
    qos = d + (c + b * t.data[1]) * t.data[1];
    dis = dot(qos, qos);
    res = min(res, dis);
    qos = d + (c + b * t.data[2]) * t.data[2];
    dis = dot(qos, qos);
    res = min(res, dis);
    res = sqrt(res);
  }
  return res;
}
f32x2 getHeartPosition(Context &__ctx, float t) {
  return f32x2(16.0f * sin(t) * sin(t) * sin(t),
               -(13.0f * cos(t) - 5.0f * cos(2.0f * t) - 2.0f * cos(3.0f * t) -
                 cos(4.0f * t)));
}
float getGlow(Context &__ctx, float dist, float radius, float intensity) {
  return pow(radius / dist, intensity);
}
float getSegment(Context &__ctx, float t, f32x2 pos, float offset) {
  for (int i = 0; i < 2; i++) {
    points[i] = getHeartPosition(__ctx, offset + float(i) * len +
                                            fract(speed * t) * 6.28f);
  }
  f32x2 c = (points[0] + points[1]) / 2.0f;
  f32x2 c_prev;
  float dist = 1000.0f;
  for (int i = 0; i < 2 - 1; i++) {
    c_prev = c;
    c = (points[i] + points[i + 1]) / 2.0f;
    dist = min(dist, sdBezier(__ctx, pos, scale * c_prev, scale * points[i],
                              scale * c));
  }
  return max(0.0f, dist);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  float widthHeightRatio = iResolution.data[0] / iResolution.data[1];
  f32x2 centre = f32x2(0.5f, 0.5f);
  f32x2 pos = centre - uv;
  pos.data[1] /= widthHeightRatio;
  pos.data[1] += 0.03f;
  float t = iTime;
  float dist = getSegment(__ctx, t, pos, 0.0f);
  float glow = getGlow(__ctx, dist, radius, intensity);
  f32x3 col = f32x3(0.0f);
  col += 10.0f * f32x3(smoothstep(0.006f, 0.003f, dist));
  col += glow * f32x3(10.0f, 0.05f, 0.3f);
  dist = getSegment(__ctx, t, pos, 0.4f);
  glow = getGlow(__ctx, dist, radius, intensity);
  col += 10.0f * f32x3(smoothstep(0.006f, 0.003f, dist));
  col += glow * f32x3(0.1f, 0.4f, 1.0f);
  col = 1.0f - exp(-col);
  for (float i = 0.0f; i < 10.f; i += 1.f) {
    dist = getSegment(__ctx, t, pos * sin(iTime),
                      (pow(iMouse.data[0], sin(iTime * .001f))) * i * 10.1f);
    glow = getGlow(__ctx, dist, radius, intensity);
    col += 10.0f * f32x3(smoothstep(0.006f, 0.003f, dist));
    col += glow * f32x3(0.0f, 1.0f, 0.0f);
  }
  col = 1.0f - exp(-col);
  col = pow(col, f32x3(0.4545f));
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
