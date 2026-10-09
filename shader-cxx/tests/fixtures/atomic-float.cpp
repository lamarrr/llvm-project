struct P {
  __storage__ float *data;
  __storage__ double *wide;
};
__compute__
__workgroup_size__(1, 1, 1)
void atomic_float(__push_constant__ const P *p) {
  p->data[1] = atomicAdd(p->data, 2.0f);
  p->data[2] = atomicSub(p->data, 1.0f);
  p->data[3] = atomicMin(p->data, -2.0f);
  p->data[4] = atomicMax(p->data, 4.0f);
  *p->wide = atomicAdd(p->wide, 2.0);
  *p->wide = atomicMin(p->wide, -1.0);
  *p->wide = atomicMax(p->wide, 3.0);
  static __shared__ float local;
  local = 1;
  p->data[5] = atomicAdd(&local, 2.0f);
  p->data[6] = atomicMin(&local, -1.0f);
  p->data[7] = atomicMax(&local, 3.0f);
}
