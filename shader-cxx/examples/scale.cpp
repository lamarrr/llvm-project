// No includes: shader declarations and scope values are compiler-native.
struct Params {
  __storage float *data;
  unsigned count;
};

template <class T, unsigned N> const T (&borrow(const T (&value)[N]))[N] {
  return value;
}

__compute
__workgroup_size(64, 1, 1)
void scale(__push_constant const Params *params) {
  const unsigned (&id)[3] = borrow(global_invocation_id);
  if (id[0] < params->count)
    params->data[id[0]] *= 2.0f;
}
