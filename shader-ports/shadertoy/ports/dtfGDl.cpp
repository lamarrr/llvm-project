// Port of https://www.shadertoy.com/view/dtfGDl; author: katayu.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtfGDl.json.
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
const unsigned k = 1103515245U;
f32x3 hash(Context &__ctx, u32x3 x) {
  x *= k;
  x = ((x >> 2u) ^ (x.swizzle<1, 2, 0>() >> 1u) ^ x.swizzle<2, 0, 1>()) * k;
  return f32x3(x) * (1.0f / float(0xffffffffU));
}
f32x4 disk(Context &__ctx, f32x2 uv, f32x2 pos, float rad, f32x3 color) {
  float d = length(pos - uv) - rad;
  float t = clamp(d, 0.0f, 1.0f);
  return f32x4(color, 1.0f - t);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor = f32x4(1.f, 1.f, 1.f, 1.0f);
  float lineWidth = 0.005f;
  float lineSpacing = 0.1f;
  f32x2 uv = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv = uv * 2.0f - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  float y = mod(uv.data[1], lineSpacing);
  if (y <= lineWidth) {
    fragColor = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
    return;
  }
  float x = mod(uv.data[0], lineSpacing);
  if (x <= lineWidth) {
    fragColor = f32x4(0.0f, 0.0f, 0.0f, 1.0f);
    return;
  }
  float dist = length(uv - f32x2(0.0f, 0.0f));
  float diameter = 0.75f;
  u32x3 p = u32x3(fragCoord, iFrame);
  f32x3 h = hash(__ctx, p);
  float sampling = h.data[0] + h.data[1] + h.data[2];
  sampling = (sampling > 3.f * 0.5f) ? 1.f : 0.f;
  f32x3 color = f32x3(1.f, 1.f, 1.f);
  color *= sampling;
  if (dist <= diameter) {
    fragColor = f32x4(color, 1.0f);
  }
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
