// Port of https://www.shadertoy.com/view/mlB3Dw; author: FifthStateOfMatter.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlB3Dw.json.
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
float random3(Context &__ctx, f32x3 p) {
  p = fract(p * 0.1031f);
  p += dot(p, p.swizzle<2, 1, 0>() + 31.32f);
  return (fract((p.data[0] + p.data[1]) * p.data[2]) * 2.0f) - 1.0f;
}
float perlinNoise(Context &__ctx, f32x3 p) {
  f32x3 cellPos = floor(p);
  f32x3 cellFract = fract(p);
  f32x3 cellMix = cellFract * cellFract * (3.0f - 2.0f * cellFract);
  float value;
  f32x3 blf = f32x3(random3(__ctx, (cellPos + f32x3(0, 0, 0)) * 1.0f));
  f32x3 brf = f32x3(random3(__ctx, (cellPos + f32x3(1, 0, 0)) * 1.0f));
  f32x3 trf = f32x3(random3(__ctx, (cellPos + f32x3(1, 1, 0)) * 1.0f));
  f32x3 tlf = f32x3(random3(__ctx, (cellPos + f32x3(0, 1, 0)) * 1.0f));
  f32x3 blb = f32x3(random3(__ctx, (cellPos + f32x3(0, 0, 1)) * 1.0f));
  f32x3 brb = f32x3(random3(__ctx, (cellPos + f32x3(1, 0, 1)) * 1.0f));
  f32x3 trb = f32x3(random3(__ctx, (cellPos + f32x3(1, 1, 1)) * 1.0f));
  f32x3 tlb = f32x3(random3(__ctx, (cellPos + f32x3(0, 1, 1)) * 1.0f));
  value = mix(mix(mix(dot(cellFract - f32x3(0, 0, 0), blf),
                      dot(cellFract - f32x3(1, 0, 0), brf), cellMix.data[0]),
                  mix(dot(cellFract - f32x3(0, 1, 0), tlf),
                      dot(cellFract - f32x3(1, 1, 0), trf), cellMix.data[0]),
                  cellMix.data[1]),
              mix(mix(dot(cellFract - f32x3(0, 0, 1), blb),
                      dot(cellFract - f32x3(1, 0, 1), brb), cellMix.data[0]),
                  mix(dot(cellFract - f32x3(0, 1, 1), tlb),
                      dot(cellFract - f32x3(1, 1, 1), trb), cellMix.data[0]),
                  cellMix.data[1]),
              cellMix.data[2]);
  return value + 0.5f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float f = 30.0f;
  float h = 1.0f;
  float v = 0.5f;
  int d = 10;
  float n = perlinNoise(__ctx, f32x3(uv.data[0] * f, uv.data[1] * f,
                                     1.0f + iTime * v)) /
            h;
  for (int i = 0; i < d; i++) {
    h *= 2.0f;
    f *= 2.0f;
    v *= 2.0f;
    n += perlinNoise(__ctx, f32x3(uv.data[0] * f + float(i) * 10.0f,
                                  uv.data[1] * f + float(i) * 10.0f,
                                  1.0f + iTime * v + float(i) * 10.0f)) /
         h;
  }
  f32x3 col = f32x3(n / (float(d) / 5.0f));
  fragColor = f32x4(col, 1);
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
