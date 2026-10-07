__vertex__ void vertex(__output__(0) float *varying) {
  *varying = static_cast<float>(vertex_idx + instance_idx);
}
__fragment__ void fragment(__input__(0) float varying,
                           __output__(0) float *color) {
  *color = front_facing ? varying : 0.0f;
}
