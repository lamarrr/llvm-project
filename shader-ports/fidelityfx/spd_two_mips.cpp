// Port of SpdDownsampleMips_0_1_Intrinsics in upstream/ffx_spd.h.
// See upstream/license.md and the license at the top of upstream/ffx_spd.h.

__compute__
__workgroup_size__(256, 1, 1)
void fidelityfx_spd_two_mips(
    __binding__(0, 0) texture_2d<float[4]> source,
    __binding__(0, 1) storage_texture_2d<float[4], rgba32f> mip0,
    __binding__(0, 2) storage_texture_2d<float[4], rgba32f> mip1) {
  unsigned lane = local_invocation_idx & 3u;
  unsigned quad = local_invocation_idx >> 2;
  int x = int((quad & 7u) * 2u + (lane & 1u));
  int y = int((quad >> 3) * 2u + (lane >> 1));

// Each lane generates four pixels in the 32x32 first mip. Four neighboring
// lanes then reduce each quadrant to one pixel in the 16x16 second mip.
#define SPD_QUADRANT(QX, QY)                                                   \
  {                                                                            \
    int sx = int(workgroup_id.x * 64u) + 2 * x + 32 * (QX);                    \
    int sy = int(workgroup_id.y * 64u) + 2 * y + 32 * (QY);                    \
    auto a = source.load(sx, sy, 0u);                                          \
    auto b = source.load(sx + 1, sy, 0u);                                      \
    auto c = source.load(sx, sy + 1, 0u);                                      \
    auto d = source.load(sx + 1, sy + 1, 0u);                                  \
    float v[4] = {(a[0] + b[0] + c[0] + d[0]) * 0.25f,                         \
                  (a[1] + b[1] + c[1] + d[1]) * 0.25f,                         \
                  (a[2] + b[2] + c[2] + d[2]) * 0.25f,                         \
                  (a[3] + b[3] + c[3] + d[3]) * 0.25f};                        \
    int px = int(workgroup_id.x * 32u) + x + 16 * (QX);                        \
    int py = int(workgroup_id.y * 32u) + y + 16 * (QY);                        \
    mip0.store(px, py, v[0], v[1], v[2], v[3]);                                \
    auto h = subgroup_quad_swap_horizontal(v);                                 \
    auto t = subgroup_quad_swap_vertical(v);                                   \
    auto d2 = subgroup_quad_swap_diagonal(v);                                  \
    if (lane == 0u) {                                                          \
      int mx = int(workgroup_id.x * 16u) + x / 2 + 8 * (QX);                   \
      int my = int(workgroup_id.y * 16u) + y / 2 + 8 * (QY);                   \
      mip1.store(mx, my, (v[0] + h[0] + t[0] + d2[0]) * 0.25f,                 \
                 (v[1] + h[1] + t[1] + d2[1]) * 0.25f,                         \
                 (v[2] + h[2] + t[2] + d2[2]) * 0.25f,                         \
                 (v[3] + h[3] + t[3] + d2[3]) * 0.25f);                        \
    }                                                                          \
  }
  SPD_QUADRANT(0, 0)
  SPD_QUADRANT(1, 0)
  SPD_QUADRANT(0, 1)
  SPD_QUADRANT(1, 1)
#undef SPD_QUADRANT
}
