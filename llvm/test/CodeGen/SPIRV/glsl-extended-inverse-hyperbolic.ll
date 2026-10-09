; RUN: llc -O0 -mtriple=spirv1.4-unknown-vulkan1.3 %s -o - | FileCheck %s
; RUN: llc -O2 -mtriple=spirv1.4-unknown-vulkan1.3 %s -o - | FileCheck %s
; GLSL extended calls must use GLSL.std.450, not colliding OpenCL opcode IDs.
; CHECK-DAG: [[SET:%[0-9]+]] = OpExtInstImport "GLSL.std.450"
; CHECK-DAG: [[FLOAT:%[0-9]+]] = OpTypeFloat 32
; CHECK-DAG: [[VECTOR:%[0-9]+]] = OpTypeVector [[FLOAT]] 2
; CHECK: OpExtInst [[FLOAT]] [[SET]] Asinh
; CHECK: OpExtInst [[FLOAT]] [[SET]] Acosh
; CHECK: OpExtInst [[FLOAT]] [[SET]] Atanh
; CHECK: OpExtInst [[VECTOR]] [[SET]] Asinh
; CHECK: OpExtInst [[VECTOR]] [[SET]] Acosh
; CHECK: OpExtInst [[VECTOR]] [[SET]] Atanh

@scalar = external addrspace(3) global float
@vector = external addrspace(3) global <2 x float>

declare float @_Z20__spirv_shader_AsinhILy1EEvv(float)
declare float @_Z20__spirv_shader_AcoshILy1EEvv(float)
declare float @_Z20__spirv_shader_AtanhILy1EEvv(float)
declare <2 x float> @_Z20__spirv_shader_AsinhILy2EEvv(<2 x float>)
declare <2 x float> @_Z20__spirv_shader_AcoshILy2EEvv(<2 x float>)
declare <2 x float> @_Z20__spirv_shader_AtanhILy2EEvv(<2 x float>)

define void @main(float %x, <2 x float> %v) {
  %a = call float @_Z20__spirv_shader_AsinhILy1EEvv(float %x)
  store float %a, ptr addrspace(3) @scalar
  %b = call float @_Z20__spirv_shader_AcoshILy1EEvv(float %x)
  store float %b, ptr addrspace(3) @scalar
  %c = call float @_Z20__spirv_shader_AtanhILy1EEvv(float %x)
  store float %c, ptr addrspace(3) @scalar
  %d = call <2 x float> @_Z20__spirv_shader_AsinhILy2EEvv(<2 x float> %v)
  store <2 x float> %d, ptr addrspace(3) @vector
  %e = call <2 x float> @_Z20__spirv_shader_AcoshILy2EEvv(<2 x float> %v)
  store <2 x float> %e, ptr addrspace(3) @vector
  %f = call <2 x float> @_Z20__spirv_shader_AtanhILy2EEvv(<2 x float> %v)
  store <2 x float> %f, ptr addrspace(3) @vector
  ret void
}
