__vertex__
void vertex_scope(__output__(0) float *value) {
  *value = static_cast<float>(vertex_idx + instance_idx + base_vertex +
                              base_instance + draw_idx + view_idx);
}
