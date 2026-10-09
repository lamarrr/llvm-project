struct P {
  __storage__ float *data;
};
__compute__
__workgroup_size__(4, 1, 1)
void subgroup_extended(__push_constant__ const P *p) {
  unsigned bits[4] = {15, 0, 0, 0};
  float lane = (float)global_invocation_id.x + 1;
  float v[2] = {lane, lane * 2};
  auto a = subgroup_quad_broadcast(v, 0);
  auto b = subgroup_quad_swap_horizontal(v);
  auto c = subgroup_quad_swap_vertical(v);
  auto d = subgroup_quad_swap_diagonal(v);
  auto sum = subgroup_add(v);
  auto scan = subgroup_inclusive_add(v);
  auto broadcast = subgroup_broadcast_first(v);
  auto shuffle = subgroup_shuffle(v, 0);
  bool eq = subgroup_all_equal(v);
  if (global_invocation_id.x == 0) {
    p->data[0] = subgroup_inverse_ballot(bits);
    p->data[1] = subgroup_ballot_bit_extract(bits, 2);
    p->data[2] = subgroup_ballot_find_lsb(bits);
    p->data[3] = subgroup_ballot_find_msb(bits);
    p->data[4] = subgroup_ballot_inclusive_bit_count(bits);
    p->data[5] = subgroup_ballot_exclusive_bit_count(bits);
    p->data[6] = a[1] + b[1] + c[1] + d[1];
    p->data[7] = sum[1] + scan[1] + broadcast[1] + shuffle[1] + eq;
  }
  unsigned n = global_invocation_id.x + 1;
  bool predicate = (n & 1u) != 0;
  unsigned extra =
      subgroup_inclusive_min(n) + subgroup_exclusive_min(n) +
      subgroup_inclusive_max(n) + subgroup_exclusive_max(n) +
      subgroup_inclusive_and(n) + subgroup_exclusive_and(n) +
      subgroup_inclusive_or(n) + subgroup_exclusive_or(n) +
      subgroup_inclusive_xor(n) + subgroup_exclusive_xor(n) +
      subgroup_clustered_add(n, 2u) + subgroup_clustered_mul(n, 2u) +
      subgroup_clustered_min(n, 2u) + subgroup_clustered_max(n, 2u) +
      subgroup_clustered_and(n, 2u) + subgroup_clustered_or(n, 2u) +
      subgroup_clustered_xor(n, 2u);
  unsigned logical = subgroup_logical_and(predicate) +
                     subgroup_logical_or(predicate) +
                     subgroup_logical_xor(predicate) +
                     subgroup_inclusive_logical_and(predicate) +
                     subgroup_exclusive_logical_and(predicate) +
                     subgroup_inclusive_logical_or(predicate) +
                     subgroup_exclusive_logical_or(predicate) +
                     subgroup_inclusive_logical_xor(predicate) +
                     subgroup_exclusive_logical_xor(predicate) +
                     subgroup_clustered_logical_and(predicate, 2u) +
                     subgroup_clustered_logical_or(predicate, 2u) +
                     subgroup_clustered_logical_xor(predicate, 2u);
  if (global_invocation_id.x == 0)
    p->data[8] = float(extra + logical);
}
