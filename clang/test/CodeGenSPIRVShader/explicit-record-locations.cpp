// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=explicit_vertex -o %t.explicit_vertex.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_vertex.spv
// RUN: spirv-dis %t.explicit_vertex.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"
// RUN: clang-spirv-link --entry=explicit_fragment -o %t.explicit_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_fragment.spv
// RUN: spirv-dis %t.explicit_fragment.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"
// RUN: clang-spirv-link --entry=explicit_control -o %t.explicit_control.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_control.spv
// RUN: spirv-dis %t.explicit_control.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=explicit_vertex -o %t.explicit_vertex.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_vertex.spv
// RUN: spirv-dis %t.explicit_vertex.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"
// RUN: clang-spirv-link --entry=explicit_fragment -o %t.explicit_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_fragment.spv
// RUN: spirv-dis %t.explicit_fragment.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"
// RUN: clang-spirv-link --entry=explicit_control -o %t.explicit_control.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_control.spv
// RUN: spirv-dis %t.explicit_control.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O3 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=explicit_vertex -o %t.explicit_vertex.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_vertex.spv
// RUN: spirv-dis %t.explicit_vertex.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"
// RUN: clang-spirv-link --entry=explicit_fragment -o %t.explicit_fragment.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_fragment.spv
// RUN: spirv-dis %t.explicit_fragment.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"
// RUN: clang-spirv-link --entry=explicit_control -o %t.explicit_control.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.explicit_control.spv
// RUN: spirv-dis %t.explicit_control.spv | FileCheck %s --implicit-check-not="Location 0" --implicit-check-not="Location 4"

// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O0 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=explicit_scalar_vertex -o %t.scalar.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.scalar.spv
// RUN: spirv-dis %t.scalar.spv | FileCheck %s --check-prefix=SCALAR
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O2 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=explicit_scalar_vertex -o %t.scalar.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.scalar.spv
// RUN: spirv-dis %t.scalar.spv | FileCheck %s --check-prefix=SCALAR
// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -O3 \
// RUN:   -I %S/../../lib/Headers -emit-llvm-bc -o %t.bc %s
// RUN: clang-spirv-link --entry=explicit_scalar_vertex -o %t.scalar.spv %t.bc
// RUN: spirv-val --target-env vulkan1.3 %t.scalar.spv
// RUN: spirv-dis %t.scalar.spv | FileCheck %s --check-prefix=SCALAR

#include <spirv/types.hpp>
using namespace spirv;

// CHECK-DAG: OpDecorate [[UV:%[^ ]*uv[^ ]*]] Location 3
// CHECK-DAG: OpDecorate [[NORMAL:%[^ ]*normal[^ ]*]] Location 7
// CHECK-DAG: OpTypeVector {{.*}} 2
// CHECK-DAG: OpTypeVector {{.*}} 3

// SCALAR-DAG: OpDecorate [[SCALAR_UV:%[^ ]*uv[^ ]*]] Location 3
// SCALAR-DAG: OpDecorate [[SCALAR_NORMAL:%[^ ]*normal[^ ]*]] Location 7
// SCALAR-DAG: [[FLOAT:%[^ ]+]] = OpTypeFloat 32
// SCALAR-NOT: OpTypeVector [[FLOAT]] 2
// SCALAR: OpFunction
// SCALAR-NOT: OpTypeVector [[FLOAT]] 2

struct Basis {
  f32x3 normal __location(7);
};
struct Outputs {
  f32x2 uv __location(3);
  Basis basis;
};
struct Inputs {
  Basis basis;
  f32x2 uv __location(3);
};

struct ScalarOutputs {
  float uv __location(3);
  float normal __location(7);
};

__vertex
void explicit_scalar_vertex(__output ScalarOutputs *output) {
  *output = ScalarOutputs{0.25f, 1.0f};
}

__vertex
void explicit_vertex(__output Outputs *output, __position f32x4 *position) {
  output->uv = f32x2(0.25f, 0.5f);
  output->basis.normal = f32x3(0, 0, 1);
  *position = f32x4(0, 0, 0, 1);
}

__fragment
void explicit_fragment(__input Inputs input, __output(1) float *color) {
  *color = input.uv[0] + input.basis.normal[2];
}

__tessellation_control
__output_vertices(3)
void explicit_control(__input const Inputs (*input)[3],
                      __output Outputs (*output)[3]) {
  (*output)[invocation_id].uv =
      f32x2((*input)[invocation_id].uv[0], (*input)[invocation_id].uv[1]);
  (*output)[invocation_id].basis.normal =
      f32x3((*input)[invocation_id].basis.normal[0],
            (*input)[invocation_id].basis.normal[1],
            (*input)[invocation_id].basis.normal[2]);
}
