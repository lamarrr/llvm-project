struct P {
  __storage__ float *data;
  __storage__ double *wide;
};
__compute__
__workgroup_size__(1, 1, 1)
void atomic_float(__push_constant__ const P *p) {
  p->data[1] = atomic_add(p->data, 2.0f);
  p->data[2] = atomic_sub(p->data, 1.0f);
  p->data[3] = atomic_min(p->data, -2.0f);
  p->data[4] = atomic_max(p->data, 4.0f);
  *p->wide = atomic_add(p->wide, 2.0);
  *p->wide = atomic_min(p->wide, -1.0);
  *p->wide = atomic_max(p->wide, 3.0);
  static __shared__ float local;
  local = 1;
  p->data[5] = atomic_add(&local, 2.0f);
  p->data[6] = atomic_min(&local, -1.0f);
  p->data[7] = atomic_max(&local, 3.0f);
}
