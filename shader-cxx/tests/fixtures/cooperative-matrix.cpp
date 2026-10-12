struct Output {
  __storage float *data;
};

__compute
__workgroup_size(32, 1, 1)
void cooperative_matrix(__push_constant const Output *output) {
  static __shared float matrix_a[256];
  static __shared float matrix_b[256];
  static __shared float matrix_c[256];
  workgroup_barrier();
  cooperative_matrix_multiply(&matrix_a[0], &matrix_b[0], &matrix_c[0], 16, 16,
                              16);
  workgroup_barrier();
  output->data[0] = matrix_c[0];
}
