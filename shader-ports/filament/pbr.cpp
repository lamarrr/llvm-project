// Selected standard and clear-coat PBR paths from upstream/surface_*.fs.
// See upstream/LICENSE and upstream/manifest.json. Filament's generated
// material interface is represented here by explicit Shader C++ parameters.
#include <spirv/types.hpp>
using namespace spirv;

struct Frame {
  f32x4 camera_position;
  f32x4 directional_direction;
  f32x4 directional_color_intensity;
  f32x4 point_position_falloff;
  f32x4 point_color_intensity;
  f32x4 ibl_parameters;
};
static_assert(sizeof(Frame) == 96);
static_assert(__builtin_offsetof(Frame, camera_position) == 0);
static_assert(__builtin_offsetof(Frame, point_position_falloff) == 48);
static_assert(__builtin_offsetof(Frame, ibl_parameters) == 80);

struct Material {
  f32x4 base_color_factor;
  // metallic, perceptual roughness, normal scale, ambient occlusion strength
  f32x4 factors;
  // clear coat, clear-coat perceptual roughness, unused, unused
  f32x4 clear_coat;
};
static_assert(sizeof(Material) == 48);
static_assert(__builtin_offsetof(Material, base_color_factor) == 0);
static_assert(__builtin_offsetof(Material, factors) == 16);
static_assert(__builtin_offsetof(Material, clear_coat) == 32);

struct Surface {
  f32x3 normal;
  f32x3 geometric_normal;
  f32x3 view;
  f32x3 diffuse_color;
  f32x3 f0;
  f32x3 energy_compensation;
  float perceptual_roughness;
  float roughness;
  float no_v;
  float ambient_occlusion;
  float clear_coat;
  float clear_coat_roughness;
};

Frame load_frame(__uniform const Frame *source) { return *source; }

Material load_material(__push_constant const Material *source) {
  return *source;
}

float fifth_power(float x) {
  float x2 = x * x;
  return x2 * x2 * x;
}

float distribution_ggx(float roughness, float no_h) {
  float a = no_h * roughness;
  float k = min(roughness / (1.0f - no_h * no_h + a * a), 453.5f);
  return k * k * 0.3183098861837907f;
}

float visibility_smith_ggx(float roughness, float no_v, float no_l) {
  float a2 = roughness * roughness;
  float lambda_v = no_l * sqrt((no_v - a2 * no_v) * no_v + a2);
  float lambda_l = no_v * sqrt((no_l - a2 * no_l) * no_l + a2);
  return 0.5f / max(lambda_v + lambda_l, 0.0000077f);
}

f32x3 fresnel_schlick(f32x3 f0, float f90, float vo_h) {
  return f0 + (f32x3(f90) - f0) * fifth_power(1.0f - vo_h);
}

float fresnel_schlick(float f0, float f90, float vo_h) {
  return f0 + (f90 - f0) * fifth_power(1.0f - vo_h);
}

float perceptual_roughness_to_lod(float value, float max_lod) {
  return max_lod * value * (2.0f - value);
}

template <bool ClearCoat>
f32x3 shade_light(Surface surface, f32x3 light_direction, f32x3 light_color,
                  float intensity, float attenuation) {
  f32x3 half_vector = normalize(surface.view + light_direction);
  float no_l = max(dot(surface.normal, light_direction), 0.0f);
  float no_h = max(dot(surface.normal, half_vector), 0.0f);
  float lo_h = max(dot(light_direction, half_vector), 0.0f);
  float d = distribution_ggx(surface.roughness, no_h);
  float visibility =
      visibility_smith_ggx(surface.roughness, surface.no_v, no_l);
  float f90 = clamp(dot(surface.f0, f32x3(16.5f)), 0.0f, 1.0f);
  f32x3 fresnel = fresnel_schlick(surface.f0, f90, lo_h);
  f32x3 specular = fresnel * (d * visibility) * surface.energy_compensation;
  f32x3 result = surface.diffuse_color * 0.3183098861837907f + specular;
  if constexpr (ClearCoat) {
    float coat_no_h = max(dot(surface.geometric_normal, half_vector), 0.0f);
    float coat_no_l = max(dot(surface.geometric_normal, light_direction), 0.0f);
    float coat_d = distribution_ggx(surface.clear_coat_roughness, coat_no_h);
    float coat_v = 0.25f / max(lo_h * lo_h, 0.0000039f);
    float coat_f = fresnel_schlick(0.04f, 1.0f, lo_h) * surface.clear_coat;
    result = result * ((1.0f - coat_f) * no_l) +
             f32x3(coat_d * coat_v * coat_f * coat_no_l);
    return result * light_color * (intensity * attenuation);
  }
  return result * light_color * (intensity * attenuation * no_l);
}

template <bool ClearCoat>
f32x4 evaluate_pbr(f32x2 uv, f32x3 interpolated_normal, f32x4 tangent,
                   f32x3 world_position, texture_2d<f32x4> base_map,
                   texture_2d<f32x4> orm_map, texture_2d<f32x4> normal_map,
                   texture_2d<f32x4> dfg_lut,
                   texture_cube<f32x4> specular_environment, sampler filtering,
                   Frame frame, Material material) {
  f32x4 base =
      base_map.sample(filtering, uv.x(), uv.y()) * material.base_color_factor;
  f32x4 orm = orm_map.sample(filtering, uv.x(), uv.y());
  f32x4 normal_sample = normal_map.sample(filtering, uv.x(), uv.y());
  float metallic = clamp(orm.z() * material.factors.x(), 0.0f, 1.0f);
  float perceptual_roughness =
      clamp(orm.y() * material.factors.y(), 0.045f, 1.0f);
  float roughness = perceptual_roughness * perceptual_roughness;
  float ambient_occlusion = 1.0f + material.factors.w() * (orm.x() - 1.0f);

  f32x3 geometric_normal = normalize(interpolated_normal);
  f32x3 tangent_direction = normalize(tangent.xyz());
  f32x3 bitangent =
      normalize(cross(geometric_normal, tangent_direction)) * tangent.w();
  f32x3 tangent_normal((normal_sample.x() * 2.0f - 1.0f) * material.factors.z(),
                       (normal_sample.y() * 2.0f - 1.0f) * material.factors.z(),
                       normal_sample.z() * 2.0f - 1.0f);
  f32x3 normal = normalize(tangent_direction * tangent_normal.x() +
                           bitangent * tangent_normal.y() +
                           geometric_normal * tangent_normal.z());
  f32x3 view = normalize(frame.camera_position.xyz() - world_position);
  float no_v = max(dot(normal, view), 0.0001f);
  f32x3 base_rgb = base.xyz();
  f32x3 diffuse_color = base_rgb * (1.0f - metallic);
  f32x3 f0 = base_rgb * metallic + f32x3(0.04f * (1.0f - metallic));

  f32x4 dfg = dfg_lut.sample_lod(filtering, no_v, perceptual_roughness, 0.0f);
  f32x3 energy_compensation =
      f32x3(1.0f) + f0 * (1.0f / max(dfg.y(), 0.0001f) - 1.0f);
  Surface surface{normal,
                  geometric_normal,
                  view,
                  diffuse_color,
                  f0,
                  energy_compensation,
                  perceptual_roughness,
                  roughness,
                  no_v,
                  ambient_occlusion,
                  clamp(material.clear_coat.x(), 0.0f, 1.0f),
                  max(material.clear_coat.y(), 0.045f) *
                      max(material.clear_coat.y(), 0.045f)};

  f32x3 directional_direction = normalize(frame.directional_direction.xyz());
  f32x3 color = shade_light<ClearCoat>(
      surface, directional_direction, frame.directional_color_intensity.xyz(),
      frame.directional_color_intensity.w(), 1.0f);

  f32x3 position_to_light = frame.point_position_falloff.xyz() - world_position;
  float distance_squared = dot(position_to_light, position_to_light);
  float falloff = distance_squared * frame.point_position_falloff.w();
  float smooth_factor = max(1.0f - falloff * falloff, 0.0f);
  float attenuation =
      smooth_factor * smooth_factor / max(distance_squared, 0.0001f);
  color += shade_light<ClearCoat>(surface, normalize(position_to_light),
                                  frame.point_color_intensity.xyz(),
                                  frame.point_color_intensity.w(), attenuation);

  f32x3 reflection = reflect(-view, normal);
  float lod = perceptual_roughness_to_lod(perceptual_roughness,
                                          frame.ibl_parameters.x());
  f32x3 specular_radiance = specular_environment
                                .sample_lod(filtering, reflection.x(),
                                            reflection.y(), reflection.z(), lod)
                                .xyz();
  f32x3 diffuse_irradiance =
      specular_environment
          .sample_lod(filtering, normal.x(), normal.y(), normal.z(),
                      frame.ibl_parameters.x())
          .xyz();
  f32x3 dfg_term = f32x3(dfg.x()) + f0 * (dfg.y() - dfg.x());
  f32x3 indirect_specular = dfg_term * specular_radiance * energy_compensation;
  f32x3 indirect_diffuse = diffuse_color * diffuse_irradiance *
                           (f32x3(1.0f) - dfg_term) * ambient_occlusion;
  if constexpr (ClearCoat) {
    float coat_no_v = max(dot(geometric_normal, view), 0.0001f);
    float coat_f = fresnel_schlick(0.04f, 1.0f, coat_no_v) * surface.clear_coat;
    f32x3 coat_reflection = reflect(-view, geometric_normal);
    float coat_lod = perceptual_roughness_to_lod(material.clear_coat.y(),
                                                 frame.ibl_parameters.x());
    f32x3 coat_radiance =
        specular_environment
            .sample_lod(filtering, coat_reflection.x(), coat_reflection.y(),
                        coat_reflection.z(), coat_lod)
            .xyz();
    indirect_diffuse *= 1.0f - coat_f;
    indirect_specular =
        indirect_specular * (1.0f - coat_f) + coat_radiance * coat_f;
  }
  color += (indirect_diffuse + indirect_specular) * frame.ibl_parameters.y();
  return f32x4(color, base.w());
}

#define PBR_ARGUMENTS                                                          \
  __input(0) f32x2 uv, __input(1) f32x3 normal, __input(2) f32x4 tangent,      \
      __input(3) f32x3 world_position,                                         \
      __binding(0, 0) texture_2d<f32x4> base_map,                              \
      __binding(0, 1) texture_2d<f32x4> orm_map,                               \
      __binding(0, 2) texture_2d<f32x4> normal_map,                            \
      __binding(0, 3) texture_2d<f32x4> dfg_lut,                               \
      __binding(0, 4) texture_cube<f32x4> specular_environment,                \
      __binding(0, 5) sampler filtering,                                       \
      __binding(1, 0) __uniform const Frame *frame,                            \
      __push_constant const Material *material, __output(0) f32x4 *output

__fragment
void filament_pbr_standard(PBR_ARGUMENTS) {
  *output = evaluate_pbr<false>(uv, normal, tangent, world_position, base_map,
                                orm_map, normal_map, dfg_lut,
                                specular_environment, filtering,
                                load_frame(frame), load_material(material));
}

__fragment
void filament_pbr_clearcoat(PBR_ARGUMENTS) {
  *output =
      evaluate_pbr<true>(uv, normal, tangent, world_position, base_map, orm_map,
                         normal_map, dfg_lut, specular_environment, filtering,
                         load_frame(frame), load_material(material));
}

#undef PBR_ARGUMENTS
