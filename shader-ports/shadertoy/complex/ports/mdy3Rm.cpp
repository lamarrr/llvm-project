// Port of https://www.shadertoy.com/view/mdy3Rm; author: spacecat.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mdy3Rm.json.
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
f32x2 rotate2D(Context &__ctx, f32x2 p, float angle) {
  f32x2x2 m = f32x2x2(cos(angle), -sin(angle), sin(angle), cos(angle));
  return p * m;
}
float hash11(Context &__ctx, float p) {
  p = fract(p * .1031f);
  p *= p + 33.33f;
  p *= p + p;
  return fract(p);
}
float hash12(Context &__ctx, f32x2 p) {
  f32x3 p3 = fract(f32x3(p.swizzle<0, 1, 0>()) * .1031f);
  p3 += dot(p3, p3.swizzle<1, 2, 0>() + 33.33f);
  return fract((p3.data[0] + p3.data[1]) * p3.data[2]);
}
float sdCircle(Context &__ctx, f32x2 p, float r) { return length(p) - r; }
float sdStar(Context &__ctx, f32x2 p, float r) {
  p = abs(p);
  if (p.data[0] > p.data[1])
    p.set_swizzle<0, 1>(p.swizzle<1, 0>());
  if (p.data[1] <= r) {
    return -sdCircle(__ctx, p - f32x2(r), r);
  } else {
    return distance(p, f32x2(0.0f, r));
  }
}
f32x3 animatedStar(Context &__ctx, f32x2 p, f32x2 id) {
  float h = hash12(__ctx, id);
  p += h - 0.5f;
  float angle = (iTime * 1000.0f) * 0.005f * (h * 2.0f - 1.75f);
  p = rotate2D(__ctx, p, angle);
  float star = sdStar(__ctx, p, h - 0.5f);
  float t =
      sin((iTime * 1000.0f) * 0.001f * (h + 0.5f) + h * 16.0f - 8.0f) * 0.2f -
      0.2f;
  float hard = 1.0f - step(t, star);
  float soft = smoothstep(t + 0.1f, t - 0.3f, star);
  f32x3 color;
  if (0 == 0) {
    color = clamp(f32x3(0.0f), f32x3(1.0f),
                  f32x3(h + 0.2f, hash12(__ctx, id + 1.0f) + 0.2f,
                        hash12(__ctx, id + 2.0f) + 0.2f));
  } else {
    color = f32x3(0.0f, 0.3f, h * 5.0f - 4.6f);
  }
  color *= hard + soft;
  return color;
}
f32x3 starField(Context &__ctx, f32x2 uv) {
  uv *= 6.0f;
  f32x2 i = floor(uv);
  f32x2 f = fract(uv);
  f32x3 col = f32x3(0.0f);
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      col += animatedStar(__ctx, f - f32x2(x, y), i + f32x2(x, y));
    }
  }
  return clamp(col, f32x3(0.0f), f32x3(1.0f));
}
float fbmClouds(Context &__ctx, f32x2 p) {
  float f = 0.0f;
  for (int i = 0; i < 3; ++i) {
    float t = (iTime * 1000.0f) * float(i) * 0.00001f;
    f32x2 direction =
        normalize(f32x2(sin(float(i) * 0.8432f - 0.1f) * -0.01f,
                        cos(float(i) * 0.2489f - 0.8425f) * -0.1f));
    float scale = float(i + 1) * 0.6f;
    float noise =
        texture(__ctx.channel0, fract((p + direction * t) * scale)).data[0];
    f += noise * 0.5f;
  }
  f *= 1.0f - p.data[1];
  f = step(0.55f, f) * 0.5f + step(0.74f, f);
  return f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 pixelatedCoord = floor(fragCoord.swizzle<0, 1>() / 4.0f) * 4.0f;
  f32x3 col = f32x3(0.0f);
  f32x2 starsUV = pixelatedCoord * 0.001f;
  for (int i = 0; i < 2; ++i) {
    starsUV *= 1.5f;
    starsUV += 0.37258f;
    float movement = (iTime * 1000.0f) * 0.00001f * float(i + 1);
    float magnitude = 1.0f / float((i + 1) * (i + 1));
    col += starField(__ctx, starsUV + f32x2(movement, 0.0f)) * magnitude;
  }
  f32x2 cloudsUV = pixelatedCoord / iResolution.swizzle<0, 1>().swizzle<0, 1>();
  cloudsUV.data[0] *=
      iResolution.swizzle<0, 1>().data[0] / iResolution.swizzle<0, 1>().data[1];
  col = clamp(col - max(0.5f - cloudsUV.data[1], 0.0f), 0.0f, 1.0f);
  float noise = fbmClouds(__ctx, cloudsUV + f32x2(0.35f, 0.05f));
  col = mix(col, f32x3(0.3f, 0.4f, 0.5f), noise);
  f32x3 colorA = 0 == 0 ? f32x3(0.0f, 0.03f, 0.05f) : f32x3(1.0f, 0.85f, 0.9f);
  f32x3 colorB = 0 == 0 ? f32x3(0.8f, 0.9f, 0.9f) : f32x3(0.15f, 0.0f, 0.2f);
  col = mix(colorA, colorB, col);
  fragColor = f32x4(col, 1.0f);
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
