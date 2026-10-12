// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=cooperative_matrix -o %t.spv %t.bc

__compute__
__workgroup_size__(32, 1, 1)
void cooperative_matrix() {
  static __shared__ float matrix_a[256];
  static __shared__ float matrix_b[256];
  static __shared__ float matrix_c[256];
  cooperative_matrix_multiply(&matrix_a[0], &matrix_b[0], &matrix_c[0], 16, 16,
                              16);
}
