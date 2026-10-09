struct P {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void decomposition(__push_constant__ const P *p) {
  float x = p->data[0] * 6.5f;
  float v[2] = {x, -x};
  int exponent[2] = {2, 3};
  auto a = modf(x);
  auto b = frexp(x);
  auto c = modf(v);
  auto d = frexp(v);
  auto e = ldexp(v, exponent);
  p->data[0] = a.fraction;
  p->data[1] = a.whole;
  p->data[2] = b.significand;
  p->data[3] = b.exponent;
  p->data[4] = c.fraction[1];
  p->data[5] = c.whole[1];
  p->data[6] = d.significand[1];
  p->data[7] = d.exponent[1];
  p->data[8] = ldexp(x, 2);
  p->data[9] = e[1];
}
