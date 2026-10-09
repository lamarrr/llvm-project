struct P {
  __storage__ unsigned long long *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void control_extended(__push_constant__ const P *p) {
  memory_barrier(1, 0x948);
  control_barrier(2, 2, 0x108);
  p->data[0] = shader_clock(1);
  p->data[1] = shader_clock(3);
}
__fragment__
void demote_fragment(__output__(0) float *out) {
  if (helper_invocation)
    demote();
  *out = 1;
}
