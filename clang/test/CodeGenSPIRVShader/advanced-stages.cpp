// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=geometry_stage -o %t.geometry_stage.spv %t.bc
// RUN: spirv-dis %t.geometry_stage.spv | FileCheck %s --check-prefix=GEOMETRY-STAGE
// RUN: clang-spirv-link --entry=tess_control -o %t.tess_control.spv %t.bc
// RUN: spirv-dis %t.tess_control.spv | FileCheck %s --check-prefix=TESS-CONTROL
// RUN: clang-spirv-link --entry=tess_evaluation -o %t.tess_evaluation.spv %t.bc
// RUN: spirv-dis %t.tess_evaluation.spv | FileCheck %s --check-prefix=TESS-EVALUATION
// RUN: clang-spirv-link --entry=task_stage -o %t.task_stage.spv %t.bc
// RUN: spirv-dis %t.task_stage.spv | FileCheck %s --check-prefix=TASK-STAGE
// RUN: clang-spirv-link --entry=mesh_stage -o %t.mesh_stage.spv %t.bc
// RUN: spirv-dis %t.mesh_stage.spv | FileCheck %s --check-prefix=MESH-STAGE
// RUN: clang-spirv-link --entry=ray_generation_stage -o %t.ray_generation_stage.spv %t.bc
// RUN: spirv-dis %t.ray_generation_stage.spv | FileCheck %s --check-prefix=RAY-GENERATION-STAGE
// RUN: clang-spirv-link --entry=intersection_stage -o %t.intersection_stage.spv %t.bc
// RUN: spirv-dis %t.intersection_stage.spv | FileCheck %s --check-prefix=INTERSECTION-STAGE
// RUN: clang-spirv-link --entry=any_hit_stage -o %t.any_hit_stage.spv %t.bc
// RUN: spirv-dis %t.any_hit_stage.spv | FileCheck %s --check-prefix=ANY-HIT-STAGE
// RUN: clang-spirv-link --entry=closest_hit_stage -o %t.closest_hit_stage.spv %t.bc
// RUN: spirv-dis %t.closest_hit_stage.spv | FileCheck %s --check-prefix=CLOSEST-HIT-STAGE
// RUN: clang-spirv-link --entry=miss_stage -o %t.miss_stage.spv %t.bc
// RUN: spirv-dis %t.miss_stage.spv | FileCheck %s --check-prefix=MISS-STAGE
// RUN: clang-spirv-link --entry=callable_stage -o %t.callable_stage.spv %t.bc
// RUN: spirv-dis %t.callable_stage.spv | FileCheck %s --check-prefix=CALLABLE-STAGE
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=geometry_stage -o %t.geometry_stage.spv %t.bc
// RUN: spirv-dis %t.geometry_stage.spv | FileCheck %s --check-prefix=GEOMETRY-STAGE
// RUN: clang-spirv-link --entry=tess_control -o %t.tess_control.spv %t.bc
// RUN: spirv-dis %t.tess_control.spv | FileCheck %s --check-prefix=TESS-CONTROL
// RUN: clang-spirv-link --entry=tess_evaluation -o %t.tess_evaluation.spv %t.bc
// RUN: spirv-dis %t.tess_evaluation.spv | FileCheck %s --check-prefix=TESS-EVALUATION
// RUN: clang-spirv-link --entry=task_stage -o %t.task_stage.spv %t.bc
// RUN: spirv-dis %t.task_stage.spv | FileCheck %s --check-prefix=TASK-STAGE
// RUN: clang-spirv-link --entry=mesh_stage -o %t.mesh_stage.spv %t.bc
// RUN: spirv-dis %t.mesh_stage.spv | FileCheck %s --check-prefix=MESH-STAGE
// RUN: clang-spirv-link --entry=ray_generation_stage -o %t.ray_generation_stage.spv %t.bc
// RUN: spirv-dis %t.ray_generation_stage.spv | FileCheck %s --check-prefix=RAY-GENERATION-STAGE
// RUN: clang-spirv-link --entry=intersection_stage -o %t.intersection_stage.spv %t.bc
// RUN: spirv-dis %t.intersection_stage.spv | FileCheck %s --check-prefix=INTERSECTION-STAGE
// RUN: clang-spirv-link --entry=any_hit_stage -o %t.any_hit_stage.spv %t.bc
// RUN: spirv-dis %t.any_hit_stage.spv | FileCheck %s --check-prefix=ANY-HIT-STAGE
// RUN: clang-spirv-link --entry=closest_hit_stage -o %t.closest_hit_stage.spv %t.bc
// RUN: spirv-dis %t.closest_hit_stage.spv | FileCheck %s --check-prefix=CLOSEST-HIT-STAGE
// RUN: clang-spirv-link --entry=miss_stage -o %t.miss_stage.spv %t.bc
// RUN: spirv-dis %t.miss_stage.spv | FileCheck %s --check-prefix=MISS-STAGE
// RUN: clang-spirv-link --entry=callable_stage -o %t.callable_stage.spv %t.bc
// RUN: spirv-dis %t.callable_stage.spv | FileCheck %s --check-prefix=CALLABLE-STAGE

// RUN: clang-spirv-link --entry=geometry_stream_stage -o %t.stream.spv %t.bc
// RUN: spirv-dis %t.stream.spv | FileCheck %s --check-prefix=STREAM

#include "../../../shader-cxx/tests/fixtures/advanced-stages.cpp"

// STREAM-DAG: OpEmitStreamVertex
// STREAM-DAG: OpEndStreamPrimitive

// GEOMETRY-STAGE-DAG: OpEntryPoint Geometry
// GEOMETRY-STAGE-DAG: OpExecutionMode {{.*}} Triangles
// GEOMETRY-STAGE-DAG: OpExecutionMode {{.*}} OutputTriangleStrip
// GEOMETRY-STAGE-DAG: OpEmitVertex
// GEOMETRY-STAGE-DAG: OpEndPrimitive
// TESS-CONTROL-DAG: OpEntryPoint TessellationControl
// TESS-CONTROL-DAG: OpExecutionMode {{.*}} OutputVertices 3
// TESS-CONTROL-DAG: Patch
// TESS-CONTROL-DAG: TessLevelOuter
// TESS-EVALUATION-DAG: OpEntryPoint TessellationEvaluation
// TESS-EVALUATION-DAG: OpExecutionMode {{.*}} Triangles
// TESS-EVALUATION-DAG: OpExecutionMode {{.*}} SpacingEqual
// TESS-EVALUATION-DAG: OpExecutionMode {{.*}} VertexOrderCcw
// TASK-STAGE-DAG: OpEntryPoint TaskEXT
// TASK-STAGE-DAG: TaskPayloadWorkgroupEXT
// TASK-STAGE-DAG: OpEmitMeshTasksEXT
// MESH-STAGE-DAG: OpEntryPoint MeshEXT
// MESH-STAGE-DAG: OpExecutionMode {{.*}} OutputVertices 1
// MESH-STAGE-DAG: OpExecutionMode {{.*}} OutputPrimitivesEXT 1
// MESH-STAGE-DAG: OpSetMeshOutputsEXT
// RAY-GENERATION-STAGE-DAG: OpEntryPoint RayGenerationKHR
// RAY-GENERATION-STAGE-DAG: OpTraceRayKHR
// RAY-GENERATION-STAGE-DAG: OpExecuteCallableKHR
// INTERSECTION-STAGE-DAG: OpEntryPoint IntersectionKHR
// INTERSECTION-STAGE-DAG: OpReportIntersectionKHR
// ANY-HIT-STAGE-DAG: OpEntryPoint AnyHitKHR
// ANY-HIT-STAGE-DAG: OpIgnoreIntersectionKHR
// ANY-HIT-STAGE-DAG: OpTerminateRayKHR
// CLOSEST-HIT-STAGE-DAG: OpEntryPoint ClosestHitKHR
// CLOSEST-HIT-STAGE-DAG: ShaderRecordBufferKHR
// MISS-STAGE-DAG: OpEntryPoint MissKHR
// CALLABLE-STAGE-DAG: OpEntryPoint CallableKHR
