// Port of https://www.shadertoy.com/view/clS3Rz; author: ZhuKai_Jul.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clS3Rz.json.
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

float weight[5] =
    float[](0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 pixed = 2.0f / iResolution.swizzle<0, 1>();
  f32x2 texcoords[24];
  texcoords[0] = uv + f32x2(-1.0f, 0.0f) * pixed;
  texcoords[1] = uv + f32x2(0.0f, -1.0f) * pixed;
  texcoords[2] = uv + f32x2(1.0f, 0.0f) * pixed;
  texcoords[3] = uv + f32x2(0.0f, 1.0f) * pixed;
  texcoords[4] = uv + f32x2(-1.0f, -1.0f) * pixed;
  texcoords[5] = uv + f32x2(-1.0f, 1.0f) * pixed;
  texcoords[6] = uv + f32x2(1.0f, -1.0f) * pixed;
  texcoords[7] = uv + f32x2(1.0f, 1.0f) * pixed;
  texcoords[8] = uv + f32x2(-2.0f, 0.0f) * pixed;
  texcoords[9] = uv + f32x2(2.0f, 0.0f) * pixed;
  texcoords[10] = uv + f32x2(0.0f, 2.0f) * pixed;
  texcoords[11] = uv + f32x2(0.0f, -2.0f) * pixed;
  texcoords[12] = uv + f32x2(-2.0f, -1.0f) * pixed;
  texcoords[13] = uv + f32x2(-2.0f, 1.0f) * pixed;
  texcoords[14] = uv + f32x2(-1.0f, -2.0f) * pixed;
  texcoords[15] = uv + f32x2(-1.0f, 2.0f) * pixed;
  texcoords[16] = uv + f32x2(1.0f, -2.0f) * pixed;
  texcoords[17] = uv + f32x2(1.0f, 2.0f) * pixed;
  texcoords[18] = uv + f32x2(2.0f, -1.0f) * pixed;
  texcoords[19] = uv + f32x2(2.0f, 1.0f) * pixed;
  texcoords[20] = uv + f32x2(-2.0f, -2.0f) * pixed;
  texcoords[21] = uv + f32x2(-2.0f, 2.0f) * pixed;
  texcoords[22] = uv + f32x2(2.0f, -2.0f) * pixed;
  texcoords[23] = uv + f32x2(2.0f, 2.0f) * pixed;
  float w[24];
  w[0] = w[1] = w[2] = w[3] = weight[0];
  w[4] = w[5] = w[6] = w[7] = weight[1];
  w[8] = w[9] = w[10] = w[11] = weight[2];
  w[12] = w[13] = w[14] = w[15] = w[16] = w[17] = w[18] = w[19] = weight[3];
  w[20] = w[21] = w[22] = w[23] = weight[4];
  float totalWeight =
      w[0] * 4.0f + w[4] * 4.0f + w[8] * 4.0f + w[12] * 8.0f + w[20] * 4.0f;
  f32x4 gaussColor = f32x4(0.0f);
  for (int i = 0; i < 24; i++) {
    gaussColor += texture(__ctx.channel0, texcoords[i]) * w[i] / totalWeight;
  }
  f32x4 originalColor = texture(__ctx.channel0, uv);
  float normalTime = 0.7f;
  fragColor = mix(originalColor, gaussColor, normalTime);
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
