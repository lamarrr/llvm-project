// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -emit-llvm -o - %s | FileCheck %s
// CHECK-LABEL: define spir_func void @_Z12copy_records
// CHECK-NOT: llvm.memcpy
// CHECK-NOT: addrspacecast
// CHECK: getelementptr inbounds {{.*}} [4 x %struct.Payload], ptr addrspace(13)
// CHECK: load float, ptr addrspace(13)
// CHECK-NOT: llvm.memcpy
// CHECK-NOT: addrspacecast
// CHECK: load float, ptr addrspace(12)
// CHECK-NOT: llvm.memcpy
// CHECK-NOT: addrspacecast
// CHECK: load float, ptr addrspace(14)
// CHECK-NOT: llvm.memcpy
// CHECK-NOT: addrspacecast
// CHECK: ret void

struct Payload {
  float value;
  float weights[2];
};
struct Params {
  Payload records[4];
  __storage__ Payload *device;
};

__fragment__
void copy_records(__push_constant__ const Params *push,
                  __binding__(0, 0) __uniform__ const Payload *uniform,
                  __input__(0) __flat__ unsigned index,
                  __output__(0) float *output) {
  Payload from_push = push->records[index & 3];
  Payload from_uniform = *uniform;
  Payload from_storage = *push->device;
  *output = from_push.value + from_uniform.value + from_storage.value;
}
