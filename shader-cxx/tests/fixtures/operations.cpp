struct P {
  __storage__ unsigned *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void operations(__push_constant__ const P *p) {
  static __shared__ unsigned value;
  atomic_store(&value, 4u);
  workgroup_barrier();
  subgroup_barrier();
  device_memory_barrier();
  p->data[0] = atomic_load(&value);
  p->data[1] = atomic_add(&value, 2u);
  p->data[2] = atomic_sub(&value, 2u);
  p->data[3] = atomic_exchange(&value, 2u);
  p->data[4] = atomic_min(&value, 2u);
  p->data[5] = atomic_max(&value, 2u);
  p->data[6] = atomic_and(&value, 2u);
  p->data[7] = atomic_or(&value, 2u);
  p->data[8] = atomic_xor(&value, 2u);
  p->data[9] = atomic_compare_exchange(&value, 1u, 2u);
  p->data[32] = atomic_compare_exchange(&value, 0u, 7u);
  p->data[33] = atomic_compare_exchange(&value, 0u, 9u);
  p->data[34] = atomic_load(&value);
  p->data[10] = subgroup_all(true);
  p->data[11] = subgroup_any(true);
  p->data[12] = subgroup_all_equal(global_invocation_id.x);
  p->data[13] = subgroup_elect();
  p->data[14] = subgroup_broadcast(global_invocation_id.x, 0u);
  p->data[15] = subgroup_broadcast_first(global_invocation_id.x);
  p->data[16] = subgroup_shuffle(global_invocation_id.x, 0u);
  p->data[17] = subgroup_shuffle_xor(global_invocation_id.x, 0u);
  p->data[18] = subgroup_shuffle_up(global_invocation_id.x, 0u);
  p->data[19] = subgroup_shuffle_down(global_invocation_id.x, 0u);
  p->data[20] = subgroup_add(global_invocation_id.x);
  p->data[21] = subgroup_mul(global_invocation_id.x);
  p->data[22] = subgroup_min(global_invocation_id.x);
  p->data[23] = subgroup_max(global_invocation_id.x);
  p->data[24] = subgroup_and(global_invocation_id.x);
  p->data[25] = subgroup_or(global_invocation_id.x);
  p->data[26] = subgroup_xor(global_invocation_id.x);
  p->data[27] = subgroup_inclusive_add(global_invocation_id.x);
  p->data[28] = subgroup_exclusive_add(global_invocation_id.x);
  p->data[29] = subgroup_inclusive_mul(global_invocation_id.x);
  p->data[30] = subgroup_exclusive_mul(global_invocation_id.x);

  auto ballot = subgroup_ballot(true);
  p->data[31] = subgroup_ballot_bit_count(ballot);
}
