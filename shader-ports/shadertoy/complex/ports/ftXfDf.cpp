// Port of https://www.shadertoy.com/view/ftXfDf; author: artemis.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ftXfDf.json.
#include "../../support.h"
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
float sRGB(Context &__ctx, float x) {
  if (x <= 0.00031308f)
    return 12.92f * x;
  else
    return 1.055f * pow(x, (1.0f / 2.4f)) - 0.055f;
}
float saw(Context &__ctx, float t) {
  t = t / 3.141592f / 2.0f;
  return t - floor(t);
}
float tri(Context &__ctx, float t) { return 2.0f * abs(t - floor(t + 0.5f)); }
float tri2(Context &__ctx, float t) {
  t = t / 3.141592f / 2.0f;
  return 2.0f * abs(t - floor(t + 0.5f));
}
float grid(Context &__ctx, f32x2 pix, float t) {
  float d = t + (pix.data[1] / 30.0f) + sin(pix.data[0] * 500.0f);
  float distortion =
      (sin(d * 400.0f) + (sin(d * 600.0f) / 2.0f) + (sin(d * 800.0f) / 3.0f)) /
      4.0f;
  float w = tri(__ctx, (pix.data[0] + distortion) / 50.0f);
  float h = tri(__ctx, (pix.data[1] + distortion) / 50.0f);
  float power = 1.0f + 0.25f * tri(__ctx, pix.data[1] / 50.0f + t / 6.0f);
  float falloff = power * 0.75f;
  float dist = min(w, h) * 25.0f;
  return min(power, power * (falloff * falloff) / (dist * dist));
}
float electron_beam(Context &__ctx, f32x2 pix, float t, float toff) {
  float beam_x =
      (saw(__ctx, t * 4.0f + toff) * 400.0f - 200.0f +
       saw(__ctx, t / -3.0f * tri2(__ctx, t / 8.0f)) * 200.0f - 100.0f) /
      2.0f;
  float beam_y =
      (saw(__ctx, t * 3.0f - toff) * 400.0f - 200.0f +
       saw(__ctx, t / -2.0f * tri2(__ctx, t / 6.0f)) * 200.0f - 100.0f) /
      2.0f;
  float dist = distance(pix, f32x2(beam_x, beam_y));
  float power = 1.0f - (min(iTime - t, 20.0f) / 20.0f);
  float falloff = power * 1.0f;
  return min(power, power * (falloff * falloff) / (dist * dist));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 center = iResolution.swizzle<0, 1>() / 2.0f;
  float scale = 386.0f / iResolution.data[0];
  f32x2 uv = (fragCoord - center) * scale;
  float beam = 0.0f;
  f32x3 beamzz = f32x3(0.0f);
  float t = 0.0f;
  for (int i = 0; i < 2000; i++) {
    float bbbb = electron_beam(__ctx, uv, iTime - t, iTime / 2.0f + sin(t));
    float r = sin(float(i) / 50.0f) + 1.0f / 2.0f;
    float g = cos(float(i) / 75.0f) + 1.0f / 2.0f;
    float b = cos(float(i) / 25.0f) + 1.0f / 2.0f;
    beamzz += f32x3(bbbb * r, bbbb * g, bbbb * b);
    t += 0.01f;
  }
  f32x2 uv_r = uv - f32x2(sin(iTime / 5.0f + uv.data[1] / 100.0f) * 2.0f,
                          sin(uv.data[0] / 75.0f + iTime / 8.0f));
  f32x2 uv_g = uv;
  f32x2 uv_b = uv + f32x2(sin(iTime / 4.0f + uv.data[1] / 100.0f) * 2.0f,
                          cos(uv.data[0] / 50.0f + iTime / 4.0f));
  float grid_r = grid(__ctx, uv_r, iTime) * 0.672443156957688f;
  float grid_g = grid(__ctx, uv_g, iTime - 0.1f) * 0.0103298230296269f;
  float grid_b = grid(__ctx, uv_b, iTime + 0.1f) * 0.246201326707835f;
  f32x3 grid_col =
      f32x3(sRGB(__ctx, grid_r), sRGB(__ctx, grid_g), sRGB(__ctx, grid_b));
  f32x3 beam_col =
      f32x3(sRGB(__ctx, beamzz.data[0]), sRGB(__ctx, beamzz.data[1]),
            sRGB(__ctx, beamzz.data[2]));
  fragColor = f32x4(grid_col + beam_col, 1.0f);
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
