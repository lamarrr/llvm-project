// Port of https://www.shadertoy.com/view/DtfSWr; author: jackwitherell.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DtfSWr.json.
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
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - f32x2(iResolution.data[0] * 0.08f, 0.0f)) /
             min(iResolution.data[0], iResolution.data[1]);
  f32x3 colblue = f32x3(4.f / 255.f, 163.f / 255.f, 230.f / 255.f);
  f32x3 colpink = f32x3(235.f / 255.f, 20.f / 255.f, 143.f / 255.f);
  colpink = f32x3((sin(iTime) / 2.0f) + 1.0f, (sin(iTime) / 2.0f) + 0.3f, 0.0f);
  colblue = f32x3(0.0f, (sin(iTime) / 2.0f) + 0.3f, (sin(iTime) / 2.0f) + 1.0f);
  f32x3 col =
      mix(f32x3(1.f, 1.f, 1.f), mix(colblue, colpink, uv.data[1]), uv.data[0]);
  f32x2 uvjames = (uv - 0.5f);
  uv -= length(uvjames) * normalize(uvjames) *
        texelFetch(__ctx.channel0,
                   i32x2((((atan(-abs(uvjames.data[0]), uvjames.data[1]) /
                            (2.0f * 3.141592f)) +
                           1.0f) /
                          1.0f) *
                             512.0f,
                         0))
            .data[0];
  uv = uv - f32x2(0.5f, 0.5f);
  if (distance(abs(uv.data[0]) + abs(uv.data[1]), 0.4f) < 0.01f)
    col = f32x3(0.0f);
  if (distance(abs(uv.data[0]) + abs(uv.data[1]), 0.45f) < 0.005f)
    col = f32x3(0.0f);
  if (distance(abs(uv.data[0]) + abs(uv.data[1]), 0.35f) < 0.005f)
    col = f32x3(0.0f);
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
