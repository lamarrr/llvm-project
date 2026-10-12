struct FloatAtomicPolicyParams {
  __storage__ float *value;
  __storage__ double *wide;
  __storage__ _Float16 *narrow;
};

__compute__
__workgroup_size__(1, 1, 1)
void atomic_float_policy(
    __push_constant__ const FloatAtomicPolicyParams *params) {
  atomic_store(params->value, 1.0f, memory_order::release,
               memory_scope::device);
  float value =
      atomic_load(params->value, memory_order::acquire, memory_scope::device);
  params->value[1] = atomic_exchange(params->value, value + 1.0f);
  atomic_store(params->wide, 1.0);
  params->wide[1] =
      atomic_exchange(params->wide, atomic_load(params->wide) + 1.0);
  static __shared__ float shared;
  atomic_store(&shared, 1.0f, memory_order::relaxed, memory_scope::workgroup);
  params->value[2] = atomic_exchange(&shared, atomic_load(&shared));
  atomic_store(params->narrow, static_cast<_Float16>(1.0f));
  params->narrow[1] =
      atomic_exchange(params->narrow, atomic_load(params->narrow));
  params->narrow[2] = atomic_add(params->narrow, static_cast<_Float16>(1.0f));
  params->narrow[3] = atomic_min(params->narrow, static_cast<_Float16>(0.0f));
}
