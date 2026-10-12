// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -emit-llvm -o - %s | FileCheck %s

int read_push(__push_constant const int *source) {
  const int *deduced = source;
  return *deduced;
}

// CHECK-LABEL: define spir_func noundef i32 @{{.*}}read_push
// CHECK: %deduced = alloca ptr addrspace(13)
// CHECK-NOT: addrspacecast
// CHECK: load i32, ptr addrspace(13)

int read_uniform(__uniform const int *source) {
  const int *deduced = source;
  return *deduced;
}

// CHECK-LABEL: define spir_func noundef i32 @{{.*}}read_uniform
// CHECK: %deduced = alloca ptr addrspace(12)
// CHECK-NOT: addrspacecast
// CHECK: load i32, ptr addrspace(12)

void write_storage(__storage int *source) {
  int *deduced = source;
  *deduced = 17;
}

// CHECK-LABEL: define spir_func void @{{.*}}write_storage
// CHECK: %deduced = alloca ptr addrspace(14)
// CHECK-NOT: addrspacecast
// CHECK: store i32 17, ptr addrspace(14)
