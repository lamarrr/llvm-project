// Port of https://www.shadertoy.com/view/cl2Xzz; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cl2Xzz.json.
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
float f_n_dist_line_by_iq(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 ba = b - a;
  f32x2 pa = p - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.f, 1.f);
  return length(pa - h * ba);
}
float f_n_rnd(Context &__ctx, f32x2 o) {
  return fract(sin(dot(o.swizzle<0, 1>(), f32x2(12.9898f, 78.233f))) *
               43758.5453f);
}
float f_n_dist_euclidean(Context &__ctx, f32x2 o_p1, f32x2 o_p2) {
  f32x2 o_delta = o_p1 - o_p2;
  return sqrt(pow(o_delta.data[0], 2.f) + pow(o_delta.data[1], 2.f));
}
float f_n_dist_manhatten(Context &__ctx, f32x2 o_p1, f32x2 o_p2) {
  return abs(o_p1.data[0] - o_p2.data[0]) + abs(o_p1.data[1] - o_p2.data[1]);
}
f32x2 f_o_rnd_from_o_fc_floor(Context &__ctx, f32x2 o_fc_floor) {
  float n_rnd = f_n_rnd(__ctx, o_fc_floor);
  float n_rnd2 = f_n_rnd(__ctx, o_fc_floor * 20.f);
  f32x2 o_prnd = (f32x2(n_rnd, n_rnd2) - .5f) * sin(iTime);
  return o_prnd;
}
f32x4 f_o_voronoi(Context &__ctx, f32x2 o_fc_nor) {
  f32x2 o_fc_nor_floor = floor(o_fc_nor);
  f32x2 o_fc_nor_fract = fract(o_fc_nor);
  float n_t = iTime * 0.5f;
  f32x2 o_scl_krnl = f32x2(3.f);
  f32x2 o_scl_krnl_half_floor = floor(o_scl_krnl / 2.f);
  float n_dist_min = 1.f;
  float n_dist_min2 = 1.f;
  f32x2 o_fc_nor_min = f32x2(0.f);
  f32x2 o_trn_knrl_min = f32x2(0.f);
  for (float n_x = -o_scl_krnl_half_floor.data[0];
       n_x <= o_scl_krnl_half_floor.data[0]; n_x += 1.f) {
    for (float n_y = -o_scl_krnl_half_floor.data[1];
         n_y <= o_scl_krnl_half_floor.data[1]; n_y += 1.f) {
      f32x2 o_fc_nor_krnl_floor = f32x2(n_x, n_y) + o_fc_nor_floor;
      f32x2 o_rnd = f_o_rnd_from_o_fc_floor(__ctx, o_fc_nor_krnl_floor);
      f32x2 o_fc_nor_krnl_plus_fract = f32x2(n_x, n_y) - o_fc_nor_fract + o_rnd;
      float n_dist = length(o_fc_nor_krnl_plus_fract);
      if (n_dist < n_dist_min) {
        o_trn_knrl_min = f32x2(n_x, n_y);
        o_fc_nor_min = f32x2(n_x, n_y) + o_rnd;
        n_dist_min2 = n_dist_min;
        n_dist_min = n_dist;
      }
      if (n_dist > n_dist_min && n_dist < n_dist_min2) {
        n_dist_min2 = n_dist;
      }
    }
  }
  return f32x4(n_dist_min, n_dist_min2, o_fc_nor_min);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 o_fc_nor =
      (fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * .5f) /
      iResolution.data[1];
  f32x2 o_mc_nor =
      (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * .5f) /
      iResolution.data[1];
  f32x2 o_fc_nor_nooff =
      fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 o_fc_nor_zoomed = o_fc_nor * 10.f;
  f32x4 o_voronoi = f_o_voronoi(__ctx, o_fc_nor_zoomed);
  float n_idx = float(o_fc_nor.data[0] > 0.f);
  fragColor = f32x4(o_voronoi[int(n_idx)]);
  if (o_fc_nor.data[1] > 0.f) {
    float n_min1_minus_min2 = o_voronoi.data[0] - o_voronoi.data[1];
    fragColor = f32x4(abs(sin(n_min1_minus_min2 * 6.2831f * 2.f)));
  }
  float n_radius = (1.f / 0.09f);
  float n_outline = (2.f);
  float n_dist_o_fc_nor_zoomed_point =
      length((fract(o_fc_nor_zoomed - .5f) - .5f));
  n_dist_o_fc_nor_zoomed_point =
      smoothstep(1.f, 0.5f, n_dist_o_fc_nor_zoomed_point * n_radius);
  float n_dist_o_fc_nor_zoomed_point2 = smoothstep(
      1.f, 0.5f, n_dist_o_fc_nor_zoomed_point * (n_radius + n_outline));
  fragColor *= f32x4(1.f) * (1.f - n_dist_o_fc_nor_zoomed_point);
  fragColor *= f32x4(n_dist_o_fc_nor_zoomed_point2);
  fragColor += f32x4(0.f, 1.f, 0.f, 1.f) * n_dist_o_fc_nor_zoomed_point;
  float n_dist_o_min = length(o_voronoi.data[0]);
  n_dist_o_min = smoothstep(1.f, 0.5f, n_dist_o_min * n_radius);
  float n_dist_o_min2 =
      smoothstep(1.f, 0.5f, n_dist_o_min * (n_radius + n_outline));
  fragColor *= f32x4(1.f) * (1.f - n_dist_o_min);
  fragColor *= f32x4(n_dist_o_min2);
  fragColor += f32x4(1.f, 1.f, 0.f, 1.f) * n_dist_o_min;
  f32x2 o = fract(o_fc_nor_zoomed) - f32x2(0.5f);
  float n_d_grid = max(abs(o.data[0]), abs(o.data[1]));
  n_d_grid = .5f - n_d_grid;
  float n_th = 0.01f;
  float n_aa = 1.f / iResolution.data[0];
  n_d_grid = smoothstep(n_th, n_th + n_aa, n_d_grid);
  fragColor *= f32x4(n_d_grid);
  f32x2 o_rnd = o_voronoi.swizzle<2, 3>();
  n_th = 0.005f;
  float n_its = 5.f;
  float n_dist_line = 0.f;
  for (float n_x = 0.f; n_x <= 1.f; n_x += 1.f / n_its) {
    for (float n_y = 0.f; n_y <= 1.f; n_y += 1.f / n_its) {
      float n = f_n_dist_line_by_iq(__ctx, fract(o_fc_nor_zoomed),
                                    f32x2(n_x, n_y), o_rnd);
      n_dist_line += smoothstep(n_th + n_aa, n_th, n);
    }
  }
  if (o_fc_nor.data[0] >= .0f && o_fc_nor.data[1] > 0.f) {
    fragColor = f32x4(1.f - n_dist_line);
    fragColor *= pow(o_voronoi.data[0], 1.f / 2.f);
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
