// Port of https://www.shadertoy.com/view/DdBGW3; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdBGW3.json.
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
float f_n_dist_line(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  p -= a, b -= a;
  float h = clamp(dot(p, b) / dot(b, b), 0.f, 1.f);
  return length(p - b * h);
}
float f_n_dist_polygon_slower(Context &__ctx, float n_sides, float n_radius,
                              float n_radians_offset, f32x2 o_pix_cor_nor) {
  float n_tau = 6.283185f;
  float n_dist_line_lowest = 1.0f;
  for (float n_side_norm = 0.0f; n_side_norm < 1.0f;
       n_side_norm += 1.0f / n_sides) {
    float n_side_norm_next = n_side_norm + (1.0f / n_sides);
    f32x2 p1 = f32x2(sin(n_radians_offset + n_tau * n_side_norm) * n_radius,
                     cos(n_radians_offset + n_tau * n_side_norm) * n_radius);
    f32x2 p2 =
        f32x2(sin(n_radians_offset + n_tau * n_side_norm_next) * n_radius,
              cos(n_radians_offset + n_tau * n_side_norm_next) * n_radius);
    float n_dist = f_n_dist_line(__ctx, o_pix_cor_nor, p1, p2);
    if (n_dist < n_dist_line_lowest) {
      n_dist_line_lowest = n_dist;
    }
  }
  return n_dist_line_lowest;
}
float f_n_dist_polygon(Context &__ctx, float n_sides, float n_radius,
                       f32x2 o_pix_cor_nor) {
  float n_tau = 6.283185f;
  float n_dist_line_lowest = 1.0f;
  float n_center_xy = 0.0f;
  float delta_x = o_pix_cor_nor.data[0] - n_center_xy;
  float delta_y = o_pix_cor_nor.data[1] - n_center_xy;
  float n_rad_current = atan(delta_x, delta_y);
  float n_nth_line = floor(n_rad_current / (n_tau / n_sides));
  float n_radians = (n_tau / n_sides) * (n_nth_line);
  float n_radians_next = (n_tau / n_sides) * ((n_nth_line) + 1.0f);
  f32x2 p1 = f32x2(sin((n_nth_line) * (n_tau / n_sides)) * n_radius,
                   cos((n_nth_line) * (n_tau / n_sides)) * n_radius);
  f32x2 p2 =
      f32x2(sin(n_radians_next) * n_radius, cos(n_radians_next) * n_radius);
  float n_dist = f_n_dist_line(__ctx, o_pix_cor_nor, p2, p1);
  return n_dist;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 o_pixel_coord_norm =
      fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  o_pixel_coord_norm -= 0.5f;
  o_pixel_coord_norm.data[0] *= iResolution.data[0] / iResolution.data[1];
  float n_ires_min = min(iResolution.data[0], iResolution.data[1]);
  float n_ires_max = max(iResolution.data[0], iResolution.data[1]);
  f32x2 o_pix_cor_nor = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 o_mou_cor_nor = iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  o_pix_cor_nor -= 0.5f;
  o_pix_cor_nor *= f32x2(n_ires_max / n_ires_min, n_ires_min / n_ires_min);
  o_pix_cor_nor *= 2.0f;
  float n_poly_norm_lowest = 0.f;
  float n_polys = min(iResolution.data[0], iResolution.data[1]) / 20.0f +
                  430.f * o_mou_cor_nor.data[0];
  float n_dist_lowest = 1.0f;
  float n_radius_norm = 0.0f;
  for (float n_poly_norm = 0.0f; n_poly_norm < 1.0f;
       n_poly_norm += 1.0f / n_polys) {
    float n_sides = 5.0f;
    n_radius_norm += 1.0f / n_polys;
    float n_dist_polygon = f_n_dist_polygon_slower(
        __ctx, n_sides, n_radius_norm, sin(n_poly_norm * 2.0f + iTime * 2.0f),
        o_pix_cor_nor);
    if (n_dist_polygon < n_dist_lowest) {
      n_dist_lowest = n_dist_polygon;
      n_poly_norm_lowest = n_poly_norm;
    }
  }
  fragColor = f32x4(
      pow(n_dist_lowest, sin(n_poly_norm_lowest * 5.f + iTime) * 0.5f + 0.5f));
  fragColor = sqrt(fragColor);
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
