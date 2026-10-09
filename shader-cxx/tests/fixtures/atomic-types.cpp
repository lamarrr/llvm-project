template <class T> void atomic_values(__storage__ T *p) {
  atomicStore(p, T(4));
  T total = atomicLoad(p);
  total += atomicAdd(p, T(2));
  total += atomicSub(p, T(2));
  total += atomicExch(p, T(2));
  total += atomicMin(p, T(2));
  total += atomicMax(p, T(2));
  total += atomicAnd(p, T(2));
  total += atomicOr(p, T(2));
  total += atomicXor(p, T(2));
  total += atomicCAS(p, T(1), T(2));
  p[1] = total;
}
struct P {
  __storage__ int *signed32;
  __storage__ unsigned *unsigned32;
  __storage__ long long *signed64;
  __storage__ unsigned long long *unsigned64;
};
__compute__
__workgroup_size__(1, 1, 1)
void atomic_types(__push_constant__ const P *p) {
  atomic_values(p->signed32);
  atomic_values(p->unsigned32);
  atomic_values(p->signed64);
  atomic_values(p->unsigned64);
}
