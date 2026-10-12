// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: not clang-spirv-link --entry=compute_interlock \
// RUN:   -o %t.compute.spv %t.bc 2>&1 | FileCheck %s --check-prefix=STAGE
// RUN: not clang-spirv-link --entry=unpaired_interlock \
// RUN:   -o %t.unpaired.spv %t.bc 2>&1 | FileCheck %s --check-prefix=PAIR

// STAGE: fragment interlock requires a fragment entry
// PAIR: fragment interlock requires both begin and end calls

__compute
__workgroup_size(1, 1, 1)
void compute_interlock() {
  begin_fragment_interlock();
  end_fragment_interlock();
}

__fragment
void unpaired_interlock() { begin_fragment_interlock(); }
