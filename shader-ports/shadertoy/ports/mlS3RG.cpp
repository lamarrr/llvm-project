// Port of https://www.shadertoy.com/view/mlS3RG; author: mla.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlS3RG.json.
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
  Texture2D channel3;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
bool alert = false;
void assert(Context &__ctx, bool b) {
  if (!b)
    alert = true;
}
const float PI = 3.141592654f;
const float TWOPI = 2.0f * PI;
const int CHAR_A = 65;
const int CHAR_C = 67;
const int CHAR_T = 84;
const int CHAR_X = 88;
const int CHAR_Y = 89;
const int CHAR_Z = 90;
f32x3 h2rgb(Context &__ctx, float h) {
  f32x3 rgb =
      clamp(abs(mod(h * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
            0.0f, 1.0f);
  return rgb * rgb * (3.0f - 2.0f * rgb);
}
f32x3 hsv2rgb(Context &__ctx, f32x3 c) {
  f32x3 rgb = clamp(
      abs(mod(c.data[0] * 6.0f + f32x3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f,
      0.0f, 1.0f);
  rgb = rgb * rgb * (3.0f - 2.0f * rgb);
  return c.data[2] * mix(f32x3(1.0f), rgb, c.data[1]);
}
float spiral = 2.0f;
float scale = 1.0f;
float k = 0.55f;
f32x3 palette(Context &__ctx, float t) {
  f32x3 col = t * f32x3(0.414f, 0.618f, 1);
  col = mod(col, 2.0f);
  col = min(col, 2.0f - col);
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_X), 2)).data[0] != 0.0f))
    col *= col;
  return col;
}
float wave(Context &__ctx, float t) {
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_T), 2)).data[0] != 0.0f))
    return 0.5f * (1.0f - cos(TWOPI * t));
  t *= 2.0f;
  t = mod(t, 2.0f);
  t = min(t, 2.0f - t);
  t = smoothstep(0.0f, 1.0f, t);
  return t;
}
void mainImage(Context &__ctx, f32x4 &o, f32x2 p) {
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_Z), 2)).data[0] != 0.0f) &&
      iMouse.data[0] > 1.0f)
    k = iMouse.data[0] / iResolution.data[0];
  p -= 0.5f * iResolution.swizzle<0, 1>();
  p /= scale;
  float d = length(p);
  float px = fwidth(p.data[0]);
  float angle = spiral * atan(p.data[1], p.data[0]) / TWOPI;
  float h = 0.5f * k * d / px;
  h -= 1.0f * iTime;
  h += angle;
  float h0 = h;
  h = floor(h);
  h -= angle;
  o = f32x4(0.8f);
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_C), 2)).data[0] != 0.0f))
    o.set_swizzle<0, 1, 2>(palette(__ctx, h));
  o *= wave(__ctx, h0);
  float g = 0.4545f;
  if (!(texelFetch(__ctx.channel3, i32x2((CHAR_X), 2)).data[0] != 0.0f))
    o = pow(o, f32x4(g));
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 3) texture_2d<f32x4> channel3,
               __binding__(1, 3) sampler sampling3) {
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
  Context __ctx{{channel3, sampling3}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
