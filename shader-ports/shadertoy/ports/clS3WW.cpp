// Port of https://www.shadertoy.com/view/clS3WW; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/clS3WW.json.
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

float f_n_rand(Context &__ctx, float n) {
  n += n * 0.1338f;
  n *= n + 0.2193f;
  n = sin(n * 396.393f + n * 2.34f) * 2341.123f;
  return fract(n);
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
  o_trn_fc_nor_offset *= 2.5f;
  float n_its = 50.f;
  float n_dmin = 1.f;
  float n_it_dmin = 0.f;
  float n_dprod = 1.f;
  float n_dsqrtprod = 1.f;
  float n_dist_secondmin = 1.f;
  for (float n_it = 0.f; n_it < n_its; n_it += 1.f) {
    float n_t = floor(iTime) * 0.1f;
    f32x2 o_prand =
        f32x2((f_n_rand(__ctx, n_it + n_t) - 0.5f) *
                  (2.f + (iResolution.data[0] / iResolution.data[1])),
              (f_n_rand(__ctx, n_it * 3.123f + n_t) - 0.5f) * 2.f);
    float n_dist = length(o_trn_fc_nor_offset - o_prand);
    n_dprod *= n_dist;
    n_dsqrtprod *= pow(n_dist, 1.f / 10.f);
    if (n_dist < n_dmin) {
      n_dmin = n_dist;
      n_it_dmin = n_it;
    }
    if (n_dist < n_dist_secondmin && n_dist > n_dmin) {
      n_dist_secondmin = n_dist;
    }
  }
  float[] a_n = float[](n_dmin, sqrt(n_dmin), n_it_dmin / n_its,
                        sin(n_dmin * 6.2831f * 20.f), sqrt(n_dsqrtprod / n_its),
                        n_dprod / 100000000000.f, n_dist_secondmin);
  float n_index = o_trn_fc_nor.data[0] / (1.f / float(a_n.length()));
  float n = a_n[int(n_index)];
  fragColor = f32x4(n);
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
