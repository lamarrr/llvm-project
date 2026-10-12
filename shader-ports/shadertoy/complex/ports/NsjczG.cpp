// Port of https://www.shadertoy.com/view/NsjczG; author: jeyko.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NsjczG.json.
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

float udSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 ba = b - a;
  f32x2 pa = p - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return length(pa - h * ba);
}
float hash11(Context &__ctx, float p) {
  p += 1.5f;
  p = fract(p * .1031f);
  p *= p + 33.33f;
  p *= p + p;
  return fract(p);
}
float hash12(Context &__ctx, f32x2 p) {
  p += 0.2512f;
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * .1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
f32x4 hash42(Context &__ctx, f32x2 p) {
  p += 1.f;
  f32x4 p4 = fract(f32x4(p.swizzle<0, 1, 0, 1>()) *
                   f32x4(.1031f, .1030f, .0973f, .1099f));
  p4 += dot(p4, p4.swizzle<3, 2, 0, 1>() + 33.33f);
  return fract((p4.swizzle<0, 0, 1, 2>() + p4.swizzle<1, 2, 2, 3>()) *
               p4.swizzle<2, 1, 3, 0>());
}
f32x4 valueNoise(Context &__ctx, f32x2 t, float w) {
  f32x2 fr = fract(t);
  f32x2 fl = floor(t);
  return mix(mix(hash42(__ctx, f32x2(fl.data[0], fl.data[1])),
                 hash42(__ctx, f32x2(fl.data[0], fl.data[1] + 1.f)),
                 smoothstep(0.f, 1.f, fr.data[1])),
             mix(hash42(__ctx, f32x2(fl.data[0] + 1.f, fl.data[1])),
                 hash42(__ctx, f32x2(fl.data[0] + 1.f, fl.data[1] + 1.f)),
                 smoothstep(0.f, 1.f, fr.data[1])),
             smoothstep(0.f, 1.f, pow(fr.data[0], abs(w))));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 col = f32x3(1);
  uv.data[1] += iTime * 0.1f + 10.f;
  for (float i = 0.f; i < 471.f; i++) {
    float ra = hash11(__ctx, i);
    float rb = hash11(__ctx, i + 1.5f);
    float rc = hash11(__ctx, i + 0.61f);
    float rd = hash11(__ctx, i + 0.21f);
    float repd = mix(0.1f, 0.2f, ra);
    f32x2 p = uv + rb * 100.f;
    if (hash11(__ctx, i + 0.15f) < 0.1f) {
      p *= f32x2x2(cos(0.5f * 3.14f), -sin(0.5f * 3.14f), sin(0.5f * 3.14f),
                   cos(0.5f * 3.14f));
      repd *= 0.5f;
    }
    p.data[0] -=
        valueNoise(__ctx, f32x2(p.data[1]) * 3.f + i, 1.f).data[0] * 0.01f;
    f32x2 id = floor(p / repd);
    p = mod(p, repd) - 0.5f * repd;
    if (hash12(__ctx, id + rc + 0.15f) < 0.36f) {
      continue;
    }
    float h = hash12(__ctx, id + rd) * repd * 0.4f;
    float sd = udSegment(__ctx, p, f32x2(0, h), f32x2(0, -h));
    sd -= valueNoise(__ctx, f32x2(p.data[1]) * 40.f + i, 1.f).data[0] * 0.001f;
    float lineWeight = hash12(__ctx, id + ra);
    lineWeight = pow(lineWeight, 5.f) * 0.15f;
    sd += smoothstep(h / 2.f, h, abs(p.data[1])) * 0.003f;
    col = mix(col, f32x3(0), smoothstep(0.0018f, 0.f, sd) * lineWeight);
  }
  col *= 0.9f;
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
