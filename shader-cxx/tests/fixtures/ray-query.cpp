ray_query create_query() { return make_ray_query(); }
template <class Q> bool advance(Q &q) { return q.proceed(); }
struct P {
  __storage unsigned *data;
};
__compute
__workgroup_size(1, 1, 1)
void query(__binding(0, 0) acceleration_structure scenes[4],
           __push_constant const P *p) {
  ray_query q = create_query();
  ray_query *pointer = &q;
  pointer->initialize(scenes[0], 0, 255, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                      1.0f, 1000.0f);
  while (advance(q)) {
    p->data[17] = q.object_to_world(0)[1][2];
    if (q.intersection_type(0) == 0)
      q.confirm_intersection();
    else {
      p->data[14] = q.candidate_aabb_opaque();
      q.generate_intersection(1.0f);
    }
    ray_query alias = q;
    alias.terminate();
  }
  const ray_query &result = q;
  static_assert(sizeof(decltype(result.object_to_world(1))) == 12 * sizeof(float));
  static_assert(sizeof(decltype(result.world_to_object(1))) == 12 * sizeof(float));
  if (result.intersection_type(1) != 0) {
    p->data[0] = result.primitive_idx(1);
    p->data[1] = result.instance_custom_idx(1);
    p->data[2] = result.geometry_idx(1);
    p->data[4] = result.intersection_t(1);
    p->data[5] = result.instance_idx(1);
    p->data[6] = result.instance_sbt_offset(1);
    p->data[10] = result.object_ray_origin(1)[1];
    p->data[11] = result.object_ray_direction(1)[2];
    p->data[15] = result.object_to_world(1)[0][3];
    p->data[16] = result.world_to_object(1)[2][1];
    if (result.intersection_type(1) == 1) {
      p->data[3] = result.front_facing(1);
      p->data[9] = result.barycentrics(1)[0];
    }
  }
  p->data[7] = result.ray_t_min();
  p->data[8] = result.ray_flags();
  p->data[12] = result.world_ray_origin()[1];
  p->data[13] = result.world_ray_direction()[2];
}
