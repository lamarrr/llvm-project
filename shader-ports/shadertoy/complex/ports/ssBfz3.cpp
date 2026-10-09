// Port of https://www.shadertoy.com/view/ssBfz3; author: R3N.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ssBfz3.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
struct RayMarch {
  float dist;
  float steps;
  f32x3 pos;
  f32x3 normal;
  f32x4 col;
};
float rand(Context &__ctx, f32x2 p) {
  return fract(sin(dot(p, f32x2(12.543f, 514.123f))) * 4732.12f);
}
float box(Context &__ctx, f32x2 p, f32x2 r) {
  f32x2 d = abs(p) - r;
  return length(max(d, 0.0f)) + min(max(d.data[0], d.data[1]), 0.0f);
}
float greeble(Context &__ctx, f32x3 p) {
  float r = 0.0f;
  for (int i = 0; i < 4; i++) {
    p += 2.f * rand(__ctx, floor(p.swizzle<0, 2>()));
    r += sign(sin(p.data[0]) * sin(p.data[1]) * sin(p.data[2]));
    p *= 2.0f;
  }
  return r;
}
float scene(Context &__ctx, f32x3 p) {
  float gr = 0.01f * greeble(__ctx, 0.5f * p);
  f32x3 pc = p;
  pc.data[2] = mod(pc.data[2], 40.0f) - 20.0f;
  p.set_swizzle<0, 1>(abs(p.swizzle<0, 1>()));
  p.data[0] = mod(p.data[0], 2.0f) - 1.0f;
  p.data[2] = mod(p.data[2], 2.0f) - 1.0f;
  p.data[1] = mod(p.data[1], 2.0f) - .0f;
  float r = 1.0f - p.data[1];
  r = max(r, -box(__ctx, pc.swizzle<1, 2>(), f32x2(2.5f, 10.f)));
  r = min(r, box(__ctx, p.swizzle<0, 2>(), f32x2(0.1f)));
  r = min(r, box(__ctx, p.swizzle<0, 1>() - f32x2(0, 1), f32x2(0.1f)));
  r = min(r, box(__ctx, p.swizzle<2, 1>() - f32x2(0, 1), f32x2(0.1f)));
  return 0.9f * r + gr;
}
f32x4 textureColor(Context &__ctx, f32x3 p, f32x3 n) {
  int axis = (abs(n.data[1]) >= 0.9f ? 0 : (abs(n.data[2]) >= 0.9f ? 1 : 2));
  f32x2 tc = (axis == 0 ? p.swizzle<0, 2>()
                        : (axis == 1 ? p.swizzle<0, 1>() : p.swizzle<1, 2>()));
  tc *= 0.5f;
  tc += 0.1f * p.data[2];
  tc += 0.25f * p.data[1];
  f32x4 col = texture(__ctx.channel0, tc);
  col *= col * col;
  col = f32x4(3.0f * col.data[0]);
  return clamp(col, 0.0f, 1.0f);
}
RayMarch march(Context &__ctx, f32x3 cam, f32x3 ray) {
  float dist = 0.0f;
  float d = 0.0f;
  float steps = 0.0f;
  f32x3 p;
  while (steps < 100.0f) {
    p = cam + ray * dist;
    d = scene(__ctx, p);
    dist += 0.75f * d;
    if (d < 0.001f || dist > 75.0f)
      break;
    steps++;
  }
  f32x3 n = normalize(
      d - f32x3(scene(__ctx, p - f32x2(0.01f, 0).swizzle<0, 1, 1>()),
                scene(__ctx, p - f32x2(0.01f, 0).swizzle<1, 0, 1>()),
                scene(__ctx, p - f32x2(0.01f, 0).swizzle<1, 1, 0>())));
  f32x4 col = textureColor(__ctx, p, n);
  return RayMarch(dist, steps, p, n, col);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.data[1];
  uv -= iResolution.swizzle<0, 1>() / iResolution.data[1] / 2.0f;
  f32x3 cam = f32x3(0);
  cam.data[2] += 0.5f * iTime;
  f32x3 ray = normalize(f32x3(uv.data[0], uv.data[1], 0.8f));
  float rx =
      (iMouse.data[2] >= 0.5f
           ? iMouse.data[0] / iResolution.data[0] * 2.0f * 3.14159f - 3.14159f
           : 0.25f * sin(0.2f * iTime));
  float ry =
      (iMouse.data[2] >= 0.5f
           ? iMouse.data[1] / iResolution.data[1] * 3.14159f - 3.14159f / 2.0f
           : sin(0.3f * iTime) * smoothstep(-1.0f, 1.0f, sin(0.6f * iTime)));
  ray.set_swizzle<1, 2>(ray.swizzle<1, 2>() *
                        (f32x2x2(cos(ry), sin(ry), -sin(ry), cos(ry))));
  ray.set_swizzle<0, 2>(ray.swizzle<0, 2>() *
                        (f32x2x2(cos(rx), sin(rx), -sin(rx), cos(rx))));
  ray.set_swizzle<0, 1>(ray.swizzle<0, 1>() *
                        (f32x2x2(cos(3.14159f / 2.0f), sin(3.14159f / 2.0f),
                                 -sin(3.14159f / 2.0f), cos(3.14159f / 2.0f))));
  RayMarch rm = march(__ctx, cam, ray);
  float l1 = max(dot(rm.normal, normalize(f32x3(1, -1.0f, 0))), 0.0f);
  float l2 = max(dot(rm.normal, normalize(f32x3(1, 1.0f, 0))), 0.0f);
  float l3 = max(dot(rm.normal, normalize(f32x3(0, 0, -1))), 0.0f);
  float l4 = max(dot(rm.normal, normalize(f32x3(-1, 0, 0))), 0.0f);
  float l5 = max(dot(rm.normal, normalize(f32x3(0, 0, 1))), 0.0f);
  f32x4 l = rm.col * 2.0f *
            mix(2.0f * f32x4(0.3f, 0.25f, 0.2f, 0), f32x4(0.5f), 0.75f) *
            ((l1 + l2) + 0.3f * (l3 + l5) + 0.2f * l4);
  float drk = clamp(rm.pos.data[0] / 15.0f + 1.0f, 0.0f, 1.0f);
  drk *= drk * drk;
  float lght = clamp(rm.pos.data[0] / 20.0f - 0.1f, 0.0f, 1.0f);
  float d = 1.0f - rm.dist / 75.0f;
  d *= d * d * d;
  fragColor =
      drk * l * d +
      2.5f * mix(2.0f * f32x4(0.3f, 0.25f, 0.2f, 0), f32x4(0.5f), 0.75f) * lght;
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
