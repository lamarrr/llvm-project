struct P {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void trig(__push_constant__ const P *p) {
  float x = p->data[0] * 0.25f;
  p->data[0] = sin(x);
  p->data[1] = cos(x);
  p->data[2] = tan(x);
  p->data[3] = asin(x);
  p->data[4] = acos(x);
  p->data[5] = atan(x);
  p->data[6] = atan2(x, -x);
  p->data[7] = sinh(x);
  p->data[8] = cosh(x);
  p->data[9] = tanh(x);
  p->data[10] = asinh(x);
  p->data[11] = acosh(x + 1.0f);
  p->data[12] = atanh(x);
  p->data[13] = radians(x);
  p->data[14] = degrees(x);
}
