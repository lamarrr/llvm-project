struct P {
  __storage__ float *data;
};
__compute__
__workgroup_size__(1, 1, 1)
void specialization_types(__push_constant__ const P *p) {
  using UInt = unsigned;
  p->data[0] =
      specialization_constant(0, UInt(4)) + specialization_constant(0, 4u);
  p->data[1] = specialization_constant(1, static_cast<signed char>(-1));
  p->data[2] = specialization_constant(2, static_cast<unsigned char>(255));
  p->data[3] = specialization_constant(3, static_cast<short>(-1234));
  p->data[4] = specialization_constant(4, static_cast<unsigned short>(60000));
  p->data[5] = specialization_constant(5, -9000000000LL);
  p->data[6] = specialization_constant(6, 0xffffffffffffffffULL);
  p->data[7] = specialization_constant(7, 1.25);
  p->data[8] = specialization_constant(8, static_cast<_Float16>(1.5));
}
