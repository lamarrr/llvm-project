// Port of https://www.shadertoy.com/view/DdjGzw; author: jonasfrey.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DdjGzw.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

float f_n_distance(Context &__ctx, float n_x, float n_y, float n_x2,
                   float n_y2) {
  float n_delta_x = n_x - n_x2;
  float n_delta_y = n_y - n_y2;
  float n_distance_pow = pow(n_delta_x, 2.0f) + pow(n_delta_y, 2.0f);
  float n_distance_sqrt = sqrt(n_distance_pow);
  return n_distance_sqrt;
}
f32x4 f_a_n_blurred(Context &__ctx, float n_scale_x_kernel,
                    float n_scale_y_kernel, float n_x_kernel,
                    float n_y_kernel) {
  f32x4 a_n_rgba_sum = f32x4(0.0f, 0.0f, 0.0f, 0.0f);
  int n_x = 0;
  int n_y = 0;
  for (int n_y; n_y < int(n_scale_y_kernel); n_y += 1) {
    for (int n_x; n_x < int(n_scale_x_kernel); n_x += 1) {
      f32x2 n_coords_normalized_kernel =
          f32x2(float(n_x) + n_x_kernel - float(n_scale_x_kernel) / 2.0f,
                float(n_y) + n_y_kernel - float(n_scale_y_kernel) / 2.0f) /
          iResolution.swizzle<0, 1>();
      a_n_rgba_sum += texture(__ctx.channel0, n_coords_normalized_kernel);
    }
  }
  return a_n_rgba_sum / (n_scale_x_kernel * n_scale_y_kernel);
}
f32x4 f_a_n_blurred_ignore_out_of_bounds(Context &__ctx, float n_scale_x_kernel,
                                         float n_scale_y_kernel,
                                         float n_x_kernel, float n_y_kernel) {
  f32x4 a_n_rgba_sum = f32x4(0.0f, 0.0f, 0.0f, 0.0f);
  int n_x = 0;
  int n_y = 0;
  int n_count = 0;
  for (int n_y; n_y < int(n_scale_y_kernel); n_y += 1) {
    for (int n_x; n_x < int(n_scale_x_kernel); n_x += 1) {
      float n_x_target =
          float(n_x) + n_x_kernel - float(n_scale_x_kernel) / 2.0f;
      float n_y_target =
          float(n_y) + n_y_kernel - float(n_scale_y_kernel) / 2.0f;
      if (n_x_target >= 0.0f && n_x_target <= float(iResolution.data[0]) &&
          n_y_target >= 0.0f && n_y_target <= float(iResolution.data[1])) {
        f32x2 n_coords_normalized_kernel =
            f32x2(n_x_target, n_y_target) / iResolution.swizzle<0, 1>();
        a_n_rgba_sum += texture(__ctx.channel0, n_coords_normalized_kernel);
        n_count += 1;
      }
    }
  }
  return a_n_rgba_sum / float(n_count);
}
f32x4 f_a_n_blurred_ignore_out_of_bounds_gaussian(Context &__ctx,
                                                  float n_scale_x_kernel,
                                                  float n_scale_y_kernel,
                                                  float n_x_kernel,
                                                  float n_y_kernel) {
  float n_distance_to_kernel_max =
      f_n_distance(__ctx, 0.0f, 0.0f, n_scale_x_kernel, n_scale_y_kernel);
  f32x2 n_coords_kernel_normalized =
      f32x2(n_x_kernel, n_y_kernel) / iResolution.swizzle<0, 1>();
  f32x4 a_n_rgba_sum = f32x4(0.0f, 0.0f, 0.0f, 0.0f);
  int n_x = 0;
  int n_y = 0;
  int n_count = 0;
  for (int n_y; n_y < int(n_scale_y_kernel); n_y += 1) {
    for (int n_x; n_x < int(n_scale_x_kernel); n_x += 1) {
      float n_x_target =
          float(n_x) + n_x_kernel - float(n_scale_x_kernel) / 2.0f;
      float n_y_target =
          float(n_y) + n_y_kernel - float(n_scale_y_kernel) / 2.0f;
      if (n_x_target >= 0.0f && n_x_target <= float(iResolution.data[0]) &&
          n_y_target >= 0.0f && n_y_target <= float(iResolution.data[1])) {
        f32x2 n_coords_target_normalized =
            f32x2(n_x_target, n_y_target) / iResolution.swizzle<0, 1>();
        float n_distance_to_center =
            f_n_distance(__ctx, n_coords_target_normalized.data[0],
                         n_coords_target_normalized.data[1],
                         n_coords_kernel_normalized.data[0],
                         n_coords_kernel_normalized.data[1]);
        f32x2 n_coords_normalized_kernel =
            f32x2(n_x_target, n_y_target) / iResolution.swizzle<0, 1>();
        a_n_rgba_sum +=
            (texture(__ctx.channel0, n_coords_normalized_kernel)) *
            (1.0f - (n_distance_to_center / n_distance_to_kernel_max));
        n_count += 1;
      }
    }
  }
  return a_n_rgba_sum / float(n_count);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 a_n_coords_normalized_mouse =
      iMouse.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 a_n_coords_normalized_pixel =
      fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  float n_max_kernel_size = 100.0f;
  float n_kernel = n_max_kernel_size * a_n_coords_normalized_mouse.data[0];
  f32x4 a_n_rgba_blurred = f32x4(0.0f, 0.0f, 0.0f, 0.0f);
  if (a_n_coords_normalized_pixel.data[0] < 1.0f / 3.0f) {
    fragColor = f_a_n_blurred(__ctx, n_kernel, n_kernel, fragCoord.data[0],
                              fragCoord.data[1]);
    return;
  }
  if (a_n_coords_normalized_pixel.data[0] < 2.0f / 3.0f) {
    fragColor = f_a_n_blurred_ignore_out_of_bounds(
        __ctx, n_kernel, n_kernel, fragCoord.data[0], fragCoord.data[1]);
    return;
  }
  if (a_n_coords_normalized_pixel.data[0] < 3.0f / 3.0f) {
    fragColor = f_a_n_blurred_ignore_out_of_bounds_gaussian(
        __ctx, n_kernel, n_kernel, fragCoord.data[0], fragCoord.data[1]);
    return;
  }
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
