__fragment
void fragment_scope(__output(0) float *color) {
  *color = frag_coord.x + frag_coord.y + frag_coord.z + frag_coord.w +
           point_coord.x + point_coord.y + sample_position.x +
           sample_position.y + static_cast<float>(sample_id + primitive_idx) +
           (front_facing ? 1.0f : 0.0f) + (helper_invocation ? 1.0f : 0.0f);
}
