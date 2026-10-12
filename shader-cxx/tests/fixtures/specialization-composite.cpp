struct Output {
  __storage float *data;
};

__compute
__workgroup_size(1, 1, 1)
void specialization_composite(__push_constant const Output *output) {
  auto pair = specialization_composite2(specialization_constant(0, 1.0f),
                                        specialization_constant(1, 2.0f));
  auto triple =
      specialization_composite3(specialization_constant(2, 3.0f), 4.0f, 5.0f);
  auto quad = specialization_composite4(specialization_constant(3, 6.0f), 7.0f,
                                        8.0f, 9.0f);
  output->data[0] = pair[0] + pair[1] + triple[0] + triple[1] + triple[2] +
                    quad[0] + quad[1] + quad[2] + quad[3];
}
