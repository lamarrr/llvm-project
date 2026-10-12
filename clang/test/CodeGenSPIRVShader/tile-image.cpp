// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=tile_reads -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=tile_reads -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s
// RUN: not clang-spirv-link --entry=wrong_stage -o %t.bad.spv %t.bc 2>&1 | FileCheck %s --check-prefix=STAGE
// RUN: not clang-spirv-link --entry=wrong_early -o %t.bad.spv %t.bc 2>&1 | FileCheck %s --check-prefix=EARLY

// CHECK-DAG: OpCapability TileImageColorReadAccessEXT
// CHECK-DAG: OpCapability TileImageDepthReadAccessEXT
// CHECK-DAG: OpCapability TileImageStencilReadAccessEXT
// CHECK-DAG: OpExtension "SPV_EXT_shader_tile_image"
// CHECK-DAG: OpExecutionMode {{%[^ ]+}} NonCoherentColorAttachmentReadEXT
// CHECK-DAG: OpExecutionMode {{%[^ ]+}} NonCoherentDepthAttachmentReadEXT
// CHECK-DAG: OpExecutionMode {{%[^ ]+}} NonCoherentStencilAttachmentReadEXT
// CHECK: OpTypeImage {{%[^ ]+}} TileImageDataEXT 0 0 0 2 Unknown
// CHECK: OpVariable {{%[^ ]+}} TileImageEXT
// CHECK: OpColorAttachmentReadEXT
// CHECK: OpColorAttachmentReadEXT
// CHECK: OpColorAttachmentReadEXT
// CHECK: OpDepthAttachmentReadEXT
// CHECK: OpStencilAttachmentReadEXT
// STAGE: tile attachment reads require a fragment entry
// EARLY: depth and stencil attachment reads cannot use early fragment tests

struct Pair { int first, second; };

__fragment__
__noncoherent_color_attachment_read__
__noncoherent_depth_attachment_read__
__noncoherent_stencil_attachment_read__
void tile_reads(tile_image<float, 0> color,
                tile_image<Pair, 1> signed_color,
                tile_image<unsigned, 2> unsigned_color,
                __output__(0) float *output) {
  Pair pair = signed_color.load(1u);
  *output = color.load() + static_cast<float>(pair.first) +
            static_cast<float>(unsigned_color.load()) +
            depth_attachment_read() + static_cast<float>(stencil_attachment_read(1u));
}

__compute__ __workgroup_size__(1, 1, 1)
void wrong_stage(__push_constant__ const float *value) {
  if (depth_attachment_read() > *value)
    memory_barrier(1u, 0u);
}

__fragment__ __early_fragment_tests__
void wrong_early(__output__(0) float *color) { *color = depth_attachment_read(); }
