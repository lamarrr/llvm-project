// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: not clang-spirv-link --entry=bad_geometry -o %t.spv %t.bc 2>&1 | FileCheck %s --check-prefix=GEOMETRY
// RUN: not clang-spirv-link --entry=bad_mesh -o %t.spv %t.bc 2>&1 | FileCheck %s --check-prefix=MESH
// RUN: not clang-spirv-link --entry=bad_task -o %t.spv %t.bc 2>&1 | FileCheck %s --check-prefix=TASK
// RUN: not clang-spirv-link --entry=bad_intersection -o %t.spv %t.bc 2>&1 | FileCheck %s --check-prefix=INTERSECTION
// RUN: not clang-spirv-link --entry=bad_hit -o %t.spv %t.bc 2>&1 | FileCheck %s --check-prefix=HIT

void emit_helper() { emit_vertex(); }
__compute
__workgroup_size(1, 1, 1)
void bad_geometry() { emit_helper(); }
__compute
__workgroup_size(1, 1, 1)
void bad_mesh() { set_mesh_outputs(1u, 1u); }
__compute
__workgroup_size(1, 1, 1)
void bad_task() { emit_mesh_tasks(1u, 1u, 1u); }
__compute
__workgroup_size(1, 1, 1)
void bad_intersection() { report_intersection(1.0f, 1u); }
__compute
__workgroup_size(1, 1, 1)
void bad_hit() { terminate_ray(); }

// GEOMETRY: stage operation 'EmitVertex' is unavailable in compute
// MESH: stage operation 'SetMeshOutputsEXT' is unavailable in compute
// TASK: stage operation 'EmitMeshTasksEXT' is unavailable in compute
// INTERSECTION: stage operation 'ReportIntersectionKHR' is unavailable in compute
// HIT: stage operation 'TerminateRayKHR' is unavailable in compute
