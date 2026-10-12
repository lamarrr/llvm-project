// Port of https://www.shadertoy.com/view/dtSGRh; author: ersteller.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtSGRh.json.
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

uniform f32x2 u_resolution;
uniform f32x2 u_mouse;
uniform float u_time;
const int[] aiErsteller =
    int[](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
          0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
          1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0,
          0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0,
          1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0);
const f32x2 bitmap_size = f32x2(7, 15);
float random(Context &__ctx, float co) { return fract(sin(co) * 33333.f); }
f32x3 getRandomGridColor(Context &__ctx, f32x2 uv, f32x2 grid_size,
                         float bits) {
  int frac = int(fract(iTime) * 4095.f);
  f32x2 fetch_pos = f32x2(uv * grid_size);
  int idx = int(fetch_pos.data[0]) +
            (int(grid_size.data[1]) - 1 - int(fetch_pos.data[1])) *
                int(grid_size.data[0]);
  float seed = float(frac + idx);
  return f32x3(floor(random(__ctx, 3.f * seed) * (bits + 1.f)) / bits,
               floor(random(__ctx, 11.f * seed) * (bits + 1.f)) / bits,
               floor(random(__ctx, 13.f * seed) * (bits + 1.f)) / bits);
}
f32x3 getValueXY(Context &__ctx, f32x2 uv) {
  float value = 1.f;
  f32x3 col;
  f32x2 fetch_pos = f32x2(uv * bitmap_size.data[1]);
  if (fetch_pos.data[0] >= 0.f && fetch_pos.data[1] >= 0.f &&
      fetch_pos.data[0] < bitmap_size.data[0] &&
      fetch_pos.data[1] < bitmap_size.data[1]) {
    int idx = int(fetch_pos.data[0]) +
              (int(bitmap_size.data[1]) - 1 - int(fetch_pos.data[1])) *
                  int(bitmap_size.data[0]);
    value = float(1 - aiErsteller[idx]);
  } else
    value = 1.f;
  return f32x3(value);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = f32x2(fragCoord.data[0] / iResolution.data[1],
                   fragCoord.data[1] / iResolution.data[1]);
  uv.data[0] = uv.data[0] - 0.6f;
  uv.data[1] = uv.data[1] - 0.f;
  float amp = sin((uv.data[1] + iTime * 0.5f) * 5.f) - 0.95f;
  if (amp > 0.f) {
    uv.data[0] -= amp * .5f * sin((-uv.data[1] * 10.f + iTime) * 5.f) * 0.5f;
  }
  f32x3 col = getValueXY(__ctx, uv);
  if (amp > 0.f && col.data[0] == 0.f) {
    col = getRandomGridColor(__ctx, uv, f32x2(15, 15), 1.f);
  }
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
