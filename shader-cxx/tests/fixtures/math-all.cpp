struct Params {
  __storage float *data;
};
__compute
__workgroup_size(1, 1, 1)
void math_all(__push_constant const Params *p) {
  float x = p->data[0];
  float y = p->data[1];
  p->data[0] = sqrtf(x);
  p->data[1] = sinf(x);
  p->data[2] = cosf(x);
  p->data[3] = expf(x);
  p->data[4] = exp2f(x);
  p->data[5] = logf(x);
  p->data[6] = log2f(x);
  p->data[7] = fabsf(x);
  p->data[8] = floorf(x);
  p->data[9] = ceilf(x);
  p->data[10] = truncf(x);
  p->data[11] = fminf(x, y);
  p->data[12] = fmaxf(x, y);
  p->data[13] = fmaf(x, y, 1.0f);
}
