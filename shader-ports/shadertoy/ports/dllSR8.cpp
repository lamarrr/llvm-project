// Port of https://www.shadertoy.com/view/dllSR8; author: ___HackerMann___.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dllSR8.json.
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
struct Context {
  TextureCube channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
float soft_min(Context &__ctx, float lhs, float rhs, float radius) {
  float h = clamp(0.5f + 0.5f * (lhs - rhs) / radius, 0.0f, 1.0f);
  return mix(lhs, rhs, h) - radius * h * (1.0f - h);
}
float sphere_sdf(Context &__ctx, f32x3 position, f32x3 centre, float radius) {
  return length(position - centre) - radius;
}
float torus_sdf(Context &__ctx, f32x3 position, float R, float r) {
  f32x2 q = f32x2(length(position.swizzle<0, 2>()) - R, position.data[1]);
  return length(q) - r;
}
float scene_sdf(Context &__ctx, f32x3 position) {
  float sphere_h =
      (pow(-1.5f * sin(0.23123f * iTime + 0.123f), 4.0f) - 1.8f) * 0.2f;
  float sphere = sphere_sdf(__ctx, position, f32x3(0, sphere_h, 0), 0.25f);
  float torus = torus_sdf(__ctx, position, 0.5f, 0.13f);
  return soft_min(__ctx, sphere, torus, 0.5f);
}
f32x3 calculate_normal(Context &__ctx, f32x3 pos) {
  f32x2 eps = f32x2(0.0001f, 0);
  return normalize(f32x3(scene_sdf(__ctx, pos + eps.swizzle<0, 1, 1>()) -
                             scene_sdf(__ctx, pos - eps.swizzle<0, 1, 1>()),
                         scene_sdf(__ctx, pos + eps.swizzle<1, 0, 1>()) -
                             scene_sdf(__ctx, pos - eps.swizzle<1, 0, 1>()),
                         scene_sdf(__ctx, pos + eps.swizzle<1, 1, 0>()) -
                             scene_sdf(__ctx, pos - eps.swizzle<1, 1, 0>())));
}
float march_shadow(Context &__ctx, f32x3 ray_origin, f32x3 to_light_direction,
                   float min_t, float max_t, float k) {
  float result = 1.0f;
  for (float t = min_t; t <= max_t;) {
    float h = scene_sdf(__ctx, ray_origin + t * to_light_direction);
    if (h < 0.0001f)
      return 0.0f;
    result = min(result, k * h / t);
    t += h;
  }
  return result;
}
float ray_march(Context &__ctx, f32x3 ray_origin, f32x3 ray_direction,
                int &steps) {
  float dist = 0.0f;
  steps = 0;
  for (; steps < 1000; ++steps) {
    float depth = scene_sdf(__ctx, ray_origin + dist * ray_direction);
    if (depth <= 0.0001f)
      return depth + dist;
    dist += depth;
    if (depth > 1000.0f)
      return 1000.0f;
  }
  return 1000.0f;
}
void mainImage(Context &__ctx, f32x4 &frag_color, f32x2 frag_coord) {
  f32x3 ambient_color = f32x3(0.6f);
  f32x2 uv = 2.0f * frag_coord / iResolution.swizzle<0, 1>() - f32x2(1);
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 ray_origin = f32x3(0, 2, -4);
  f32x3 ray_direction = normalize(f32x3(uv, 0) - ray_origin);
  f32x3 texture_sample =
      texture(__ctx.channel0, ray_direction).swizzle<0, 1, 2>();
  int steps;
  float dist = ray_march(__ctx, ray_origin, ray_direction, steps);
  if (60.0f < dist) {
    frag_color = f32x4(texture_sample, 1);
    return;
  }
  f32x3 hit_pos = ray_origin + dist * ray_direction;
  f32x3 normal = calculate_normal(__ctx, hit_pos);
  f32x3 light_position = 10.0f * f32x3(sin(1.5f * iTime), 1, cos(1.5f * iTime));
  f32x3 to_light_direction = normalize(light_position - hit_pos);
  float brightness =
      max(dot(normal, to_light_direction), ambient_color.data[0]);
  float to_light_dist = length(light_position - hit_pos);
  brightness /= 0.006f * to_light_dist * to_light_dist;
  f32x3 reflected = reflect(ray_direction, normal);
  float specular_power = 200.0f;
  float specular_brightness =
      pow(max(0.0f, dot(reflected, to_light_direction)), specular_power);
  float fresnel_power = 7.0f;
  float fresnel_factor = pow(1.0f - dot(normal, -ray_direction), fresnel_power);
  f32x3 albedo = f32x3(hit_pos * 0.5f + f32x3(0.5f));
  f32x3 color = brightness * albedo;
  float shadow = march_shadow(__ctx, hit_pos + 0.0001f * normal * 4.0f,
                              to_light_direction, 0.0f, 10000.0f, 2.2f);
  shadow = exp(shadow) / exp(1.0f);
  shadow = (shadow + ambient_color.data[0] + 0.8f) /
           (1.0f + ambient_color.data[0] + 0.8f);
  color = shadow * albedo;
  color += fresnel_factor * 0.35f;
  color += specular_brightness;
  frag_color = f32x4(color, 1);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_cube<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
