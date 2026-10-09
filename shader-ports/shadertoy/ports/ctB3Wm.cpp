// Port of https://www.shadertoy.com/view/ctB3Wm; author: me_123.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctB3Wm.json.
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

const int it = 18;
const float eps = 0.001f;
float linf(Context &__ctx, f32x3 rd) {
  f32x3 k = -f32x3(1.0f) / abs(rd);
  return -max(max(k.data[0], k.data[1]), k.data[2]);
}
f32x3 transform(Context &__ctx, f32x3 p) {
  float d = 100000.f;
  int i;
  for (i = 0; i < it; i += 1) {
    p = abs(p);
    p *= 2.0f;
    p -= f32x3(1);
    p.set_swizzle<0, 1>(f32x2(abs(1.f - max(p.data[0], -p.data[1])),
                              (min(p.data[0], -p.data[1]))) +
                        f32x2(0.75f, 0.45f));
    p.data[1] = abs(p.data[1]);
    p.set_swizzle<1, 2>(
        f32x2(min(p.data[1], -p.data[2]), max(p.data[1], -p.data[2])) -
        f32x2(1.0f, 0.5f));
    p.set_swizzle<0, 2>(
        f32x2(min(p.data[0], p.data[2]), max(p.data[0], p.data[2])) - 0.9f);
  }
  return p;
}
f32x3 color(Context &__ctx, f32x3 p) {
  float d = 100000.f;
  int i;
  f32x3 v = f32x3(0);
  float ao = 0.0f;
  float v1 = 0.0f;
  for (i = 0; i < 12; i += 1) {
    p = abs(p);
    p *= 2.0f;
    p -= f32x3(1);
    p.set_swizzle<0, 1>(f32x2(abs(1.f - max(p.data[0], -p.data[1])),
                              (min(p.data[0], -p.data[1]))) +
                        f32x2(0.75f, 0.45f));
    p.data[1] = abs(p.data[1]);
    p.set_swizzle<1, 2>(
        f32x2(min(p.data[1], -p.data[2]), max(p.data[1], -p.data[2])) -
        f32x2(1.0f, 0.5f));
    p.set_swizzle<0, 2>(
        f32x2(min(p.data[0], p.data[2]), max(p.data[0], p.data[2])) - 0.9f);
    ao += length(p);
    v1 += dot(p, p);
  }
  v = p;
  f32x3 color = f32x3(1);
  if (1.8f + v.data[1] > 0.0f)
    color = f32x3(0.5f, 1.0f, 0.5f);
  if (0.6f + v.data[0] > 0.0f)
    color = f32x3(0, 1, 0.5f);
  if (-0.8f + v.data[2] > 0.0f)
    color = f32x3(0.1f, 0.1f, 1);
  return f32x3(color * (v1 - 20.0f) * 0.05f);
}
float sdf(Context &__ctx, f32x3 p) {
  p = abs(transform(__ctx, p)) - 1.0f;
  return ((max(max(p.data[0], p.data[1]), p.data[2]))) * pow(2.f, -float(it));
}
f32x3 getColor(Context &__ctx, f32x3 ro, f32x3 rd) {
  float d0 = 0.0f;
  int i;
  float m = linf(__ctx, rd);
  for (i = 0; i < 1000; i += 1) {
    float d = sdf(__ctx, ro + rd * d0) * m;
    d0 += d;
    if (d < d0 * eps || d0 > 100.f)
      break;
  }
  if (d0 > 100.f)
    return f32x3(0);
  return mix(color(__ctx, ro + rd * d0) / (float(i + 1) * 0.1f), f32x3(0),
             1.f - exp(-d0 * 5.0f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = f32x4(0);
  f32x2 uv = ((fragCoord.swizzle<0, 1>()) / iResolution.swizzle<0, 1>()) - 0.5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  uv.data[1] = -uv.data[1];
  f32x2 m = iMouse.data[2] > 0.0f
                ? ((iMouse.swizzle<0, 1>()) / iResolution.swizzle<0, 1>()) *
                      f32x2(3.141592653f * 2.f, 3.141592653f)
                : f32x2(iTime * 0.125f, 2.0f + sin(iTime * 0.25f) * 0.5f);
  f32x3 ro = f32x3(sin(m.data[1]) * cos(-m.data[0]),
                   sin(m.data[1]) * sin(-m.data[0]), cos(m.data[1])) *
             0.05f;
  f32x3 rd = -normalize(ro);
  f32x3 u = f32x3(normalize(f32x2(rd.data[1], -rd.data[0])), 0);
  f32x3 v = cross(u, rd);
  rd = normalize(1.0f * rd + u * uv.data[0] + v * uv.data[1]);
  ro -= f32x3(-0.6625000371078297f, 1.6874999810014544f, 0.4951437083081428f);
  fragColor += f32x4(getColor(__ctx, ro, rd), 1);
  fragColor = 1.f - exp(-fragColor * 3.0f);
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
