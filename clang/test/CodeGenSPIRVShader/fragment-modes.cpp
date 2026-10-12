// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=fragment_modes -o %t.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 --scalar-block-layout %t.spv
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: clang-spirv-link --entry=post_fragment -o %t-post.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 --scalar-block-layout %t-post.spv
// RUN: spirv-dis %t-post.spv | FileCheck %s --check-prefix=POST
// RUN: clang-spirv-link --entry=stencil_fragment -o %t-stencil.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 --scalar-block-layout %t-stencil.spv
// RUN: spirv-dis %t-stencil.spv | FileCheck %s --check-prefix=STENCIL

// CHECK: OpEntryPoint Fragment %[[ENTRY:[^ ]+]]
// CHECK: OpExecutionMode %[[ENTRY]] DepthReplacing
// CHECK: OpExecutionMode %[[ENTRY]] EarlyFragmentTests
// CHECK: OpExecutionMode %[[ENTRY]] DepthGreater
// CHECK: OpExecutionMode %[[ENTRY]] OriginUpperLeft

__early_fragment_tests__ __depth_greater__ __fragment__
void fragment_modes(__frag_depth__ float *depth) { *depth = 0.5f; }

// POST: OpCapability SampleMaskPostDepthCoverage
// POST: OpExtension "SPV_KHR_post_depth_coverage"
// POST: OpExecutionMode {{%[^ ]+}} EarlyFragmentTests
// POST: OpExecutionMode {{%[^ ]+}} PostDepthCoverage
// POST: OpDecorate {{%[^ ]+}} BuiltIn SampleMask
__early_fragment_tests__ __post_depth_coverage__ __fragment__
void post_fragment(__sample_mask__ const unsigned (*mask)[1],
                   __output__(0) float *color) {
  *color = ((*mask)[0] & 1u) ? 1.0f : 0.0f;
}

// STENCIL: OpCapability StencilExportEXT
// STENCIL: OpExtension "SPV_EXT_shader_stencil_export"
// STENCIL: OpExecutionMode {{%[^ ]+}} StencilRefReplacingEXT
// STENCIL: OpDecorate {{%[^ ]+}} BuiltIn FragStencilRefEXT
__fragment__
void stencil_fragment(__stencil_ref__ unsigned *stencil) { *stencil = 1u; }
