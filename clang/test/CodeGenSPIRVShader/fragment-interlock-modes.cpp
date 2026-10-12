// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=pixel_ordered -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s --check-prefix=MODE0
// RUN: clang-spirv-link --entry=pixel_unordered -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s --check-prefix=MODE1
// RUN: clang-spirv-link --entry=sample_ordered -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s --check-prefix=MODE2
// RUN: clang-spirv-link --entry=sample_unordered -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s --check-prefix=MODE3
// RUN: clang-spirv-link --entry=shading_rate_ordered -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s --check-prefix=MODE4
// RUN: clang-spirv-link --entry=shading_rate_unordered -o %t.spv %t.bc
// RUN: spirv-dis %t.spv | FileCheck %s --check-prefix=MODE5

// MODE0: OpExecutionMode {{%[^ ]+}} PixelInterlockOrderedEXT
__fragment
__fragment_interlock(interlock_mode::pixel_ordered)
void pixel_ordered(__output(0) float *color) {
  begin_fragment_interlock();
  *color = 1.0f;
  end_fragment_interlock();
}

// MODE1: OpExecutionMode {{%[^ ]+}} PixelInterlockUnorderedEXT
__fragment
__fragment_interlock(interlock_mode::pixel_unordered)
void pixel_unordered(__output(0) float *color) {
  begin_fragment_interlock();
  *color = 1.0f;
  end_fragment_interlock();
}

// MODE2: OpExecutionMode {{%[^ ]+}} SampleInterlockOrderedEXT
__fragment
__fragment_interlock(interlock_mode::sample_ordered)
void sample_ordered(__output(0) float *color) {
  begin_fragment_interlock();
  *color = 1.0f;
  end_fragment_interlock();
}

// MODE3: OpExecutionMode {{%[^ ]+}} SampleInterlockUnorderedEXT
__fragment
__fragment_interlock(interlock_mode::sample_unordered)
void sample_unordered(__output(0) float *color) {
  begin_fragment_interlock();
  *color = 1.0f;
  end_fragment_interlock();
}

// MODE4: OpExecutionMode {{%[^ ]+}} ShadingRateInterlockOrderedEXT
__fragment
__fragment_interlock(interlock_mode::shading_rate_ordered)
void shading_rate_ordered(__output(0) float *color) {
  begin_fragment_interlock();
  *color = 1.0f;
  end_fragment_interlock();
}

// MODE5: OpExecutionMode {{%[^ ]+}} ShadingRateInterlockUnorderedEXT
__fragment
__fragment_interlock(interlock_mode::shading_rate_unordered)
void shading_rate_unordered(__output(0) float *color) {
  begin_fragment_interlock();
  *color = 1.0f;
  end_fragment_interlock();
}
