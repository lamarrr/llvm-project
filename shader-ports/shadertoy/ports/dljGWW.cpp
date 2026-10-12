// Port of https://www.shadertoy.com/view/dljGWW; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dljGWW.json.
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

f32x2 f_o_p(Context &__ctx, float n_radians, float n_radius) {
  f32x2 o_p = f32x2(sin(n_radians) * n_radius, cos(n_radians) * n_radius);
  return o_p;
}
f32x4 f_o_less_efficient_way(Context &__ctx, f32x2 o_trn_fc_nor_offset,
                             float n_its, float n_radius) {
  float n_tau = 6.283158f;
  float n_dmin = 1.f;
  float n_it_dmin = 0.f;
  float n_ang_nor =
      atan(o_trn_fc_nor_offset.data[0], o_trn_fc_nor_offset.data[1]) / n_tau +
      0.5f;
  float n_dsum = 0.f;
  float n_dprod = 1.f;
  for (float n_it = 0.f; n_it < n_its; n_it += 1.f) {
    float n_it_nor = n_it / n_its;
    if (n_it == 0.f) {
      n_radius = 0.f;
    }
    f32x2 o_p = f_o_p(__ctx, n_it_nor * n_tau, n_radius);
    float n_d = length(o_trn_fc_nor_offset - o_p);
    if (n_d < n_dmin) {
      n_dmin = n_d;
      n_it_dmin = n_it;
      if (n_it == 0.f) {
        n_it_dmin = -1.f;
      }
    }
    n_dsum += n_d;
    n_dprod *= n_d;
  }
  return f32x4(n_dmin, n_it_dmin / n_its, n_dsum, n_dprod);
}
f32x4 f_o_more_efficient(Context &__ctx, f32x2 o_trn_fc_nor_offset, float n_its,
                         float n_radius) {
  float n_tau = 6.283158f;
  float n_dmin = 1.f;
  float n_it_dmin = 0.f;
  float n_ang_nor =
      (atan(o_trn_fc_nor_offset.data[0], o_trn_fc_nor_offset.data[1]) / n_tau) +
      0.5f;
  float n_one_it_nor = (1.f / n_its);
  float n_it = floor(n_ang_nor / n_one_it_nor);
  float n_it_nor = n_it / n_its;
  float n_max = 3.f;
  float n_dsum = 0.f;
  float n_dprod = 1.f;
  for (float n = 0.f; n < n_max; n += 1.f) {
    float n_pm = (n - 1.f);
    float n_radians =
        ((n_it_nor) + (n_pm * n_one_it_nor)) * n_tau + n_tau / 2.f;
    f32x2 o_p = f_o_p(__ctx, n_radians, n_radius);
    float n_d = length(o_trn_fc_nor_offset - o_p);
    if (n_d < n_dmin) {
      n_dmin = n_d;
      n_it_dmin = mod(n_it + n_pm, n_its);
    }
    n_dsum += n_d;
    n_dprod *= n_d;
  }
  float n_d = length(o_trn_fc_nor_offset - f32x2(0.f));
  if (n_d < n_dmin) {
    n_dmin = n_d;
    n_it_dmin = -1.f;
  }
  n_dsum += n_d;
  n_dprod *= n_d;
  return f32x4(n_dmin, n_it_dmin / n_its, n_dsum, n_dprod);
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
  float n_its = floor((sin(iTime) * 0.5f + 0.5f) * 9.f + 3.f);
  f32x4 o_dvonoroipoly;
  float n_radius = 0.5f;
  o_dvonoroipoly =
      f_o_less_efficient_way(__ctx, o_trn_fc_nor_offset, n_its, n_radius);
  o_dvonoroipoly =
      f_o_more_efficient(__ctx, o_trn_fc_nor_offset, n_its, n_radius);
  fragColor = f32x4(o_dvonoroipoly.data[1]);
  float b_polygon = float(o_dvonoroipoly.data[1] < 0.f);
  fragColor = f32x4(b_polygon);
  if (o_trn_fc_nor.data[1] > 1.f / 3.f * 1.f) {
    fragColor = f32x4(o_dvonoroipoly.data[0]);
  }
  if (o_trn_fc_nor.data[1] > 1.f / 3.f * 2.f) {
    fragColor = f32x4(o_dvonoroipoly.data[1]);
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
