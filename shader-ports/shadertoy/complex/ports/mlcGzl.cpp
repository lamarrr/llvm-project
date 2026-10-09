// Port of https://www.shadertoy.com/view/mlcGzl; author: volosati.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlcGzl.json.
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
  TextureCube channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const int maxDroplets = 10;
const float dropletExpandSpeed = 1.5f;
const float dropletHeightFactor = 1.f;
const float dropletRipple = 80.0f;
const float PI = 3.141f;
float getDropletHeight(Context &__ctx, f32x2 uv) {
  float time = (iTime + 1245.6789f) * 0.5f;
  f32x2 ddxy = 1.0f / iChannelResolution[0].swizzle<0, 1>();
  float height = 0.0f;
  for (int i = 0; i < maxDroplets; ++i) {
    float decayRate =
        0.323f +
        2.0f * texture(__ctx.channel0, f32x2(float(i) * ddxy.data[0], 0.237f))
                   .data[0];
    float dropletStrength =
        texture(__ctx.channel0, f32x2(float(i) * ddxy.data[0], 0)).data[0];
    float dropletStrengthBias =
        0.1f +
        0.2f * texture(__ctx.channel0, f32x2(float(i) * ddxy.data[0], 0.345f))
                   .data[0];
    float dropFraction = time / decayRate;
    float dropIndex = floor(dropFraction);
    dropFraction = fract(dropFraction);
    float dropletX =
        texture(__ctx.channel0,
                f32x2(float(i) * ddxy.data[0] + dropIndex * 0.1f, dropIndex))
            .data[0];
    float dropletY =
        texture(__ctx.channel0,
                f32x2(float(i) * ddxy.data[1] + dropIndex * 0.1f, dropIndex))
            .data[1];
    f32x2 dropletPosition;
    dropletPosition = (f32x2(dropletX, dropletY) * 2.0f - 1.0f);
    float ringRadius = dropletExpandSpeed * dropFraction * dropletStrength -
                       dropletStrengthBias;
    float distanceToDroplet = distance(uv, dropletPosition);
    float distanceToEdge =
        max(0.0f, ringRadius - distanceToDroplet) / ringRadius;
    float dropletHeight =
        distanceToDroplet > ringRadius ? 0.0f : distanceToDroplet;
    dropletHeight = cos(PI + (dropletHeight - ringRadius) * dropletRipple *
                                 dropletStrength) *
                        0.5f +
                    0.5f;
    dropletHeight *= 1.0f - dropFraction;
    dropletHeight *=
        distanceToDroplet > ringRadius ? 0.0f : distanceToDroplet / ringRadius;
    height += (1.0f - (cos(dropletHeight * 3.141f) + 1.0f) * 0.5f) *
              dropletHeightFactor;
  }
  return height;
}
f32x3 getDropletNormal(Context &__ctx, f32x2 uv) {
  float NORMAL_OFF = (1.0f / 128.0f);
  f32x3 off = f32x3(-NORMAL_OFF, 0, NORMAL_OFF);
  float s11 = getDropletHeight(__ctx, uv);
  float s01 =
      getDropletHeight(__ctx, f32x2(uv.swizzle<0, 1>() + off.swizzle<0, 1>()));
  float s21 =
      getDropletHeight(__ctx, f32x2(uv.swizzle<0, 1>() + off.swizzle<2, 1>()));
  float s10 =
      getDropletHeight(__ctx, f32x2(uv.swizzle<0, 1>() + off.swizzle<1, 0>()));
  float s12 =
      getDropletHeight(__ctx, f32x2(uv.swizzle<0, 1>() + off.swizzle<1, 2>()));
  f32x3 va = normalize(f32x3(off.data[2], 0.0f, s21 - s11));
  f32x3 vb = normalize(f32x3(0.0f, off.data[2], s12 - s11));
  f32x3 normal = cross(va, vb);
  normal.set_swizzle<0, 1>(normal.swizzle<0, 1>() * (s11));
  return normalize(normal);
}
f32x3 generateNormalMap(Context &__ctx, f32x2 uv) {
  f32x3 d = getDropletNormal(__ctx, uv - 0.5f);
  return normalize(d);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = gl_FragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  if (2 > 0) {
    f32x3 normal = generateNormalMap(__ctx, uv);
    if (2 == 1) {
      fragColor = f32x4((normal + 1.0f) * 0.5f, 1.0f);
    } else if (2 == 2) {
      fragColor.set_swizzle<0, 1, 2>(
          texture(__ctx.channel1, normal).swizzle<0, 1, 2>());
    }
  } else {
    float h = getDropletHeight(__ctx, uv);
    fragColor = f32x4(h, h, h, 1.0f);
  }
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_cube<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
