__vertex
void vertex_scope(__output(0) float *value) {
  *value = static_cast<float>(vertex_idx + instance_idx + base_vertex +
                              base_instance + draw_idx + view_idx);
}
