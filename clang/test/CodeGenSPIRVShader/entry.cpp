// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -emit-llvm -o - %s | FileCheck %s
// CHECK: %struct.Params = type { ptr addrspace(14), i32 }
// CHECK: @__spirv_cxx_scale_arg0 = external addrspace(13) global %struct.Params
// CHECK: define void @scale()
// CHECK: call{{.*}} void @_Z5scale
// CHECK: "hlsl.numthreads"="64,1,1"
// CHECK: "hlsl.shader"="compute"
// CHECK: "spirv.cxx.stage"="compute"
// CHECK: !spirv.MemoryModel
// CHECK: !{i32 5348, i32 3}
struct Params { __storage__ float *data; unsigned count; };
__compute__ __workgroup_size__(64, 1, 1)
void scale(__push_constant__ const Params *p) {
  if (global_invocation_id.x < p->count)
    p->data[global_invocation_id[0]] *= 2.0f;
}
