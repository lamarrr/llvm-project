struct P {
  __storage__ unsigned *data;
};
__compute__
__workgroup_size__(4, 1, 1)
void synchronization(__push_constant__ const P *p) {
  static __shared__ unsigned count;
  if (local_invocation_id.x == 0)
    atomic_store(&count, 0u);
  workgroup_barrier();
  atomic_add(&count, 1u);
  workgroup_barrier();
  if (local_invocation_id.x == 0)
    p->data[0] = atomic_load(&count);
}
