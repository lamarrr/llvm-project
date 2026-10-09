struct P {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void bit_packing(__push_constant__ const P *p) {
  unsigned x = (unsigned)p->data[0] * 5;
  unsigned v[2] = {x, x << 1};
  int s[2] = {-8, 8};
  p->data[0] = bit_count(x);
  p->data[1] = bit_reverse(x) == 0xa0000000u;
  p->data[2] = bit_extract(x, 1, 2);
  p->data[3] = bit_insert(x, 3u, 4, 2);
  p->data[4] = find_lsb(x);
  p->data[5] = find_msb(x);
  auto c = bit_count(v);
  auto r = bit_reverse(v);
  auto e = bit_extract(s, 1, 3);
  auto i = bit_insert(v, v, 4, 2);
  auto l = find_lsb(v);
  auto m = find_msb(s);
  p->data[6] = c[1];
  p->data[7] = r[0] == 0xa0000000u;
  p->data[8] = e[0];
  p->data[9] = i[1];
  p->data[10] = l[1];
  p->data[11] = m[0];
  float f4[4] = {-1, 0, 1, 0};
  float u4[4] = {0, 1, 0, 1};
  float f2[2] = {-1, 1};
  float u2[2] = {0, 1};
  float h2[2] = {1, -2};
  unsigned d2[2] = {0, 0x3ff00000u};
  auto a = unpack_snorm4x8(pack_snorm4x8(f4));
  auto b = unpack_unorm4x8(pack_unorm4x8(u4));
  auto d = unpack_snorm2x16(pack_snorm2x16(f2));
  auto g = unpack_unorm2x16(pack_unorm2x16(u2));
  auto h = unpack_half2x16(pack_half2x16(h2));
  auto q = unpack_double2x32(pack_double2x32(d2));
  p->data[12] = a[0];
  p->data[13] = a[2];
  p->data[14] = b[0];
  p->data[15] = b[1];
  p->data[16] = d[0];
  p->data[17] = d[1];
  p->data[18] = g[0];
  p->data[19] = g[1];
  p->data[20] = h[0];
  p->data[21] = h[1];
  p->data[22] = q[0] == 0;
  p->data[23] = q[1] == 0x3ff00000u;
}
