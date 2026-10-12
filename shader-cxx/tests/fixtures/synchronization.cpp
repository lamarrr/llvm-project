struct P {
  __storage unsigned *data;
};
__compute
__workgroup_size(4, 1, 1)
void synchronization(__push_constant const P *p) {
  static __shared unsigned count;
  if (local_invocation_id.x == 0)
    atomic_store(&count, 0u);
  workgroup_barrier();
  atomic_add(&count, 1u);
  workgroup_barrier();
  if (local_invocation_id.x == 0)
    p->data[0] = atomic_load(&count);
}
