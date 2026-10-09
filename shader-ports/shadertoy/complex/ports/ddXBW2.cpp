// Port of https://www.shadertoy.com/view/ddXBW2; author: GabeRundlett.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/ddXBW2.json.
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
f32x2 circle_pos;
f32x2 rect_size;
f32x2 noise_offset_a;
f32x2 noise_offset_b;
f32x2 noise_offset_c;
const float MAX_DIST = 1.0e9f;
const float SQRT_2 = 1.41421356237f;
float nonzero_sign(Context &__ctx, float x) {
  if (x < 0.0f)
    return -1.0f;
  return 1.0f;
}
f32x2 nonzero_sign(Context &__ctx, f32x2 x) {
  return f32x2(nonzero_sign(__ctx, x.data[0]), nonzero_sign(__ctx, x.data[1]));
}
float sd_union(Context &__ctx, float a, float b) { return min(a, b); }
float sd_add(Context &__ctx, float a, float b) { return (a + b); }
float sd_mul(Context &__ctx, float a, float b) { return (a * b); }
float sd_circle_nearest(Context &__ctx, f32x2 p, float r) {
  return length(p) - r;
}
float sd_circle_furthest(Context &__ctx, f32x2 p, float r) {
  return length(p) + r * 2.0f;
}
float sd_box_nearest(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f));
}
float sd_box_furthest(Context &__ctx, f32x2 p, f32x2 b) {
  return length(b * nonzero_sign(__ctx, p) + p);
}
f32x2 minmax_sd_circle_in_region(Context &__ctx, f32x2 region_center,
                                 f32x2 region_size, float r) {
  float min_d = sd_box_nearest(__ctx, -region_center, region_size);
  float max_d = sd_box_furthest(__ctx, -region_center, region_size);
  return f32x2(min_d, max_d) - r;
}
f32x2 minmax_gradient_y_in_region(Context &__ctx, f32x2 region_center,
                                  f32x2 region_size, float f) {
  float min_d = (region_center.data[1] - region_size.data[1]) * f;
  float max_d = (region_center.data[1] + region_size.data[1]) * f;
  return f32x2(min_d, max_d);
}
float hash(Context &__ctx, float n) { return fract(sin(n) * 753.5453123f); }
f32x4 noised(Context &__ctx, f32x3 x) {
  f32x3 p = floor(x);
  f32x3 w = fract(x);
  f32x3 u = w * w * (3.0f - 2.0f * w);
  f32x3 du = 6.0f * w * (1.0f - w);
  float n = p.data[0] + p.data[1] * 157.0f + 113.0f * p.data[2];
  float a = hash(__ctx, n + 0.0f);
  float b = hash(__ctx, n + 1.0f);
  float c = hash(__ctx, n + 157.0f);
  float d = hash(__ctx, n + 158.0f);
  float e = hash(__ctx, n + 113.0f);
  float f = hash(__ctx, n + 114.0f);
  float g = hash(__ctx, n + 270.0f);
  float h = hash(__ctx, n + 271.0f);
  float k0 = a;
  float k1 = b - a;
  float k2 = c - a;
  float k3 = e - a;
  float k4 = a - b - c + d;
  float k5 = a - c - e + g;
  float k6 = a - b - e + f;
  float k7 = -a + b + c - d + e - f - g + h;
  return f32x4(
      k0 + k1 * u.data[0] + k2 * u.data[1] + k3 * u.data[2] +
          k4 * u.data[0] * u.data[1] + k5 * u.data[1] * u.data[2] +
          k6 * u.data[2] * u.data[0] + k7 * u.data[0] * u.data[1] * u.data[2],
      du * (f32x3(k1, k2, k3) + u.swizzle<1, 2, 0>() * f32x3(k4, k5, k6) +
            u.swizzle<2, 0, 1>() * f32x3(k6, k4, k5) +
            k7 * u.swizzle<1, 2, 0>() * u.swizzle<2, 0, 1>()));
}
float sd_noise_conservative(Context &__ctx, f32x2 p, float scale, float amp) {
  float noise_val = noised(__ctx, f32x3(p * scale, 0.0f)).data[0] * 2.0f - 1.0f;
  return noise_val * amp;
}
f32x2 minmax_noise_in_region(Context &__ctx, f32x2 region_center,
                             f32x2 region_size, float scale, float amp) {
  float lipschitz = 2.0f * 1.5f * scale * amp;
  float noise_val = sd_noise_conservative(__ctx, region_center, scale, amp);
  float max_dist = length(region_size);
  return f32x2(max(noise_val - max_dist * lipschitz, -amp),
               min(noise_val + max_dist * lipschitz, amp));
}
float map(Context &__ctx, f32x2 p) {
  float d = MAX_DIST;
  d = sd_union(__ctx, d, 5.0f);
  d = sd_add(__ctx, d,
             sd_noise_conservative(__ctx, p - noise_offset_a, 0.01f, 7.5f));
  d = sd_add(__ctx, d,
             sd_noise_conservative(__ctx, p - noise_offset_b, 0.15f, 1.1f));
  d = sd_add(__ctx, d,
             sd_noise_conservative(__ctx, p - noise_offset_c, 0.60f, 0.3f));
  d = sd_add(__ctx, d, p.data[1] * 0.17f);
  d = sd_union(__ctx, d, sd_circle_nearest(__ctx, p - circle_pos, 40.0f));
  d = sd_union(__ctx, d,
               sd_box_nearest(__ctx, p - circle_pos - f32x2(0.0f, -10.0f),
                              f32x2(50.0f, 10.0f)));
  d = sd_union(__ctx, d,
               sd_box_nearest(__ctx, p - f32x2(80.0f, 0.0f), rect_size));
  return d;
}
f32x2 map_at_lod(Context &__ctx, f32x2 p, unsigned lod_level) {
  float chunk_edge_length_at_lod = float(1u << lod_level);
  f32x2 lod_chunk_center_p =
      (floor(p / chunk_edge_length_at_lod) + 0.5f) * chunk_edge_length_at_lod;
  float chunk_diag_length_at_lod = SQRT_2 * 0.5f * chunk_edge_length_at_lod;
  float min_d;
  float max_d;
  bool first = true;
  {
    float d = 5.0f;
    if (first) {
      min_d = d;
      max_d = d;
    } else {
      min_d = sd_union(__ctx, min_d, d);
      max_d = sd_union(__ctx, max_d, d);
    }
    first = false;
  }
  {
    f32x2 d = minmax_noise_in_region(__ctx, lod_chunk_center_p - noise_offset_a,
                                     f32x2(chunk_edge_length_at_lod * 0.5f),
                                     0.01f, 7.5f);
    if (first) {
      min_d = d[0];
      max_d = d[1];
    } else {
      min_d = sd_add(__ctx, min_d, d[0]);
      max_d = sd_add(__ctx, max_d, d[1]);
    }
    first = false;
  }
  {
    f32x2 d = minmax_noise_in_region(__ctx, lod_chunk_center_p - noise_offset_b,
                                     f32x2(chunk_edge_length_at_lod * 0.5f),
                                     0.15f, 1.1f);
    if (first) {
      min_d = d[0];
      max_d = d[1];
    } else {
      min_d = sd_add(__ctx, min_d, d[0]);
      max_d = sd_add(__ctx, max_d, d[1]);
    }
    first = false;
  }
  {
    f32x2 d = minmax_noise_in_region(__ctx, lod_chunk_center_p - noise_offset_c,
                                     f32x2(chunk_edge_length_at_lod * 0.5f),
                                     0.60f, 0.3f);
    if (first) {
      min_d = d[0];
      max_d = d[1];
    } else {
      min_d = sd_add(__ctx, min_d, d[0]);
      max_d = sd_add(__ctx, max_d, d[1]);
    }
    first = false;
  }
  {
    f32x2 d = minmax_gradient_y_in_region(
        __ctx, lod_chunk_center_p, f32x2(chunk_edge_length_at_lod * 0.5f),
        0.17f);
    if (first) {
      min_d = d[0];
      max_d = d[1];
    } else {
      min_d = sd_add(__ctx, min_d, d[0]);
      max_d = sd_add(__ctx, max_d, d[1]);
    }
    first = false;
  }
  {
    f32x2 d = minmax_sd_circle_in_region(__ctx, lod_chunk_center_p - circle_pos,
                                         f32x2(chunk_edge_length_at_lod * 0.5f),
                                         40.0f);
    if (first) {
      min_d = d[0];
      max_d = d[1];
    } else {
      min_d = sd_union(__ctx, min_d, d[0]);
      max_d = sd_union(__ctx, max_d, d[1]);
    }
    first = false;
  }
  {
    float d = sd_box_nearest(
        __ctx, lod_chunk_center_p - circle_pos - f32x2(0.0f, -10.0f),
        f32x2(50.0f, 10.0f));
    if (first) {
      min_d = d - chunk_diag_length_at_lod;
      max_d = d + chunk_diag_length_at_lod;
    } else {
      min_d = sd_union(__ctx, min_d, d - chunk_diag_length_at_lod);
      max_d = sd_union(__ctx, max_d, d + chunk_diag_length_at_lod);
    }
    first = false;
  }
  {
    float d = sd_box_nearest(__ctx, lod_chunk_center_p - f32x2(80.0f, 0.0f),
                             rect_size);
    if (first) {
      min_d = d - chunk_diag_length_at_lod;
      max_d = d + chunk_diag_length_at_lod;
    } else {
      min_d = sd_union(__ctx, min_d, d - chunk_diag_length_at_lod);
      max_d = sd_union(__ctx, max_d, d + chunk_diag_length_at_lod);
    }
    first = false;
  }
  return f32x2(min_d, max_d);
}
bool is_map_uniform_on_lod(Context &__ctx, f32x2 p, unsigned lod_level) {
  if (lod_level == 0u)
    return true;
  f32x2 minmax = map_at_lod(__ctx, p, lod_level);
  return minmax[0] >= 0.0f || minmax[1] < 0.0f;
}
const unsigned MAX_LOD = 12u;
const unsigned LOD_STEP = 1u;
float WORLD_SCALE;
const bool USE_NAIVE_METHOD = true;
const bool SHOW_ESTIMATED_SURFACE_VOXELS = false;
const f32x3 BLUE = f32x3(0.357f, 0.612f, 0.882f);
const f32x3 ORANGE = f32x3(0.969f, 0.663f, 0.133f);
f32x2 pixel_space_to_world_space(Context &__ctx, f32x2 pixel_p) {
  float aspect = iResolution.data[0] / iResolution.data[1];
  f32x2 uv = pixel_p / iResolution.swizzle<0, 1>() * 2.0f - 1.0f;
  uv.data[0] *= aspect;
  uv *= WORLD_SCALE;
  return uv;
}
void setup_scene(Context &__ctx) {
  WORLD_SCALE = iResolution.data[1] / 20.0f * 3.0f;
  circle_pos = f32x2(-55.0f, sin(iTime * 1.4f) * 30.0f);
  if (iMouse.data[0] > 0.0f && iMouse.data[1] > 0.0f) {
    circle_pos = pixel_space_to_world_space(__ctx, iMouse.swizzle<0, 1>());
  }
  rect_size = f32x2(12.0f + sin(iTime * 2.0f) * 6.0f, 60.0f);
  noise_offset_a = f32x2(0.0f, -100.0f + sin(iTime) * 10.0f);
  noise_offset_b = f32x2(cos(iTime) * 10.0f, +100.0f);
  noise_offset_c = f32x2(cos(iTime * 2.0f) * 10.0f, -sin(iTime) * 10.0f);
}
float sd_grid(Context &__ctx, f32x2 p) {
  f32x2 uv = abs(fract(p + 0.5f) - 0.5f);
  return min(uv.data[0], uv.data[1]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  setup_scene(__ctx);
  f32x2 uv = pixel_space_to_world_space(__ctx, fragCoord);
  f32x2 voxel_pos = floor(uv) + 0.5f;
  f32x3 voxel_color = f32x3(0.0f);
  if (USE_NAIVE_METHOD) {
    float d = map(__ctx, voxel_pos);
    if (d < 0.0f) {
      voxel_color = BLUE;
    } else {
      voxel_color = ORANGE;
    }
  } else {
    for (unsigned lod_i = 0u; lod_i <= MAX_LOD; lod_i += LOD_STEP) {
      unsigned lod_level = MAX_LOD - lod_i;
      if (is_map_uniform_on_lod(__ctx, voxel_pos, lod_level)) {
        float d = map(__ctx, voxel_pos);
        if (d < 0.0f) {
          voxel_color = BLUE;
        } else {
          voxel_color = ORANGE;
        }
        break;
      }
    }
  }
  f32x3 color = voxel_color;
  if (SHOW_ESTIMATED_SURFACE_VOXELS) {
    unsigned lod_i = MAX_LOD;
    unsigned lod_level = MAX_LOD - lod_i;
    float lod_scl = float(1u << lod_level);
    if (is_map_uniform_on_lod(__ctx, uv, lod_level + 1u)) {
      color *= 0.0f;
    } else {
      float grid_lines_fac = smoothstep(0.000f, 0.25f / float(lod_scl),
                                        sd_grid(__ctx, uv / lod_scl)) *
                                 0.4f +
                             0.6f;
      color *= grid_lines_fac;
    }
  } else {
    for (unsigned lod_i = 0u; lod_i <= MAX_LOD; lod_i += LOD_STEP) {
      unsigned lod_level = MAX_LOD - lod_i;
      float lod_scl = float(1u << lod_level);
      if (is_map_uniform_on_lod(__ctx, uv, lod_level)) {
        float grid_lines_fac = smoothstep(0.000f, 0.25f / float(lod_scl),
                                          sd_grid(__ctx, uv / lod_scl)) *
                                   0.4f +
                               0.6f;
        color *= grid_lines_fac;
        break;
      }
    }
  }
  fragColor = f32x4(color, 1.0f);
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
