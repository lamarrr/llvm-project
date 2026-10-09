struct AtomicPolicyParams {
  __storage__ unsigned *value;
};

__compute__
__workgroup_size__(1, 1, 1)
void atomic_policy(__push_constant__ const AtomicPolicyParams *params) {
  static __shared__ unsigned scratch = 0;
  unsigned old =
      atomicLoad(params->value, memory_order::acquire, memory_scope::device);
  atomicStore(params->value, old, memory_order::release, memory_scope::device);
  old = atomicAdd(params->value, 1u, memory_order::acquire_release,
                  memory_scope::device);
  old = atomicCAS(params->value, old, old + 1u, memory_order::acquire_release,
                  memory_order::acquire, memory_scope::device);
  atomicAdd(&scratch, old, memory_order::relaxed, memory_scope::workgroup);
}
