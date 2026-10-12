// Port of https://www.shadertoy.com/view/mlS3Dd; author: MysteryPancake.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlS3Dd.json.
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
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

const float TAU = 6.28318530f;
float getAlpha(Context &__ctx, f32x3 col) {
  const f32x3 green = f32x3(0.0f, 1.0f, 0.0f);
  return step(0.7f, distance(col, green));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  const float dirSteps = 64.0f;
  const float radius = 32.0f;
  float distort = iMouse.data[2] > 0.0f
                      ? iMouse.data[0] / iResolution.data[0] * 2.0f - 1.0f
                      : -0.5f;
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 bg = f32x3(uv.data[0], 1.0f - uv.data[1], 1.0f) +
             smoothstep(0.4f, 0.6f, length(uv - 0.5f));
  fragColor = texture(__ctx.channel0, uv);
  float alpha = getAlpha(__ctx, fragColor.swizzle<0, 1, 2>());
  f32x2 aspect = 1.0f / f32x2(textureSize(__ctx.channel0, 0));
  f32x2 dirAvg = f32x2(0.0f);
  bool interior = false;
  for (float i = 0.0f; i < TAU; i += TAU / dirSteps) {
    f32x2 dir = f32x2(sin(i), cos(i));
    f32x4 col = texture(__ctx.channel0, uv + dir * aspect * radius);
    float neighbor = getAlpha(__ctx, col.swizzle<0, 1, 2>());
    if (alpha != neighbor) {
      dirAvg += dir;
      interior = alpha > neighbor;
    }
  }
  if (length(dirAvg) <= 0.0f) {
    fragColor.set_swizzle<0, 1, 2>(
        mix(bg, fragColor.swizzle<0, 1, 2>(), alpha));
    return;
  }
  dirAvg = normalize(dirAvg);
  float d = radius * 0.5f;
  float move = d * 0.5f;
  for (int i = 0; i < 8; i++, move *= 0.5f) {
    f32x4 col = texture(__ctx.channel0, uv + dirAvg * aspect * d);
    d += alpha != getAlpha(__ctx, col.swizzle<0, 1, 2>()) ? -move : move;
  }
  dirAvg = interior ? dirAvg : -dirAvg;
  d = interior ? radius - d : d + radius;
  fragColor = texture(__ctx.channel0, uv + dirAvg * aspect * d * distort);
  fragColor.set_swizzle<0, 1, 2>(
      mix(bg, fragColor.swizzle<0, 1, 2>(),
          getAlpha(__ctx, fragColor.swizzle<0, 1, 2>())));
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
