// Port of https://www.shadertoy.com/view/sdtfW7; author: mds2.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/sdtfW7.json.
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

const float max_dist = 10.0f;
const float other_side_bright = 0.25f;
float to_toroid_sdf(Context &__ctx, f32x3 pt) {
  float to_toroid_spine = length(pt.swizzle<0, 2>()) - 0.75f;
  float to_toroid = length(f32x2(to_toroid_spine, pt.data[1])) - 0.25f;
  return to_toroid;
}
float sdf(Context &__ctx, f32x3 pt) {
  float to_toroid_spine = length(pt.swizzle<0, 2>()) - 0.75f;
  float to_toroid = length(f32x2(to_toroid_spine, pt.data[1])) - 0.25f;
  float angle = atan(pt.data[0], pt.data[2]);
  f32x2 p1 = f32x2(sin(0.5f * angle), cos(0.5f * angle));
  f32x2 p2 = f32x2(cos(0.5f * angle), -sin(0.5f * angle));
  float d1 = dot(f32x2(to_toroid_spine, pt.data[1]), p1);
  float d2 = dot(f32x2(to_toroid_spine, pt.data[1]), p2);
  float max_angle_dist = min(max(d1, d2), max(-d1, -d2));
  return max(to_toroid, max_angle_dist);
}
float sdf_other_side(Context &__ctx, f32x3 pt) {
  float to_toroid_spine = length(pt.swizzle<0, 2>()) - 0.75f;
  float to_toroid = length(f32x2(to_toroid_spine, pt.data[1])) - 0.25f;
  float angle = atan(pt.data[0], pt.data[2]);
  f32x2 p1 = f32x2(sin(0.5f * angle), cos(0.5f * angle));
  f32x2 p2 = f32x2(cos(0.5f * angle), -sin(0.5f * angle));
  float d1 = dot(f32x2(to_toroid_spine, pt.data[1]), p1);
  float d2 = dot(f32x2(to_toroid_spine, pt.data[1]), p2);
  float max_angle_dist = min(max(-d1, d2), max(d1, -d2));
  return max(to_toroid, max_angle_dist);
}
float rough_raycast(Context &__ctx, f32x3 orig, f32x3 dir) {
  float accum = 0.0f;
  float d = to_toroid_sdf(__ctx, orig);
  for (int i = 0; i < 256; ++i) {
    accum = accum + 0.9f * d;
    f32x3 pt = orig + accum * dir;
    d = to_toroid_sdf(__ctx, pt);
    if (abs(d) < 1.0e-2f) {
      return accum;
    }
    if (d > max_dist) {
      return max_dist + 1.0f;
    }
  }
  return accum;
}
float raycast(Context &__ctx, f32x3 orig, f32x3 dir, float start_guess) {
  float accum = start_guess;
  if (accum > max_dist) {
    return accum;
  }
  float d = sdf(__ctx, orig + accum * dir);
  for (int i = 0; i < 256; ++i) {
    accum = accum + 0.7f * d;
    f32x3 pt = orig + accum * dir;
    d = sdf(__ctx, pt);
    if (abs(d) < 1.0e-6f) {
      return accum;
    }
    if (accum > max_dist) {
      return max_dist + 1.0f;
    }
  }
  return accum;
}
float raycast_other_side(Context &__ctx, f32x3 orig, f32x3 dir,
                         float start_guess) {
  float accum = start_guess;
  if (accum > max_dist) {
    return accum;
  }
  float d = sdf_other_side(__ctx, orig + dir * accum);
  for (int i = 0; i < 256; ++i) {
    accum = accum + 0.7f * d;
    f32x3 pt = orig + accum * dir;
    d = sdf_other_side(__ctx, pt);
    if (d < 1.0e-6f) {
      return accum;
    }
    if (d > max_dist) {
      return max_dist + 1.0f;
    }
  }
  return accum;
}
f32x3 get_norm(Context &__ctx, f32x3 pt) {
  float f = sdf(__ctx, pt);
  const float h = 1.0e-3f;
  return normalize(f32x3(sdf(__ctx, pt + f32x3(h, 0.0f, 0.0f)) - f,
                         sdf(__ctx, pt + f32x3(0.0f, h, 0.0f)) - f,
                         sdf(__ctx, pt + f32x3(0.0f, 0.0f, h)) - f));
}
f32x3 get_norm_other_side(Context &__ctx, f32x3 pt) {
  float f = sdf_other_side(__ctx, pt);
  const float h = 1.0e-3f;
  return normalize(f32x3(sdf_other_side(__ctx, pt + f32x3(h, 0.0f, 0.0f)) - f,
                         sdf_other_side(__ctx, pt + f32x3(0.0f, h, 0.0f)) - f,
                         sdf_other_side(__ctx, pt + f32x3(0.0f, 0.0f, h)) - f));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float uv_scale = min(iResolution.data[0], iResolution.data[1]);
  f32x2 uv = (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / uv_scale;
  f32x3 orig = f32x3(0.0f, 0.0f, -3.0f);
  f32x3 ray = normalize(f32x3(uv, 0) - orig);
  float theta_tilt = 2.0f * iMouse.data[1] / iResolution.data[1] - 1.0f;
  float ct = cos(theta_tilt);
  float st = sin(theta_tilt);
  f32x3x3 tilt = f32x3x3(1.0f, 0.0f, 0.0f, 0.0f, ct, -st, 0.0f, st, ct);
  float phi_yaw = 8.0f * (2.0f * iMouse.data[0] / iResolution.data[0] - 1.0f);
  float cp = cos(phi_yaw);
  float sp = sin(phi_yaw);
  f32x3x3 yaw = f32x3x3(cp, 0.0f, -sp, 0.0f, 1.0f, 0.0f, sp, 0.0f, cp);
  orig = yaw * tilt * orig;
  ray = yaw * tilt * ray;
  float bright = 0.95f;
  f32x3 overlay = f32x3(0.5f);
  f32x3 col = f32x3(0.0f);
  f32x3 light = -f32x3(0.8f * ray.data[0] + 0.6f * ray.data[2], ray.data[1],
                       0.8f * ray.data[2] - 0.6f * ray.data[0]);
  float start_guess = rough_raycast(__ctx, orig, ray);
  float dist = raycast(__ctx, orig, ray, start_guess);
  if (dist < max_dist) {
    overlay = f32x3(0.0f);
    f32x3 n = get_norm(__ctx, orig + dist * ray);
    f32x3 bounce = normalize(reflect(ray, n));
    col = 0.5f * bounce + 0.5f;
    float bounce_dot = max(0.0f, dot(bounce, light));
    bounce_dot = bounce_dot * bounce_dot;
    bounce_dot = bounce_dot * bounce_dot;
    col += 0.5f * bounce_dot;
    bright = 1.0f;
  }
  float dist_other_side = raycast_other_side(__ctx, orig, ray, start_guess);
  if (dist_other_side < max_dist && dist_other_side < dist) {
    if (dist >= max_dist) {
      overlay = f32x3(0.0f);
    }
    bright = 1.0f - other_side_bright;
    f32x3 hit_pt = orig + dist_other_side * ray;
    f32x3 n = get_norm_other_side(__ctx, hit_pt);
    f32x3 bounce = normalize(reflect(ray, n));
    float stripe =
        mod(atan(hit_pt.data[0], hit_pt.data[2]) / 3.141592654f, 0.1f);
    overlay += max(0.75f, dot(bounce, light)) * 0.3f * step(0.015f, stripe);
  }
  fragColor = f32x4(bright * col + overlay, 1.0f);
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
