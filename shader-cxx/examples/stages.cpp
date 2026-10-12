__vertex
void vertex(__output(0) float *varying) {
  *varying = static_cast<float>(vertex_idx + instance_idx);
}
__fragment
void fragment(__input(0) float varying, __output(0) float *color) {
  *color = front_facing ? varying : 0.0f;
}
