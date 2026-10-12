struct AtomicPolicyParams {
  __storage unsigned *value;
};

__compute
__workgroup_size(1, 1, 1)
void atomic_policy(__push_constant const AtomicPolicyParams *params) {
  static __shared unsigned scratch = 0;
  unsigned old =
      atomic_load(params->value, memory_order::acquire, memory_scope::device);
  atomic_store(params->value, old, memory_order::release, memory_scope::device);
  old = atomic_add(params->value, 1u, memory_order::acquire_release,
                   memory_scope::device);
  old = atomic_compare_exchange(params->value, old, old + 1u,
                                memory_order::acquire_release,
                                memory_order::acquire, memory_scope::device);
  atomic_add(&scratch, old, memory_order::relaxed, memory_scope::workgroup);
}
