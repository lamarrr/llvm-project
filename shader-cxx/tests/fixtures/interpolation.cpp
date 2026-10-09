// Matching location-based varyings; all interfaces belong to entry parameters.
__vertex__
void interpolation_vertex(__flat__ __output__(0) unsigned *id,
                          __output__(1) __noperspective__
                          __centroid__ float *linear,
                          __sample__ __output__(2) float *per_sample,
                          __noperspective__ __output__(3) float (*coords)[2],
                          __output__(5) float *perspective) {
  *id = vertex_idx;
  *linear = static_cast<float>(vertex_idx);
  *per_sample = static_cast<float>(instance_idx);
  (*coords)[0] = static_cast<float>(vertex_idx);
  (*coords)[1] = static_cast<float>(instance_idx);
  *perspective = 1.0f;
}
__fragment__
void interpolation_fragment(__input__(0) __flat__ unsigned id,
                            __centroid__ __noperspective__
                            __input__(1) float linear,
                            __input__(2) __sample__ float per_sample,
                            __input__(3)
                            __noperspective__ const float (*coords)[2],
                            __input__(5) float perspective,
                            __output__(0) float *color) {
  *color = static_cast<float>(id) + linear + per_sample + (*coords)[0] +
           (*coords)[1] + perspective;
}
