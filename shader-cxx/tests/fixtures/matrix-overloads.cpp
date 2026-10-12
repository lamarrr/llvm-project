struct P {
  __storage float *data;
};
__compute
__workgroup_size(1, 1, 1)
void matrix_overloads(__push_constant const P *p) {
  float x = p->data[0];
  float a[2][3] = {{x, 2, 3}, {4, 5, 6}};
  float v[3] = {1, 2, 3};
  float w[2] = {2, 3};
  auto av = mul(a, v);
  auto wa = mul(w, a);
  auto scaled = mul(a, x * 2);
  auto scaled2 = mul(x * 3, a);
  auto outer = outer_product(w, v);
  p->data[0] = av[0];
  p->data[1] = av[1];
  p->data[2] = wa[0];
  p->data[3] = wa[1];
  p->data[4] = wa[2];
  p->data[5] = scaled[1][2];
  p->data[6] = scaled2[1][2];
  p->data[7] = outer[0][2];
  p->data[8] = outer[1][2];
}
