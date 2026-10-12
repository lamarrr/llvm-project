// Port of https://www.shadertoy.com/view/dlsSz2; author: GetItemFromBlock.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dlsSz2.json.
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

float smin(Context &__ctx, float a, float b, float delta) {
  float h = clamp(0.5f + 0.5f * (a - b) / delta, 0.0f, 1.0f);
  return mix(a, b, h) - delta * h * (1.0f - h);
}
float circle(Context &__ctx, f32x2 pos, f32x2 center, float diameter) {
  return length(pos - center) > diameter ? 0.f : 1.f;
}
float circle2(Context &__ctx, f32x2 pos, f32x2 centerA, f32x2 centerB,
              float diameter, float delta) {
  float a = length(pos - centerA) - diameter;
  float b = length(pos - centerB) - diameter;
  return smin(__ctx, a, b, delta) < 0.f ? 1.0f : 0.0f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 col = f32x3(0);
  col = mix(col, f32x3(0.9f, 0.15f, 0.15f),
            circle2(__ctx, uv, f32x2(0.58f, 0.35f), f32x2(0.58f, 0.15f), 0.03f,
                    0.37f));
  col = mix(col, f32x3(0.9f, 0.15f, 0.15f),
            circle2(__ctx, uv, f32x2(0.35f, 0.65f), f32x2(0.35f, 0.45f), 0.04f,
                    0.35f));
  col = mix(
      col, f32x3(1.0f, 0.2f, 0.2f),
      circle2(__ctx, uv, f32x2(0.5f, 0.7f), f32x2(0.5f, 0.4f), 0.15f, 0.3f));
  col = mix(col, f32x3(1.0f, 0.2f, 0.2f),
            circle2(__ctx, uv, f32x2(0.42f, 0.35f), f32x2(0.42f, 0.15f), 0.03f,
                    0.37f));
  col = mix(col, f32x3(0.1f, 0.1f, 1.0f),
            circle(__ctx, uv, f32x2(0.6f, 0.65f), 0.1f));
  col = mix(col, f32x3(0.5f, 0.5f, 1.0f),
            circle(__ctx, uv, f32x2(0.55f, 0.7f), 0.015f));
  col = mix(col, f32x3(1.0f, 1.0f, 1.0f),
            circle(__ctx, uv, f32x2(0.55f, 0.7f), 0.01f));
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
