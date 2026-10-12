// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=specialized_workgroup -o %t.spv %t.bc

__compute__
__workgroup_size_id__(0, 4, 1, 2, 2, 1)
void specialized_workgroup() {}
