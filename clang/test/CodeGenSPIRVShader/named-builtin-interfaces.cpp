// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=named_geometry -o %t.geometry.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.geometry.spv
// RUN: spirv-dis %t.geometry.spv | FileCheck %s --check-prefix=GEOMETRY
// RUN: clang-spirv-link --entry=named_control -o %t.control.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.control.spv
// RUN: spirv-dis %t.control.spv | FileCheck %s --check-prefix=CONTROL
// RUN: clang-spirv-link --entry=named_line_mesh -o %t.mesh.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mesh.spv
// RUN: spirv-dis %t.mesh.spv | FileCheck %s --check-prefix=MESH
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=named_geometry -o %t.geometry.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.geometry.spv
// RUN: spirv-dis %t.geometry.spv | FileCheck %s --check-prefix=GEOMETRY
// RUN: clang-spirv-link --entry=named_control -o %t.control.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.control.spv
// RUN: spirv-dis %t.control.spv | FileCheck %s --check-prefix=CONTROL
// RUN: clang-spirv-link --entry=named_line_mesh -o %t.mesh.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mesh.spv
// RUN: spirv-dis %t.mesh.spv | FileCheck %s --check-prefix=MESH
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O3 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=named_geometry -o %t.geometry.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.geometry.spv
// RUN: spirv-dis %t.geometry.spv | FileCheck %s --check-prefix=GEOMETRY
// RUN: clang-spirv-link --entry=named_control -o %t.control.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.control.spv
// RUN: spirv-dis %t.control.spv | FileCheck %s --check-prefix=CONTROL
// RUN: clang-spirv-link --entry=named_line_mesh -o %t.mesh.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mesh.spv
// RUN: spirv-dis %t.mesh.spv | FileCheck %s --check-prefix=MESH

struct Float4 {
  float x, y, z, w;
};
struct UInt2 {
  unsigned x, y;
};

__geometry
__geometry_mode(geometry_input::points, geometry_output::points, 1)
void named_geometry(__primitive_id const int *input_id,
                    __invocation_id const int *invocation,
                    __primitive_id int *output_id, __layer int *layer,
                    __viewport_index int *viewport,
                    __position Float4 *position) {
  *output_id = *input_id;
  *layer = *invocation;
  *viewport = 0;
  position->x = float(*input_id);
  position->y = position->z = 0.0f;
  position->w = 1.0f;
  emit_vertex();
  end_primitive();
}

__tessellation_control
__output_vertices(3)
void named_control(__patch_vertices const int *vertices,
                   __invocation_id const int *invocation,
                   __primitive_id const int *primitive,
                   __tess_level_outer __patch Float4 *outer,
                   __tess_level_inner __patch float (*inner)[2]) {
  if (*invocation == 0) {
    outer->x = float(*vertices + *primitive);
    outer->y = outer->z = outer->w = 1.0f;
    (*inner)[0] = (*inner)[1] = 1.0f;
  }
}

__mesh
__workgroup_size(1, 1, 1)
__mesh_output(mesh_topology::lines, 2, 1)
void named_line_mesh(__position Float4 (*positions)[2],
                     __primitive_line_indices UInt2 (*indices)[1],
                     __cull_primitive bool (*cull)[1]) {
  set_mesh_outputs(2u, 1u);
  for (unsigned index = 0; index != 2; ++index) {
    (*positions)[index].x = float(index);
    (*positions)[index].y = (*positions)[index].z = 0.0f;
    (*positions)[index].w = 1.0f;
  }
  (*indices)[0].x = 0u;
  (*indices)[0].y = 1u;
  (*cull)[0] = false;
}

// GEOMETRY-DAG: OpDecorate {{.*}} BuiltIn PrimitiveId
// GEOMETRY-DAG: OpDecorate {{.*}} BuiltIn InvocationId
// GEOMETRY-DAG: OpDecorate {{.*}} BuiltIn Layer
// GEOMETRY-DAG: OpDecorate {{.*}} BuiltIn ViewportIndex
// GEOMETRY-DAG: OpDecorate {{.*}} BuiltIn Position
// CONTROL-DAG: OpDecorate {{.*}} BuiltIn PatchVertices
// CONTROL-DAG: OpDecorate {{.*}} BuiltIn InvocationId
// CONTROL-DAG: OpDecorate {{.*}} BuiltIn PrimitiveId
// CONTROL-DAG: OpDecorate {{.*}} BuiltIn TessLevelOuter
// CONTROL-DAG: OpDecorate {{.*}} BuiltIn TessLevelInner
// MESH-DAG: OpDecorate {{.*}} BuiltIn PrimitiveLineIndicesEXT
// MESH-DAG: OpDecorate {{.*}} BuiltIn CullPrimitiveEXT
// MESH-DAG: OpDecorate {{.*}} PerPrimitiveEXT
