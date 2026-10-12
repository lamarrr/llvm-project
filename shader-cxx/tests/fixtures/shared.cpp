struct Params {
  __storage float *data;
};
__compute
__workgroup_size(1, 1, 1)
void shared(__push_constant const Params *p) {
  static __shared float scratch[4];
  scratch[0] = *p->data;
  *p->data = scratch[0] + 1.0f;
}
