// Port of upstream/separate_image_sampler.vert; see upstream/LICENSE.
struct Vec2 {
  float x, y;
};
struct Vec3 {
  float x, y, z;
};
struct Camera {
  // C row-major matrices. The original GLSL UBO expects column-major values.
  float projection[4][4];
  float model[4][4];
  float view_position[4];
};
static_assert(sizeof(Camera) == 144);

__vertex__
void vulkan_separate_sampler_vertex(__input__(0) Vec3 in_position,
                                    __input__(1) Vec2 in_uv,
                                    __input__(2) Vec3 in_normal,
                                    __binding__(0, 0)
                                    __uniform__ const Camera *camera,
                                    __output__(0) Vec2 *out_uv,
                                    __position__ float (*position)[4]) {
  *out_uv = in_uv;
  float vertex[4] = {in_position.x, in_position.y, in_position.z, 1.0f};
  auto model_position = mul(camera->model, vertex);
  auto clip_position = mul(camera->projection, model_position);
  (*position)[0] = clip_position[0];
  (*position)[1] = clip_position[1];
  (*position)[2] = clip_position[2];
  (*position)[3] = clip_position[3];
}
