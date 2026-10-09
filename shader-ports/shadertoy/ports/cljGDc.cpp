// Port of https://www.shadertoy.com/view/cljGDc; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cljGDc.json.
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
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float n_dscl = 1.f;
  f32x2 o_lires = iResolution.swizzle<0, 1>() * n_dscl;
  f32x2 o_lfc = fragCoord.swizzle<0, 1>() * n_dscl;
  float n_ratio_1to_iresx = 1.f / o_lires.data[0];
  f32x2 o_lfc_nor = floor(o_lfc) / o_lires;
  f32x2 o_lfc_nor_off =
      (floor(o_lfc) - 0.5f * o_lires.swizzle<0, 1>()) / o_lires.data[1];
  f32x2 o_fc_nor = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 o_fc_nor_off =
      (fragCoord.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>()) /
      iResolution.data[1];
  f32x2 o_mo_nor_off =
      (iMouse.swizzle<0, 1>() - 0.5f * iResolution.swizzle<0, 1>()) /
      iResolution.data[1];
  float n_its = 4.f;
  float n_tau = 6.283185307179586f;
  float n_ang_nor =
      fract(atan(o_lfc_nor_off.data[1], o_lfc_nor_off.data[0]) / n_tau);
  float n_dcntr = length(o_lfc_nor_off);
  float n_one_it_nor = 1.f / n_its;
  float n_it = floor(n_ang_nor / n_one_it_nor);
  float n_it_nor = n_it / n_its;
  float n_nei = floor((1.f - n_dcntr) * n_its);
  f32x2 o_len_min = f32x2(1.f);
  float n_it_min = 0.f;
  float n_ang_off = 0.f;
  f32x2 o = f32x2(cos(((n_it / n_its) + n_ang_off) * n_tau) * n_dcntr,
                  sin(((n_it / n_its) + n_ang_off) * n_tau) * n_dcntr);
  float n_len = length(o_fc_nor_off - o);
  float n_len_nor = n_len / n_dcntr * n_its / 4.f;
  if (iMouse.data[2] == 0.f) {
    float n_t = iTime * 2.f;
    o_mo_nor_off = f32x2(sin(n_t), cos(n_t) * 0.5f + 0.5f);
  }
  fragColor = f32x4(n_len);
  if (o_fc_nor_off.data[0] > o_mo_nor_off.data[0]) {
    fragColor = f32x4(smoothstep(0.1f, 0.1f, n_len));
  }
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
