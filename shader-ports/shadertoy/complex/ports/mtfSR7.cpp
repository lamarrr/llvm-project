// Port of https://www.shadertoy.com/view/mtfSR7; author: ___HackerMann___.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mtfSR7.json.
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

f32x3 sphere_centre = f32x3(0);
float sphere_radius = 0.4f;
f32x2 sphere_line_intersection(Context &__ctx, f32x3 centre, float radius,
                               f32x3 ray_origin, f32x3 ray_direction) {
  float dir_sqr = dot(ray_direction, ray_direction);
  float diff = dot(ray_direction, centre - ray_origin);
  float sum =
      dot(ray_origin, ray_origin) + dot(centre, centre) - radius * radius;
  float discr_sqrt = sqrt(diff * diff - dir_sqr * sum);
  return f32x2(diff + discr_sqrt, diff - discr_sqrt) / dir_sqr;
}
f32x3 get_sphere_normal(Context &__ctx, f32x3 centre, f32x3 position) {
  return normalize(position - centre);
}
f32x3 calculate_density(Context &__ctx, f32x3 centre, f32x3 position,
                        float radius, f32x3 density_falloff) {
  float initial_density = 1.0f;
  float dist = length(centre - position);
  return initial_density *
         (exp(-dist / density_falloff) - exp(-radius / density_falloff));
}
f32x3 calculate_volume(Context &__ctx, f32x3 ray_origin, f32x3 ray_direction,
                       float max_dist, f32x3 light_pos) {
  int MAX_STEPS = 5000 / 26;
  float STEP_SIZE = 0.006f;
  f32x3 EPS_GAS_VOLUME = f32x3(1) / 10000.0f;
  f32x3 PLANET_CENTRE = f32x3(0);
  float ATMOSPHERE_RADIUS = 0.65f;
  f32x3 DISTANCE_MULTIPLIER = f32x3(260, 220, 280) * 3000000.0f;
  f32x3 gas_volume = f32x3(0.0f);
  float dist = 0.0f;
  for (int i = 0; i < MAX_STEPS; ++i) {
    f32x3 position = ray_origin + dist * ray_direction;
    f32x3 to_light_dir = normalize(light_pos - position);
    to_light_dir.data[2] *= -1.0f;
    f32x2 t = sphere_line_intersection(__ctx, sphere_centre, sphere_radius,
                                       position, to_light_dir);
    f32x3 density =
        max(calculate_density(__ctx, PLANET_CENTRE, position, ATMOSPHERE_RADIUS,
                              f32x3(1, 1.07f, 1.13f) * 0.1f),
            f32x3(0));
    f32x3 volume = STEP_SIZE * EPS_GAS_VOLUME * density;
    if (dist > max_dist - STEP_SIZE)
      break;
    dist += STEP_SIZE;
    if (t.data[0] < 1000.0f && 0.0f < t.data[0])
      continue;
    float light_dist = length(light_pos - position);
    gas_volume += volume / exp(light_dist * 2.0f) * DISTANCE_MULTIPLIER;
  }
  return gas_volume;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float ambient_brightness = 0.2f;
  float specular_power = 20.0f;
  float specular_multiplier = 0.4f;
  float gas_color_density = 18000.0f;
  float diffusion_intensity = 40.0f;
  f32x2 uv = 2.0f * fragCoord / iResolution.swizzle<0, 1>() - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 ray_origin = f32x3(0, 0, -1);
  f32x3 ray_direction = normalize(f32x3(uv, 0) - ray_origin);
  f32x2 t = sphere_line_intersection(__ctx, sphere_centre, sphere_radius,
                                     ray_origin, ray_direction);
  f32x3 nearest_hit_pos = ray_origin + t.data[0] * ray_direction;
  f32x3 light_pos = 6.0f * f32x3(cos(2.0f * iTime), 1, sin(2.0f * iTime));
  f32x3 dir_to_light = normalize(light_pos - nearest_hit_pos);
  f32x3 gas_volume =
      calculate_volume(__ctx, ray_origin, ray_direction, t.data[0], light_pos);
  f32x3 normal = get_sphere_normal(__ctx, sphere_centre, nearest_hit_pos);
  float light_dist = length(nearest_hit_pos - light_pos);
  float brightness = diffusion_intensity *
                     max(ambient_brightness, dot(normal, dir_to_light)) /
                     (light_dist * light_dist);
  f32x3 reflected = reflect(-ray_direction, normal);
  float r_dot_l = max(0.0f, dot(dir_to_light, reflected));
  float specular_brightness =
      pow(r_dot_l, specular_power) * specular_multiplier;
  f32x3 color = normal * 0.5f + 0.5f;
  color *= brightness;
  color += specular_brightness;
  color = clamp(color, 0.0f, 1.0f);
  f32x3 gas_color = gas_volume * gas_color_density;
  fragColor = f32x4(gas_color + color, 1);
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
