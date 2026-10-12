// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=fragment_interlock -o %t.spv %t.bc

__fragment__
void fragment_interlock(__output__(0) float *color) {
  begin_fragment_interlock();
  *color = 1.0f;
  end_fragment_interlock();
}
