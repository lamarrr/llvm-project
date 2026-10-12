// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 \
// RUN:   -fshader-host-math -I %S/../../lib/Headers -fsyntax-only -verify %s
// expected-no-diagnostics
#include <spirv/vec.hpp>
#include <spirv/mat.hpp>

spirv::vec<float, 2> vector_math(spirv::vec<float, 2> first,
                                 spirv::vec<float, 2> second) {
  return mix(first, second, 0.5f);
}

spirv::mat<2, 2> matrix_math(spirv::mat<2, 2> first,
                             spirv::mat<2, 2> second) {
  return matrix_comp_mult(first, second);
}
