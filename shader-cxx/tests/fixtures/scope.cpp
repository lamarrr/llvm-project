struct Params {
  __storage__ unsigned *data;
};
template <class T, unsigned N> const T (&borrow(const T (&x)[N]))[N] {
  return x;
}
__compute__
__workgroup_size__(2, 3, 4) void scope(__push_constant__ const Params *p) {
  auto store = [p](unsigned i, unsigned x) { p->data[i] = x; };
  const unsigned (&global)[3] = borrow(global_invocation_id);
  const unsigned *local = local_invocation_id;
  store(0, global[0] + global_invocation_id.y + global_invocation_id.z);
  store(1, local[0] + local_invocation_id.y + local_invocation_id.z);
  store(2, workgroup_id.x + workgroup_id.y + workgroup_id.z);
  store(3, num_workgroups.x + num_workgroups.y + num_workgroups.z);
  store(4, workgroup_size.x + workgroup_size.y + workgroup_size.z);
  store(5, local_invocation_idx);
  store(6, subgroup_size);
  store(7, num_subgroups);
  store(8, subgroup_id);
  store(9, subgroup_invocation_id);
}
