// Port of the first-level quad reduction in upstream/ffx_spd.h.
// See upstream/license.md and the license at the top of upstream/ffx_spd.h.

__compute
__workgroup_size(64, 1, 1)
void fidelityfx_spd_quad(
    __binding(0, 0) texture_2d<float[4]> source,
    __binding(0, 1) storage_texture_2d<float[4], rgba32f> destination) {
  unsigned local = local_invocation_idx;
  unsigned quad = local / 4;
  int x = int(workgroup_id.x * 8 + (quad % 4) * 2 + (local & 1));
  int y = int(workgroup_id.y * 8 + (quad / 4) * 2 + ((local >> 1) & 1));
  auto value = source.load(x, y, 0u);
  auto horizontal = subgroup_quad_swap_horizontal(value);
  auto vertical = subgroup_quad_swap_vertical(value);
  auto diagonal = subgroup_quad_swap_diagonal(value);
  if ((local & 3u) == 0u) {
    int output_x = int(workgroup_id.x * 4 + quad % 4);
    int output_y = int(workgroup_id.y * 4 + quad / 4);
    destination.store(
        output_x, output_y,
        (value[0] + horizontal[0] + vertical[0] + diagonal[0]) * 0.25f,
        (value[1] + horizontal[1] + vertical[1] + diagonal[1]) * 0.25f,
        (value[2] + horizontal[2] + vertical[2] + diagonal[2]) * 0.25f,
        (value[3] + horizontal[3] + vertical[3] + diagonal[3]) * 0.25f);
  }
}
