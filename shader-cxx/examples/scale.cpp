// No includes: shader declarations and scope values are compiler-native.
struct Params {
  __storage__ __align__(16) float *data;
  unsigned count;
};

template <class T, unsigned N> const T (&borrow(const T (&value)[N]))[N] {
  return value;
}

__compute__
__workgroup_size__(64, 1, 1)
void scale(__push_constant__ const Params *params) {
  const unsigned (&id)[3] = borrow(global_invocation_id);
  if (id[0] < params->count)
    params->data[id[0]] *= 2.0f;
}
