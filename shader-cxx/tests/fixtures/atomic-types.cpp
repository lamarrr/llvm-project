template <class T> void atomic_values(__storage__ T *p) {
  atomic_store(p, T(4));
  T total = atomic_load(p);
  total += atomic_add(p, T(2));
  total += atomic_sub(p, T(2));
  total += atomic_exchange(p, T(2));
  total += atomic_min(p, T(2));
  total += atomic_max(p, T(2));
  total += atomic_and(p, T(2));
  total += atomic_or(p, T(2));
  total += atomic_xor(p, T(2));
  total += atomic_compare_exchange(p, T(1), T(2));
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
