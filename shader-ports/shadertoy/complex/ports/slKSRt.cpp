// Port of https://www.shadertoy.com/view/slKSRt; author: NinjaKoala.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slKSRt.json.
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
const float pi = 3.1415925f;
const int num_iterations = 6;
const int num_start_params = 32;
const int marching_steps = 10;
const int num_iterations2 = 3;
const float curve_width_sq = .003f;
const float eps = .0001f;
const int clelia_fac1 = 1;
const int clelia_fac2 = 4;
const float radius = 1.f;
const float rotation_speed = .5f;
const float clelia_fac = float(clelia_fac1) / float(clelia_fac2);
const float clelia_period = float(clelia_fac2) * 2.f * pi;
float spectral(Context &__ctx, float x) {
  return clamp(abs(mod((x / pi + 1.f) * 4.f, 8.f) - 4.f) - 2.f, -.75f, .75f) *
             .5f / .75f +
         .5f;
}
f32x3 to_col(Context &__ctx, float x) {
  return f32x3(spectral(__ctx, x), spectral(__ctx, x + pi / 2.f),
               spectral(__ctx, x - pi));
}
f32x2x2 rot(Context &__ctx, float t) {
  return f32x2x2(cos(t), -sin(t), sin(t), cos(t));
}
f32x3 parametric(Context &__ctx, float t) {
  t *= clelia_period;
  f32x3 p = radius * f32x3(cos(t) * cos(clelia_fac * t),
                           cos(t) * sin(clelia_fac * t), sin(t));
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                      (rot(__ctx, mod(rotation_speed * iTime, 2.f * pi))));
  return p;
}
f32x3 parametric_diff(Context &__ctx, float t) {
  t *= clelia_period;
  f32x3 p = clelia_period * radius *
            f32x3(-clelia_fac * cos(t) * sin(clelia_fac * t) -
                      sin(t) * cos(clelia_fac * t),
                  clelia_fac * cos(t) * cos(clelia_fac * t) -
                      sin(t) * sin(clelia_fac * t),
                  cos(t));
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() *
                      (rot(__ctx, mod(rotation_speed * iTime, 2.f * pi))));
  return p;
}
float parametric_normal_iteration3d(Context &__ctx, float t, f32x3 p0) {
  f32x3 p0_to_p = parametric(__ctx, t) - p0;
  f32x3 tang = parametric_diff(__ctx, t);
  float l_tang = dot(tang, tang);
  return t - dot(tang, p0_to_p) / l_tang;
}
float ray_to_curve_dis_sq(Context &__ctx, f32x3 ro, f32x3 rd, float t0) {
  f32x3 p0 = parametric(__ctx, t0);
  float s0 = dot(p0 - ro, rd);
  f32x3 p1 = ro + s0 * rd - p0;
  return dot(p1, p1);
}
float ray_to_curve_dis_sq_diff(Context &__ctx, f32x3 ro, f32x3 rd, float t0) {
  f32x3 p0 = parametric(__ctx, t0);
  f32x3 p0_diff = parametric_diff(__ctx, t0);
  float s0 = dot(p0 - ro, rd);
  f32x3 p1 = ro + s0 * rd - p0;
  return 2.f * dot(p1, dot(p0_diff, rd) * rd - p0_diff);
}
bool parametric_curve_newton_trace(Context &__ctx, f32x3 ro, f32x3 rd,
                                   f32x3 &p0, f32x3 &nor, float &t1) {
  float t0 = 0.f;
  float d0 = 1e38f;
  t1 = 1e38f;
  for (int i = 0; i < num_start_params; i++) {
    float t = t0;
    for (int j = 0; j < num_iterations; j++) {
      t -= (ray_to_curve_dis_sq(__ctx, ro, rd, t) - curve_width_sq) /
           ray_to_curve_dis_sq_diff(__ctx, ro, rd, t);
    }
    f32x3 p1 = parametric(__ctx, t);
    float d1 = dot(p1 - ro, rd);
    if (abs(ray_to_curve_dis_sq(__ctx, ro, rd, t) - curve_width_sq) < eps &&
        d1 < d0) {
      t1 = t;
      d0 = d1;
    }
    t0 += 1.f / float(num_start_params - 1);
  }
  if (t1 != 1e38f) {
    f32x3 p1 = ro + d0 * rd;
    for (int i = 0; i < marching_steps; i++) {
      for (int j = 0; j < num_iterations2; j++) {
        t1 = parametric_normal_iteration3d(__ctx, t1, p1);
      }
      p0 = parametric(__ctx, t1);
      d0 = dot(p0 - ro, rd);
      p1 = ro + d0 * rd;
    }
    p1 -= rd * sqrt(curve_width_sq - dot(p1 - p0, p1 - p0));
    nor = normalize(p1 - p0);
    return true;
  } else {
    return false;
  }
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = gl_FragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  uv -= .5f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 ro = f32x3(0, 0, 3);
  f32x3 rd = normalize(f32x3(uv, 0.f) - f32x3(0, 0, 1));
  f32x3 nor, p0;
  float t0;
  bool hit = parametric_curve_newton_trace(__ctx, ro, rd, p0, nor, t0);
  if (hit) {
    f32x3 light = f32x3(0, 1, 4);
    float dif = clamp(dot(nor, normalize(light - p0)), 0.f, 1.f);
    dif *= 5.f / dot(light - p0, light - p0);
    fragColor = f32x4(f32x3(pow(dif, 0.4545f)), 1);
  } else {
    fragColor = f32x4(0, 0, 0, 1);
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
