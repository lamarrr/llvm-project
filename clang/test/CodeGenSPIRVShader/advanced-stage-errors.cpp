// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST=1 -emit-llvm -o %t.ll %s 2>&1 | FileCheck %s
// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST=2 -emit-llvm -o %t.ll %s 2>&1 | FileCheck %s
// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST=3 -emit-llvm -o %t.ll %s 2>&1 | FileCheck %s
// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST=4 -emit-llvm -o %t.ll %s 2>&1 | FileCheck %s
// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST=5 -emit-llvm -o %t.ll %s 2>&1 | FileCheck %s
// RUN: not %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -DTEST=6 -emit-llvm -o %t.ll %s 2>&1 | FileCheck %s --check-prefix=PAYLOAD

#if TEST == 1
__geometry__ void missing_geometry() {}
#elif TEST == 2
__tessellation_control__ void missing_control() {}
#elif TEST == 3
__tessellation_evaluation__ void missing_evaluation() {}
#elif TEST == 4
__mesh__ __workgroup_size__(1, 1, 1) void missing_mesh() {}
#elif TEST == 5
__vertex__ __geometry_mode__(22, 29, 3) void wrong_mode() {}
#elif TEST == 6
__compute__ __workgroup_size__(1, 1, 1)
void wrong_payload(__incoming_ray_payload__ unsigned *payload) {}
#endif

// CHECK: advanced stage requires its matching execution mode annotation
// PAYLOAD: shader payload or interface qualifier is invalid for this stage
