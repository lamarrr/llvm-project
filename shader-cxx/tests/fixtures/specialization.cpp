struct P {
  __storage float *data;
};
__compute
__workgroup_size(1, 1, 1)
void specialization(__push_constant const P *p) {
  unsigned n = specialization_constant(0, 4u);
  float scale = specialization_constant(1, 2.0f);
  bool gate = specialization_constant(2, true);
  p->data[0] = gate ? n * scale : -1;
  p->data[1] = specialization_constant(0, 4u);
  p->data[2] = specialization_constant(3, -7);
  p->data[3] = specialization_constant(4, 9000000000ULL);
}
