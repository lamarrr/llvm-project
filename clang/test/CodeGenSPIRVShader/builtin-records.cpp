// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=record_vertex -o %t.record_vertex.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_vertex.spv
// RUN: spirv-dis %t.record_vertex.spv | FileCheck %s --check-prefix=RECORD-VERTEX
// RUN: clang-spirv-link --entry=record_fragment -o %t.record_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_fragment.spv
// RUN: spirv-dis %t.record_fragment.spv | FileCheck %s --check-prefix=RECORD-FRAGMENT
// RUN: clang-spirv-link --entry=record_control -o %t.record_control.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_control.spv
// RUN: spirv-dis %t.record_control.spv | FileCheck %s --check-prefix=RECORD-CONTROL
// RUN: clang-spirv-link --entry=record_evaluation -o %t.record_evaluation.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_evaluation.spv
// RUN: spirv-dis %t.record_evaluation.spv | FileCheck %s --check-prefix=RECORD-EVALUATION
// RUN: clang-spirv-link --entry=record_mesh -o %t.record_mesh.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_mesh.spv
// RUN: spirv-dis %t.record_mesh.spv | FileCheck %s --check-prefix=RECORD-MESH
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=record_vertex -o %t.record_vertex.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_vertex.spv
// RUN: spirv-dis %t.record_vertex.spv | FileCheck %s --check-prefix=RECORD-VERTEX
// RUN: clang-spirv-link --entry=record_fragment -o %t.record_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_fragment.spv
// RUN: spirv-dis %t.record_fragment.spv | FileCheck %s --check-prefix=RECORD-FRAGMENT
// RUN: clang-spirv-link --entry=record_control -o %t.record_control.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_control.spv
// RUN: spirv-dis %t.record_control.spv | FileCheck %s --check-prefix=RECORD-CONTROL
// RUN: clang-spirv-link --entry=record_evaluation -o %t.record_evaluation.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_evaluation.spv
// RUN: spirv-dis %t.record_evaluation.spv | FileCheck %s --check-prefix=RECORD-EVALUATION
// RUN: clang-spirv-link --entry=record_mesh -o %t.record_mesh.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.record_mesh.spv
// RUN: spirv-dis %t.record_mesh.spv | FileCheck %s --check-prefix=RECORD-MESH

#include <spirv/types.hpp>
using namespace spirv;

struct alignas(16) Float4 {
  float x, y, z, w;
};
struct Float1 {
  float value;
};
struct UInt1 {
  unsigned value;
};
struct NestedUInt {
  UInt1 value;
};
struct Bool1 {
  bool value;
};
struct Positions {
  Float4 values[3];
};

// RECORD-VERTEX-DAG: BuiltIn Position
// RECORD-VERTEX-DAG: BuiltIn PointSize
// RECORD-VERTEX-DAG: BuiltIn ClipDistance
// RECORD-VERTEX-DAG: BuiltIn CullDistance
// RECORD-VERTEX-DAG: OpTypeVector {{.*}} 4
// RECORD-VERTEX-DAG: OpTypeArray
__vertex
void record_vertex(__position Float4 *position,
                   __point_size Float1 *size,
                   __clip_distance f32x2 *clip,
                   __cull_distance Float1 *cull) {
  *position = Float4{0, 0, 0, 1};
  *size = Float1{1};
  (*clip)[vertex_idx & 1] = 1.0f;
  cull->value = 1.0f;
}

// RECORD-FRAGMENT-DAG: BuiltIn FragDepth
// RECORD-FRAGMENT-DAG: BuiltIn SampleMask
// RECORD-FRAGMENT-DAG: BuiltIn FragStencilRefEXT
// RECORD-FRAGMENT-DAG: BuiltIn ClipDistance
// RECORD-FRAGMENT-DAG: BuiltIn CullDistance
// RECORD-FRAGMENT-DAG: OpTypeArray
__fragment
void record_fragment(__frag_depth vec<float, 1> *depth,
                     __sample_mask const NestedUInt *input_mask,
                     __sample_mask vec<unsigned, 1> *output_mask,
                     __stencil_ref UInt1 *stencil,
                     __clip_distance const f32x2 *clip,
                     __cull_distance const Float1 *cull) {
  NestedUInt mask{input_mask->value.value};
  (*depth)[0] = (*clip)[sample_id & 1] + cull->value;
  (*output_mask)[0] = mask.value.value;
  *stencil = UInt1{mask.value.value};
}

// RECORD-CONTROL-DAG: BuiltIn Position
// RECORD-CONTROL-DAG: BuiltIn TessLevelOuter
// RECORD-CONTROL-DAG: BuiltIn TessLevelInner
// RECORD-CONTROL-DAG: OpTypeArray
__tessellation_control __output_vertices(3)
void record_control(__position const Positions *input,
                    __position Positions *output,
                    __tess_level_outer __patch f32x4 *outer,
                    __tess_level_inner __patch f32x2 *inner) {
  output->values[invocation_id] = Float4{
      input->values[invocation_id].x, input->values[invocation_id].y,
      input->values[invocation_id].z, input->values[invocation_id].w};
  if (invocation_id == 0) {
    *outer = f32x4(1);
    (*inner)[invocation_id & 1] = 1;
  }
}

// RECORD-EVALUATION-DAG: BuiltIn TessCoord
// RECORD-EVALUATION-DAG: BuiltIn TessLevelOuter
// RECORD-EVALUATION-DAG: BuiltIn TessLevelInner
// RECORD-EVALUATION-DAG: BuiltIn Position
// RECORD-EVALUATION-DAG: OpTypeVector {{.*}} 3
// RECORD-EVALUATION-DAG: OpTypeArray
__tessellation_evaluation
__tessellation_mode(tessellation_domain::triangles, tessellation_spacing::equal,
                      vertex_order::counter_clockwise)
void record_evaluation(__tess_coord const f32x3 *coord,
                       __tess_level_outer __patch const Float4 *outer,
                       __tess_level_inner __patch const f32x2 *inner,
                       __position Float4 *position) {
  f32x3 value((*coord)[0], (*coord)[1], (*coord)[2]);
  *position = Float4{value[0], value[1], outer->z + (*inner)[0], 1};
}

// RECORD-MESH-DAG: BuiltIn Position
// RECORD-MESH-DAG: BuiltIn PrimitiveTriangleIndicesEXT
// RECORD-MESH-DAG: BuiltIn CullPrimitiveEXT
// RECORD-MESH-DAG: BuiltIn PrimitiveId
// RECORD-MESH-DAG: OpTypeBool
// RECORD-MESH-DAG: OpTypeVector {{.*}} 3
// RECORD-MESH-DAG: OpTypeArray
__mesh __workgroup_size(1, 1, 1)
__mesh_output(mesh_topology::triangles, 3, 1)
void record_mesh(__position Positions *positions,
                 __primitive_triangle_indices
                     u32x3 (*indices)[1],
                 __cull_primitive Bool1 (*cull)[1],
                 __primitive_id __per_primitive UInt1 (*ids)[1]) {
  set_mesh_outputs(3u, 1u);
  for (unsigned vertex = 0; vertex != 3; ++vertex)
    positions->values[vertex] = Float4{float(vertex), 0, 0, 1};
  (*indices)[0] = u32x3(0, 1, 2);
  (*cull)[local_invocation_idx].value = false;
  (*ids)[0].value = 0;
}
