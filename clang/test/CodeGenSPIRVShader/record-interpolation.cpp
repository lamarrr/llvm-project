// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=mixed_vertex -o %t.mixed_vertex.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mixed_vertex.spv
// RUN: spirv-dis %t.mixed_vertex.spv | FileCheck %s --check-prefix=MIXED --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} NoPerspective" --implicit-check-not="OpDecorate {{%[^ ]*_plain_[^ ]*}} Flat"
// RUN: clang-spirv-link --entry=mixed_fragment -o %t.mixed_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mixed_fragment.spv
// RUN: spirv-dis %t.mixed_fragment.spv | FileCheck %s --check-prefix=MIXED --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} NoPerspective" --implicit-check-not="OpDecorate {{%[^ ]*_plain_[^ ]*}} Flat"
// RUN: clang-spirv-link --entry=inherited_fragment -o %t.inherited_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.inherited_fragment.spv
// RUN: spirv-dis %t.inherited_fragment.spv | FileCheck %s --check-prefix=INHERIT --implicit-check-not="OpDecorate {{%[^ ]*_smooth_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_flat_override_[^ ]*}} Centroid" --implicit-check-not="OpDecorate {{%[^ ]*_smooth_[^ ]*}} NoPerspective"
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=mixed_vertex -o %t.mixed_vertex.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mixed_vertex.spv
// RUN: spirv-dis %t.mixed_vertex.spv | FileCheck %s --check-prefix=MIXED --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} NoPerspective" --implicit-check-not="OpDecorate {{%[^ ]*_plain_[^ ]*}} Flat"
// RUN: clang-spirv-link --entry=mixed_fragment -o %t.mixed_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mixed_fragment.spv
// RUN: spirv-dis %t.mixed_fragment.spv | FileCheck %s --check-prefix=MIXED --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} NoPerspective" --implicit-check-not="OpDecorate {{%[^ ]*_plain_[^ ]*}} Flat"
// RUN: clang-spirv-link --entry=inherited_fragment -o %t.inherited_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.inherited_fragment.spv
// RUN: spirv-dis %t.inherited_fragment.spv | FileCheck %s --check-prefix=INHERIT --implicit-check-not="OpDecorate {{%[^ ]*_smooth_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_flat_override_[^ ]*}} Centroid" --implicit-check-not="OpDecorate {{%[^ ]*_smooth_[^ ]*}} NoPerspective"
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O3 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=mixed_vertex -o %t.mixed_vertex.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mixed_vertex.spv
// RUN: spirv-dis %t.mixed_vertex.spv | FileCheck %s --check-prefix=MIXED --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} NoPerspective" --implicit-check-not="OpDecorate {{%[^ ]*_plain_[^ ]*}} Flat"
// RUN: clang-spirv-link --entry=mixed_fragment -o %t.mixed_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.mixed_fragment.spv
// RUN: spirv-dis %t.mixed_fragment.spv | FileCheck %s --check-prefix=MIXED --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_uv_[^ ]*}} NoPerspective" --implicit-check-not="OpDecorate {{%[^ ]*_plain_[^ ]*}} Flat"
// RUN: clang-spirv-link --entry=inherited_fragment -o %t.inherited_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.inherited_fragment.spv
// RUN: spirv-dis %t.inherited_fragment.spv | FileCheck %s --check-prefix=INHERIT --implicit-check-not="OpDecorate {{%[^ ]*_smooth_[^ ]*}} Flat" --implicit-check-not="OpDecorate {{%[^ ]*_flat_override_[^ ]*}} Centroid" --implicit-check-not="OpDecorate {{%[^ ]*_smooth_[^ ]*}} NoPerspective"

// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=double_fragment -o %t.double.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.double.spv
// RUN: spirv-dis %t.double.spv | FileCheck %s --check-prefix=DOUBLE
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=double_fragment -o %t.double.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.double.spv
// RUN: spirv-dis %t.double.spv | FileCheck %s --check-prefix=DOUBLE
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O3 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=double_fragment -o %t.double.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.double.spv
// RUN: spirv-dis %t.double.spv | FileCheck %s --check-prefix=DOUBLE

#include <spirv/types.hpp>
using namespace spirv;

// MIXED-DAG: OpDecorate [[ID:%[^ ]*_id_[^ ]*]] Location 0
// MIXED-DAG: OpDecorate [[ID]] Flat
// MIXED-DAG: OpDecorate [[UV:%[^ ]*_uv_[^ ]*]] Location 1
// MIXED-DAG: OpDecorate [[AFFINE:%[^ ]*_affine_[^ ]*]] Location 2
// MIXED-DAG: OpDecorate [[AFFINE]] NoPerspective
// MIXED-DAG: OpDecorate [[EDGE:%[^ ]*_edge_[^ ]*]] Location 3
// MIXED-DAG: OpDecorate [[EDGE]] Centroid
// MIXED-DAG: OpDecorate [[SUBSAMPLE:%[^ ]*_subsample_[^ ]*]] Location 4
// MIXED-DAG: OpDecorate [[SUBSAMPLE]] Sample
// MIXED-DAG: OpDecorate [[AFFINE_EDGE:%[^ ]*_affine_edge_[^ ]*]] Location 5
// MIXED-DAG: OpDecorate [[AFFINE_EDGE]] NoPerspective
// MIXED-DAG: OpDecorate [[AFFINE_EDGE]] Centroid
// MIXED-DAG: OpDecorate [[PLAIN:%[^ ]*_plain_[^ ]*]] Location 6
struct Mixed {
  unsigned id __location(0) __flat;
  f32x2 uv __location(1) __varying;
  f32x2 affine __location(2) __noperspective;
  float edge __location(3) __varying __centroid;
  float subsample __location(4) __varying __sample;
  float affine_edge __location(5) __noperspective __centroid;
  float plain __location(6);
};

__vertex
void mixed_vertex(__output Mixed *output, __position f32x4 *position) {
  output->id = vertex_idx;
  output->uv = f32x2(0.25f, 0.5f);
  output->affine = f32x2(0.25f, 0.5f);
  output->edge = 0.5f;
  output->subsample = 0.5f;
  output->affine_edge = 0.5f;
  output->plain = 0.5f;
  *position = f32x4(0, 0, 0, 1);
}

__fragment
void mixed_fragment(__input __varying Mixed input,
                    __output(0) float *color) {
  *color = float(input.id) + input.uv[0] + input.affine[0] + input.edge +
           input.subsample + input.affine_edge + input.plain;
}

// INHERIT-DAG: OpDecorate [[ID:%[^ ]*_id_[^ ]*]] Location 0
// INHERIT-DAG: OpDecorate [[ID]] Flat
// INHERIT-DAG: OpDecorate [[DEFAULT:%[^ ]*_default_flat_[^ ]*]] Location 1
// INHERIT-DAG: OpDecorate [[DEFAULT]] Flat
// INHERIT-DAG: OpDecorate [[SMOOTH:%[^ ]*_smooth_[^ ]*]] Location 2
// INHERIT-DAG: OpDecorate [[SAMPLED:%[^ ]*_sampled_[^ ]*]] Location 3
// INHERIT-DAG: OpDecorate [[SAMPLED]] Sample
// INHERIT-DAG: OpDecorate [[AFFINE:%[^ ]*_affine_[^ ]*]] Location 4
// INHERIT-DAG: OpDecorate [[AFFINE]] NoPerspective
// INHERIT-DAG: OpDecorate [[NESTED:%[^ ]*_inherited_[^ ]*]] Location 5
// INHERIT-DAG: OpDecorate [[NESTED]] NoPerspective
// INHERIT-DAG: OpDecorate [[NESTED]] Centroid
// INHERIT-DAG: OpDecorate [[PERSPECTIVE:%[^ ]*_perspective_override_[^ ]*]] Location 6
// INHERIT-DAG: OpDecorate [[PERSPECTIVE]] Centroid
// INHERIT-DAG: OpDecorate [[FLAT:%[^ ]*_flat_override_[^ ]*]] Location 7
// INHERIT-DAG: OpDecorate [[FLAT]] Flat
struct Nested {
  float inherited __location(5);
  float perspective_override __location(6) __varying;
  float flat_override __location(7) __flat;
};
struct Defaults {
  unsigned id __location(0);
  float default_flat __location(1);
  float smooth __location(2) __varying;
  float sampled __location(3) __varying __sample;
  float affine __location(4) __noperspective;
  Nested nested __noperspective __centroid;
};

// DOUBLE-DAG: OpDecorate [[DOUBLE:%[^ ]*_value_[^ ]*]] Location 0
// DOUBLE-DAG: OpDecorate [[DOUBLE]] Flat
// DOUBLE-DAG: OpTypeFloat 64
struct DoubleInput {
  double value __location(0) __flat;
};

__fragment
void double_fragment(__input DoubleInput input, __output(0) float *color) {
  *color = float(input.value);
}

__fragment
void inherited_fragment(__input __flat Defaults input,
                        __output(0) float *color) {
  *color = float(input.id) + input.default_flat + input.smooth + input.sampled +
           input.affine + input.nested.inherited +
           input.nested.perspective_override + input.nested.flat_override;
}
