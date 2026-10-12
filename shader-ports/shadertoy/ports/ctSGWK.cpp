// Port of https://www.shadertoy.com/view/ctSGWK; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ctSGWK.json.
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
f32x2 f_o_rotd(Context &__ctx, f32x2 o, float n_angle_nor) {
  float n_tau = 6.283185f;
  float n_ang_rad = n_angle_nor * n_tau;
  return f32x2(cos(n_ang_rad) * o.data[0] - sin(n_ang_rad) * o.data[1],
               sin(n_ang_rad) * o.data[0] + cos(n_ang_rad) * o.data[1]);
}
float f_n_rand(Context &__ctx, float n) {
  n = n + n + n * .2133f;
  n = n * n * 0.231f;
  n = sin(n * 1.2324f + 123.1f * n) * 124.123f;
  return fract(n);
}
float f_n_dist_sqr(Context &__ctx, f32x2 o) {
  float n = max(abs(o.data[0]), abs(o.data[1]));
  return n;
}
float f_n_dist_crc(Context &__ctx, f32x2 o) { return length(o); }
float f_n_dist(Context &__ctx, f32x2 o_trn_fc_nor_offset, f32x2 o_scl,
               f32x2 o_trn) {
  o_trn_fc_nor_offset *= o_scl;
  float n_index = (o_trn_fc_nor_offset.data[0]) +
                  floor(o_trn_fc_nor_offset.data[1]) * o_scl.data[0];
  f32x2 o = o_trn_fc_nor_offset + floor(o_trn);
  float n_x_nor = floor(o.data[0]) / o_scl.data[0];
  float n_index_floor = floor(o.data[0]) + floor(o.data[1]) * o_scl.data[0];
  float n_index_nor = floor(n_index) / (o_scl.data[0] * o_scl.data[1]);
  float n_rand =
      f_n_rand(__ctx, iTime * 0.0001f +
                          n_index_floor / (o_scl.data[0] * o_scl.data[1])) -
      0.5f;
  f32x2 o2 = (fract(o_trn_fc_nor_offset) - (o_trn)-0.5f);
  float n_tau = 6.2831f;
  o2 = f_o_rotd(__ctx, o2, sin(iTime + n_index_nor * 0.2f * n_tau));
  float n_dc = length(o_trn_fc_nor_offset);
  float nf = fract(abs(sin(-iTime * 2.f + n_dc * 0.2f)));
  o2 *= +5.f * (nf);
  float n_dist = f_n_dist_crc(__ctx, o2);
  if (mod(floor(o_trn_fc_nor_offset.data[0]), 2.f) == 0.f) {
    n_dist = f_n_dist_sqr(__ctx, o2);
  }
  return n_dist;
}
f32x2 f_o_index(Context &__ctx, f32x2 o_trn, f32x2 o_scl) {
  float n_index_floor =
      floor(o_trn.data[0]) + floor(o_trn.data[1]) * o_scl.data[0];
  return f32x2(n_index_floor,
               n_index_floor / floor(o_scl.data[0] * o_scl.data[1]));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float n_ratio_1to_iresx = 1.f / iResolution.data[0];
  f32x2 o_trn_fc_nor = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 o_trn_mo_nor = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float n_ratio_x_to_y = iResolution.data[0] / iResolution.data[1];
  f32x2 o_trn_fc_nor_offset =
      (o_trn_fc_nor - f32x2(0.5f)) * f32x2(n_ratio_x_to_y, 1.f);
  f32x2 o_trn_mo_nor_offset =
      (o_trn_mo_nor - f32x2(0.5f)) * f32x2(n_ratio_x_to_y, 1.f);
  o_trn_fc_nor_offset *= sin(iTime * 0.2f) * 10.f;
  f32x2 o_scl = f32x2(11.f);
  f32x2 o_scl_krn = f32x2(5.f);
  float n_krn_xty = o_scl_krn.data[0] * o_scl_krn.data[1];
  float n_dmin = 1.f;
  f32x2 omin = f32x2(0.f);
  for (float n = 0.f; n < n_krn_xty; n += 1.f) {
    float n_x =
        floor(mod(n, o_scl_krn.data[0]) - floor(o_scl_krn.data[0] / 2.f));
    float n_y = floor(n / o_scl_krn.data[0]) - floor(o_scl_krn.data[1] / 2.f);
    float n_dist = f_n_dist(__ctx, o_trn_fc_nor_offset, o_scl, f32x2(n_x, n_y));
    if (n_dist < n_dmin) {
      n_dmin = n_dist;
      omin = (o_trn_fc_nor_offset * o_scl) + f32x2(n_x, n_y);
    }
  }
  fragColor = f32x4(n_dmin);
  if (o_trn_fc_nor_offset.data[0] > 0.f) {
    f32x2 o_index = f_o_index(__ctx, floor(omin), o_scl);
    float nr = f_n_rand(__ctx, abs(o_index.data[1]) * 2.f);
    float nr2 = f_n_rand(__ctx, nr);
    fragColor = f32x4(nr, nr2, 0.5f, 1.f);
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
