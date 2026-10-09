// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -emit-llvm -o - %s | FileCheck %s
// CHECK-DAG: define void @_Z5shadej()
// CHECK-DAG: define void @_Z5shadef()
// CHECK-DAG: define void @_ZN6nested5shadeEv()
// CHECK-DAG: define void @plain()
// CHECK-DAG: "spirv.cxx.source_name"="nested::shade"
__compute__ __workgroup_size__(1, 1, 1)
void shade(__input__(0) unsigned value) {}
__compute__ __workgroup_size__(1, 1, 1)
void shade(__input__(0) float value) {}
namespace nested {
__compute__ __workgroup_size__(1, 1, 1)
void shade() {}
}
extern "C" {
__compute__ __workgroup_size__(1, 1, 1)
void plain() {}
}
