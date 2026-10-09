struct Params {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void shared(__push_constant__ const Params *p) {
  static __shared__ float scratch[4];
  scratch[0] = *p->data;
  *p->data = scratch[0] + 1.0f;
}
