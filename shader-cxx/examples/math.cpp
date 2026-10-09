struct Params {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void math(__push_constant__ const Params *p) {
  p->data[0] = sqrtf(p->data[0]);
  p->data[1] = sinf(p->data[1]);
  p->data[2] = fmaf(p->data[2], 2.0f, 1.0f);
  p->data[3] = fminf(fabsf(p->data[3]), 2.0f);
}
