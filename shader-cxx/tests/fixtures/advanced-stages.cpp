struct Payload {
  unsigned value;
};
struct Position {
  float data[4];
};

__geometry __geometry_mode(
    geometry_input::triangles, geometry_output::triangle_strip,
    3) void geometry_stage(__position const Position (*input)[3],
                           __position Position *position) {
  for (unsigned vertex = 0; vertex != 3; ++vertex) {
    for (unsigned component = 0; component != 4; ++component)
      position->data[component] = (*input)[vertex].data[component];
    emit_vertex();
  }
  end_primitive();
}

__tessellation_control
__output_vertices(3) void tess_control(
    __input(0) const float (*input)[3][4],
    __output(0) float (*output)[3][4],
    __tess_level_outer __patch float (*outer)[4],
    __tess_level_inner __patch float (*inner)[2],
    __input(1) const float (*weights)[3], __output(1) float (*results)[3]) {
  for (unsigned component = 0; component != 4; ++component)
    (*output)[invocation_id][component] =
        (*input)[invocation_id % patch_vertices_in][component];
  if (invocation_id == 0) {
    for (unsigned index = 0; index != 4; ++index)
      (*outer)[index] = 1.0f;
    (*inner)[0] = 1.0f;
    (*inner)[1] = 1.0f;
  }
  (*results)[invocation_id] = (*weights)[invocation_id];
}

__tessellation_evaluation
__tessellation_mode(
    tessellation_domain::triangles, tessellation_spacing::equal,
    vertex_order::counter_clockwise) void tess_evaluation(__input(0)
                                                          const float (
                                                              *input)[3][4],
                                                          __position float (
                                                              *position)[4]) {
  for (unsigned component = 0; component != 4; ++component)
    (*position)[component] = (*input)[0][component] * tess_coord[0] +
                             (*input)[1][component] * tess_coord[1] +
                             (*input)[2][component] * tess_coord[2] +
                             tess_level_outer[0] + tess_level_inner[0];
}

__task
__workgroup_size(1, 1, 1)
void task_stage(__task_payload Payload *payload) {
  payload->value = local_invocation_idx + workgroup_id[0] + num_workgroups[0];
  emit_mesh_tasks(1u, 1u, 1u, payload);
}

__mesh
__workgroup_size(1, 1, 1)
__mesh_output(mesh_topology::points, 1, 1) void mesh_stage(
    __task_payload const Payload *payload,
    __position Position (*positions)[1],
    __primitive_point_indices __per_primitive unsigned (*indices)[1],
    __cull_primitive bool (*cull)[1]) {
  set_mesh_outputs(1u, 1u);
  (*positions)[0].data[0] = float(payload->value);
  (*positions)[0].data[1] = 0.0f;
  (*positions)[0].data[2] = 0.0f;
  (*positions)[0].data[3] = 1.0f;
  (*indices)[0] = 0u;
  (*cull)[0] = false;
}

struct Float3 {
  float x, y, z;
};

__geometry __geometry_mode(19, 27, 1) void geometry_stream_stage(
    __position float (*position)[4]) {
  (*position)[0] = 0.0f;
  (*position)[1] = 0.0f;
  (*position)[2] = 0.0f;
  (*position)[3] = 1.0f;
  emit_stream_vertex(0u);
  end_stream_primitive(0u);
}

__ray_generation
void ray_generation_stage(__binding(0, 0) acceleration_structure scene,
                          __ray_payload Payload *payload,
                          __callable_data Payload *data) {
  payload->value = launch_id[0];
  trace_ray(scene, 0u, 255u, 0u, 0u, 0u, Float3{0, 0, 0}, 0.0f, Float3{0, 0, 1},
            100.0f, payload);
  data->value = payload->value;
  execute_callable(0u, data);
}

__intersection
void intersection_stage(__hit_attribute float *attribute) {
  *attribute = ray_tmin;
  report_intersection(ray_tmin + 1.0f, 1u);
}

__any_hit
void any_hit_stage(__incoming_ray_payload Payload *payload,
                   __hit_attribute const float *attribute) {
  payload->value = unsigned(*attribute);
  if (hit_kind == 0u)
    ignore_intersection();
  terminate_ray();
}

__closest_hit
void closest_hit_stage(__incoming_ray_payload Payload *payload,
                       __hit_attribute const float *attribute,
                       __shader_record const Payload *record) {
  payload->value = unsigned(*attribute) + record->value + instance_custom_idx +
                   ray_geometry_idx + primitive_idx;
}

__miss
void miss_stage(__incoming_ray_payload Payload *payload) {
  payload->value = unsigned(world_ray_direction[0]);
}

__callable
void callable_stage(__incoming_callable_data Payload *data) {
  data->value += launch_id[0];
}
