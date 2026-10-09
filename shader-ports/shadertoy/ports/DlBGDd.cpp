// Port of https://www.shadertoy.com/view/DlBGDd; author: fishy.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlBGDd.json.
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
struct Context {
  Texture3D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
f32x2x2 rotMat(Context &__ctx, float r) {
  return f32x2x2(cos(r), -sin(r), sin(r), cos(r));
}
f32x3 rotX(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rotY(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 rotZ(Context &__ctx, f32x3 p, float r) {
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() * (rotMat(__ctx, r)));
  return p;
}
f32x3 getVolume(Context &__ctx, f32x3 p) {
  f32x3 a = f32x3(0);
  float g = 1.f;
  for (int i = 0; i < 4; i++) {
    a += texture(__ctx.channel0, rotY(__ctx, p * 0.01f, 1.f) / g)
             .swizzle<0, 1, 2>() *
         g;
    g *= 0.5f;
  }
  float m = smoothstep(10.f, -20.f, (p.data[1]));
  f32x3 r = pow(a * 0.9f, f32x3(4.f * (1.f - m))) * m * 3.f;
  r = mix(r, f32x3(length(r)), 0.5f);
  return r;
}
f32x3 castRay(Context &__ctx, f32x3 ro, f32x3 rd, float steps, float dist) {
  f32x3 accumulate = f32x3(0);
  f32x3 p = ro;
  float stepSize = dist / steps;
  for (float i = float(min(iFrame, 0)); i < steps; i++) {
    f32x3 volume = getVolume(__ctx, p);
    accumulate += volume * stepSize;
    p += rd * stepSize;
  }
  return accumulate / dist;
}
f32x3 renderVolume(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 lgt,
                   float primarySteps, float primaryDist, float shadowSteps,
                   float shadowDist) {
  f32x3 accumulate = f32x3(0);
  f32x3 p = ro;
  float stepSize = primaryDist / primarySteps;
  for (float i = float(min(iFrame, 0)); i < primarySteps; i++) {
    f32x3 volume = getVolume(__ctx, p);
    float shadow = length(castRay(__ctx, p, lgt, shadowSteps, shadowDist));
    accumulate += volume * stepSize / (shadow + 1.f);
    p += rd * stepSize;
  }
  return accumulate / primaryDist;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord * 2.f - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x2 m = (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * 0.5f) /
            iResolution.data[1] * step(0.01f, iMouse.data[2]);
  f32x2 rot = f32x2(0, 0) - m.swizzle<0, 1>() * 4.f;
  f32x3 ro = f32x3(0, sin(iTime * 0.5f) * 3.f, iTime * 10.f);
  f32x3 rd = rotY(__ctx, rotX(__ctx, normalize(f32x3(uv, 1.5f)), rot.data[1]),
                  rot.data[0]);
  f32x3 col =
      renderVolume(__ctx, ro, rd, f32x3(0, 1, 0), 100.f, 50.f, 5.f, 25.f);
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_3d<f32x4> channel0,
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
