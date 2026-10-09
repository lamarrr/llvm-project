// Port of https://www.shadertoy.com/view/DtXXRB; author: afterlifevj.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtXXRB.json.
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
f32x3 hash(Context &__ctx, f32x3 p) {
  p = f32x3(dot(p, f32x3(127.1f, 311.7f, 74.7f)),
            dot(p, f32x3(269.5f, 183.3f, 246.1f)),
            dot(p, f32x3(113.5f, 271.9f, 124.6f)));
  p = -1.0f + 2.0f * fract(sin(p) * 43758.5453123f);
  return p;
}
float noise(Context &__ctx, f32x3 p) {
  f32x3 i = floor(p);
  f32x3 f = fract(p);
  f32x3 u = f * f * (3.0f - 2.0f * f);
  return mix(mix(mix(dot(hash(__ctx, i + f32x3(0.0f, 0.0f, 0.0f)),
                         f - f32x3(0.0f, 0.0f, 0.0f)),
                     dot(hash(__ctx, i + f32x3(1.0f, 0.0f, 0.0f)),
                         f - f32x3(1.0f, 0.0f, 0.0f)),
                     u.data[0]),
                 mix(dot(hash(__ctx, i + f32x3(0.0f, 1.0f, 0.0f)),
                         f - f32x3(0.0f, 1.0f, 0.0f)),
                     dot(hash(__ctx, i + f32x3(1.0f, 1.0f, 0.0f)),
                         f - f32x3(1.0f, 1.0f, 0.0f)),
                     u.data[0]),
                 u.data[1]),
             mix(mix(dot(hash(__ctx, i + f32x3(0.0f, 0.0f, 1.0f)),
                         f - f32x3(0.0f, 0.0f, 1.0f)),
                     dot(hash(__ctx, i + f32x3(1.0f, 0.0f, 1.0f)),
                         f - f32x3(1.0f, 0.0f, 1.0f)),
                     u.data[0]),
                 mix(dot(hash(__ctx, i + f32x3(0.0f, 1.0f, 1.0f)),
                         f - f32x3(0.0f, 1.0f, 1.0f)),
                     dot(hash(__ctx, i + f32x3(1.0f, 1.0f, 1.0f)),
                         f - f32x3(1.0f, 1.0f, 1.0f)),
                     u.data[0]),
                 u.data[1]),
             u.data[2]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() - .5f) / iResolution.data[1];
  float t = iTime * .25f;
  uv *= 1.2f;
  float h = noise(__ctx, f32x3(uv * 2.f, t));
  for (int n = 1; n < 5; n++) {
    float i = float(n);
    uv -= f32x2(0.7f / i * sin(i * uv.data[1] + i + t * 5.f + h * i) + 0.8f,
                0.4f / i * sin(uv.data[0] + 4.f - i + h + t * 5.f + 0.3f * i) +
                    1.6f);
  }
  uv -= f32x2(1.2f * sin(uv.data[0] + t + h) + 1.8f,
              0.4f * sin(uv.data[1] + t + 0.3f * h) + 1.6f);
  f32x3 col = f32x3(.5f * sin(uv.data[0]) + 0.5f,
                    .5f * sin(uv.data[0] + uv.data[1]) + 0.5f,
                    .5f * sin(uv.data[1]) + 0.8f) *
              0.8f;
  fragColor = f32x4(col, 1.0f);
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
