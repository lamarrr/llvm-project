// Port of https://www.shadertoy.com/view/NtXyDN; author: Veggiebob.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NtXyDN.json.
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
  TextureCube channel0;
  Texture3D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float align(Context &__ctx, float diff, float dim) {
  if (diff >= dim) {
    return diff - dim;
  } else {
    return 0.f;
  }
}
float box3D(Context &__ctx, f32x3 boxPos, f32x3 boxSize, f32x3 pos) {
  float n = align(__ctx, abs(pos.data[0] - boxPos.data[0]), boxSize.data[0]) +
            align(__ctx, abs(pos.data[1] - boxPos.data[1]), boxSize.data[1]);
  n += align(__ctx, abs(pos.data[2] - boxPos.data[2]), boxSize.data[2]);
  return n;
}
float sphere(Context &__ctx, f32x3 spherePos, f32x3 pos, float radius) {
  return length(pos - spherePos) - radius;
}
float sq(Context &__ctx, float x) { return x * x; }
f32x3 fractalColor(Context &__ctx, f32x3 p, float iters) {
  f32x3 c = texture(__ctx.channel1, p).swizzle<0, 1, 2>();
  for (float i = 0.f; i < iters; i++) {
    c += texture(__ctx.channel1, p * pow(2.f, i)).swizzle<0, 1, 2>() *
         pow(0.8f, i);
  }
  return c;
}
f32x2 rotate2D(Context &__ctx, f32x2 p, float angle) {
  return f32x2(p.data[0] * cos(angle) - p.data[1] * sin(angle),
               p.data[1] * cos(angle) + p.data[0] * sin(angle));
}
float smin(Context &__ctx, float a, float b, float k) {
  float h = clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
  return mix(b, a, h) - k * h * (1.0f - h);
}
float smin2(Context &__ctx, float a, float b, float k) {
  return min(a, b) - pow(0.5f, 10.f * mix(0.f, 0.1f, abs(a - b))) * k;
}
float SDF(Context &__ctx, f32x3 p) {
  float dP = 100.f;
  float dp = 1.f;
  float time = iTime + (600.f);
  for (float i = 16.f; i < 16.f * 2.f; i++) {
    float inc = i * 0.4f * sq(__ctx, sin(time * 0.01f * i * 0.1f)) * 0.5f;
    float constrict = 1.f;
    f32x3 pos =
        f32x3(sin(time + inc), cos(time + inc), sin((time + inc) * 2.f)) *
        0.1f * constrict;
    float smallSphere = sphere(__ctx, pos, p, 0.1f * 0.1f);
    dP = smin(__ctx, dP, smallSphere, 0.08f);
  }
  return dP;
}
f32x4 trace(Context &__ctx, f32x3 o, f32x3 r) {
  float t = 0.f;
  f32x3 p = o;
  for (int i = 0; i < 32; i++) {
    p = o + r * t;
    float d = SDF(__ctx, p);
    t += d * 1.f;
  }
  return f32x4(p, t);
}
f32x3 estimateNormal(Context &__ctx, f32x3 p) {
  return normalize(f32x3(
      SDF(__ctx, f32x3(p.data[0] + pow(10.f, -4.f), p.data[1], p.data[2])) -
          SDF(__ctx, f32x3(p.data[0] - pow(10.f, -4.f), p.data[1], p.data[2])),
      SDF(__ctx, f32x3(p.data[0], p.data[1] + pow(10.f, -4.f), p.data[2])) -
          SDF(__ctx, f32x3(p.data[0], p.data[1] - pow(10.f, -4.f), p.data[2])),
      SDF(__ctx, f32x3(p.data[0], p.data[1], p.data[2] + pow(10.f, -4.f))) -
          SDF(__ctx,
              f32x3(p.data[0], p.data[1], p.data[2] - pow(10.f, -4.f)))));
}
float bright(Context &__ctx, f32x3 c) { return dot(c, f32x3(1.f)) / 3.f; }
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 mouse =
      iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>() - f32x2(0.5f, 0.f);
  if (iMouse.swizzle<0, 1>() == f32x2(0.f))
    mouse = f32x2(0.0f, 0.3f);
  mouse.data[0] *= 5.f;
  mouse.data[1] *= 2.f;
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  uv -= 0.5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  float groundHeight = 0.f;
  float angle = mouse.data[0];
  float fov = 0.5f;
  float camDist = mouse.data[1] - 0.1f;
  f32x3 camera =
      f32x3(cos(mouse.data[0]) * camDist, 0.f, sin(mouse.data[0]) * camDist);
  f32x3 ray = f32x3(-cos(mouse.data[0] + uv.data[0] * fov) * 2.f, uv.data[1],
                    -sin(mouse.data[0] + uv.data[0] * fov) * 2.f);
  ray = normalize(ray);
  f32x4 v = trace(__ctx, camera, ray);
  f32x3 col = f32x3(0.f);
  f32x3 normal = estimateNormal(__ctx, v.swizzle<0, 1, 2>());
  f32x3 newRay = reflect(ray, normal);
  col = texture(__ctx.channel0, newRay).swizzle<0, 1, 2>();
  float fog = 1.f / (1.f + pow(v.data[3], 10.f) * 0.1f);
  f32x3 background = f32x3(1.f);
  col = col * fog + (1.f - fog) * background;
  fragColor = f32x4(col, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0,
               __binding(0, 1) texture_3d<f32x4> channel1,
               __binding(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
