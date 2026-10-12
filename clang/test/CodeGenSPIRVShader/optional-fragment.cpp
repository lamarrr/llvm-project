// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=optional_fragment -o %t.spv %t.bc

__fragment
void optional_fragment(__output(0) float *color) {
  *color = bary_coord[0] + bary_coord_noperspective[1] +
           static_cast<float>(shading_rate);
}
