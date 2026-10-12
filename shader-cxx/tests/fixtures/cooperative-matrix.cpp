struct Output {
  __storage__ float *data;
};

__compute__
__workgroup_size__(32, 1, 1)
void cooperative_matrix(__push_constant__ const Output *output) {
  static __shared__ float matrix_a[256];
  static __shared__ float matrix_b[256];
  static __shared__ float matrix_c[256];
  workgroup_barrier();
  cooperative_matrix_multiply(&matrix_a[0], &matrix_b[0], &matrix_c[0], 16, 16,
                              16);
  workgroup_barrier();
  output->data[0] = matrix_c[0];
}
