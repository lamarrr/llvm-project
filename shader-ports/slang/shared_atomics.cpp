// Port of upstream/atomics-groupshared.slang; see upstream/LICENSE.
struct Params {
  __storage unsigned *output;
};
static_assert(sizeof(Params) == 8);

__compute
__workgroup_size(4, 1, 1)
void slang_shared_atomics(__push_constant const Params *params) {
  static __shared unsigned shared[4];
  unsigned val = local_invocation_idx;
  shared[val] = 0;
  workgroup_barrier();
  unsigned original_sum = 0;
  original_sum += atomic_add(&shared[val], val);
  original_sum += atomic_add(&shared[val ^ 1u], val * 16u);
  original_sum += atomic_add(&shared[val ^ 2u], val * 256u);
  workgroup_barrier();
  params->output[global_invocation_id.x] = shared[val] ^ original_sum;
}
