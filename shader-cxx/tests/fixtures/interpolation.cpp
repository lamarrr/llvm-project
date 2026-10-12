// Matching location-based varyings; all interfaces belong to entry parameters.
__vertex
void interpolation_vertex(__flat __output(0) unsigned *id,
                          __output(1) __noperspective __centroid float *linear,
                          __sample __output(2) float *per_sample,
                          __noperspective __output(3) float (*coords)[2],
                          __output(5) float *perspective) {
  *id = vertex_idx;
  *linear = static_cast<float>(vertex_idx);
  *per_sample = static_cast<float>(instance_idx);
  (*coords)[0] = static_cast<float>(vertex_idx);
  (*coords)[1] = static_cast<float>(instance_idx);
  *perspective = 1.0f;
}
__fragment
void interpolation_fragment(__input(0) __flat unsigned id,
                            __centroid __noperspective __input(1) float linear,
                            __input(2) __sample float per_sample,
                            __input(3) __noperspective const float (*coords)[2],
                            __input(5) float perspective,
                            __output(0) float *color) {
  *color = static_cast<float>(id) + linear + per_sample + (*coords)[0] +
           (*coords)[1] + perspective;
}
