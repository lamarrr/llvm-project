// Port of upstream/ray_shadow.frag; see upstream/LICENSE.
struct Vec3 {
  float x, y, z;
};
struct Vec4 {
  float x, y, z, w;
};
struct GlobalUniform {
  float model[4][4];
  float view_projection[4][4];
  float camera_position[4];
  float light_position[4];
};
static_assert(sizeof(GlobalUniform) == 160);

__fragment
void vulkan_ray_shadow(__input(0) Vec4 position, __input(1) Vec3 normal,
                       __input(2) Vec4 scene_position,
                       __binding(0, 0) acceleration_structure scene,
                       __binding(0, 1)
                       __uniform const GlobalUniform *uniforms,
                       __output(0) Vec4 *color) {
  ray_query query = make_ray_query();
  ray_query shadow = make_ray_query();
  auto u = abs(dot(normal, Vec3{0, 0, 1})) > 0.9f
               ? cross(normal, Vec3{1, 0, 0})
               : cross(normal, Vec3{0, 0, 1});
  auto v = cross(normal, u);
  float accumulated_ao = 0.0f;
  float accumulated_factor = 0.0f;
  for (unsigned j = 0; j < 3; ++j) {
    float phi = 0.5f * (-3.14159f + 6.28318f * float(j + 1u) / 5.0f);
    for (unsigned k = 0; k < 3; ++k) {
      float theta = 0.5f * (-3.14159f + 6.28318f * float(k + 1u) / 5.0f);
      float x = cos(phi) * sin(theta);
      float y = sin(phi) * sin(theta);
      float z = cos(theta);
      float direction_x = x * u.x + y * v.x + z * normal.x;
      float direction_y = x * u.y + y * v.y + z * normal.y;
      float direction_z = x * u.z + y * v.z + z * normal.z;
      query.initialize(scene, 4u, 255u, scene_position.x, scene_position.y,
                       scene_position.z, 0.01f, direction_x, direction_y,
                       direction_z, 2.0f);
      query.proceed();
      float distance =
          query.intersection_type(1u) == 0u ? 2.0f : query.intersection_t(1u);
      float factor = 0.2f + 0.8f * z * z;
      accumulated_factor += factor;
      accumulated_ao += min(distance, 2.0f) * factor;
    }
  }
  float ao = accumulated_ao / (2.0f * accumulated_factor);
  ao = clamp(ao * ao, 0.0f, 1.0f);

  float light_x = uniforms->light_position[0] - scene_position.x;
  float light_y = uniforms->light_position[1] - scene_position.y;
  float light_z = uniforms->light_position[2] - scene_position.z;
  shadow.initialize(scene, 4u, 255u, scene_position.x, scene_position.y,
                    scene_position.z, 0.01f, light_x, light_y, light_z, 1.0f);
  shadow.proceed();
  float lighting = shadow.intersection_type(1u) == 0u ? 1.0f : 0.2f;
  *color = Vec4{lighting * ao, lighting * ao, lighting * ao, 1.0f};
}
