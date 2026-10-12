// Port of https://www.shadertoy.com/view/Dlf3DX; author: lanx06.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/Dlf3DX.json.
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
float sin_lerp(Context &__ctx, float a, float b, float t) {
  float tem = (1.0f - cos(t * 3.1415926538f)) / 2.0f;
  return a * (1.0f - tem) + b * tem;
}
float random(Context &__ctx, float seed) {
  return fract(sin(seed + 1.f) * 43758.5453f);
}
float curl_random(Context &__ctx, float seed, float offset, float freq) {
  return sin_lerp(__ctx, random(__ctx, offset + floor(mod(seed, freq))),
                  random(__ctx, offset + floor(mod(seed + 1.f, freq))),
                  abs(seed - floor(seed)));
}
f32x2 dir4[4] = f32x2[](f32x2(1.0f, 0.0f), f32x2(0.0f, 1.0f),
                        f32x2(0.0f, -1.0f), f32x2(-1.0f, 0.0f));
f32x2 dir2_1[4] = f32x2[](f32x2(1.0f, 1.0f), f32x2(1.0f, -1.0f),
                          f32x2(-1.0f, -1.0f), f32x2(-1.0f, 1.0f));
f32x2 dir8[8] =
    f32x2[](f32x2(1.0f, 0.0f), f32x2(1.0f, 1.0f), f32x2(0.0f, 1.0f),
            f32x2(1.0f, -1.0f), f32x2(0.0f, -1.0f), f32x2(-1.0f, -1.0f),
            f32x2(-1.0f, 0.0f), f32x2(-1.0f, 1.0f));
f32x2 dir4_1[4] = f32x2[](f32x2(1.0f, 1.0f), f32x2(-1.0f, -1.0f),
                          f32x2(1.0f, -1.0f), f32x2(-1.0f, 1.0f));
float shape(Context &__ctx, f32x2 uv, f32x2 dir) {
  f32x2 l = uv - 0.5f;
  l += dir;
  float pix = 0.02f;
  float thin = 0.05f;
  float radius = 0.60f;
  float offset = curl_random(__ctx, iTime, 0.f, 100.0f);
  radius += (offset - 0.5f) * radius;
  float a = smoothstep(pix, -pix, abs(length(l) - radius) - thin);
  radius = 1.0f - radius;
  float c = smoothstep(radius + thin + pix, radius + thin - pix, length(l));
  float d = smoothstep(radius - thin + pix, radius - thin - pix, length(l));
  return max(a, c - d);
}
float shape_4(Context &__ctx, f32x2 st, int type) {
  float color = 0.0f;
  if (type < 1)
    for (int i = 0; i < 2; i++)
      color = max(color, shape(__ctx, st, dir4_1[i] / 2.0f));
  else if (type < 2)
    for (int i = 2; i < 4; i++)
      color = max(color, shape(__ctx, st, dir4_1[i] / 2.0f));
  else
    for (int i = 0; i < 4; i++)
      color = max(color, shape(__ctx, st, dir4_1[i] / 2.0f));
  return color;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 st = fragCoord / iResolution.data[1];
  f32x3 color = f32x3(0.0f);
  float tem = 0.0f;
  float n = 50.0f;
  st = fract(fragCoord / n);
  f32x2 id = floor(fragCoord / n);
  float v =
      curl_random(__ctx, iTime * 0.03f, id.data[0] + id.data[1] * n, 1000.0f);
  tem = shape_4(__ctx, st, int(v * 3.0f));
  color = mix(f32x3(0.058f, 0.96f, 0.678f), f32x3(1, 0, 0.38f), tem);
  fragColor = f32x4(color, 1.0f);
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
