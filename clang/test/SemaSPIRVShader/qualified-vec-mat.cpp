// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 \
// RUN:   -fspirv-shader -I %S/../../lib/Headers -fsyntax-only -verify %s
// expected-no-diagnostics

#include <spirv/types.hpp>

using namespace spirv;

static_assert(__is_same(decltype(((__uniform const f32x4 *)nullptr)->x()),
                        const __uniform float &));
static_assert(__is_same(decltype(((__push_constant const f32x4 *)nullptr)->y()),
                        const __push_constant float &));
static_assert(__is_same(decltype(((__storage f32x4 *)nullptr)->z()),
                        __storage float &));
static_assert(__is_same(decltype((*(__uniform const f32x4 *)nullptr)[0]),
                        const __uniform float &));
static_assert(__is_same(decltype((*(__uniform const f32x4x4 *)nullptr)[0][0]),
                        const __uniform float &));
static_assert(
    __is_same(decltype((*(__push_constant const f32x4x4 *)nullptr)[0][0]),
              const __push_constant float &));
static_assert(__is_same(decltype((*(__storage const f32x4x4 *)nullptr)[1][2]),
                        const __storage float &));

float qualified_read(__uniform const f32x4 *uniform,
                     __push_constant const f32x4 *push,
                     __uniform const f32x4x4 *matrix) {
  auto pair = uniform->yx();
  auto triple = push->zyx();
  auto quadruple = uniform->wzyx();
  auto selected = uniform->swizzle<3, 1, 0>();
  return pair.x() + triple.y() + quadruple.z() + selected.z() + uniform->x() +
         push->w() + (*matrix)[1][2] + uniform->length();
}

void qualified_write(__storage f32x4 *storage) {
  storage->x() = 1.0f;
  (*storage)[1] = 2.0f;
  storage->set_swizzle<2, 3>(f32x2(3.0f, 4.0f));
}
