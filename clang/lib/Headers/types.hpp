//===-- types.hpp - Scalar, vector, and matrix aliases for Shader C++ ----===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "mat.hpp"

using u8 = __UINT8_TYPE__;
using u16 = __UINT16_TYPE__;
using u32 = __UINT32_TYPE__;
using u64 = __UINT64_TYPE__;
using i8 = __INT8_TYPE__;
using i16 = __INT16_TYPE__;
using i32 = __INT32_TYPE__;
using i64 = __INT64_TYPE__;
using f16 = _Float16;
using f32 = float;
using f64 = double;

using u8x2 = vec<u8, 2>;
using u8x3 = vec<u8, 3>;
using u8x4 = vec<u8, 4>;
using u16x2 = vec<u16, 2>;
using u16x3 = vec<u16, 3>;
using u16x4 = vec<u16, 4>;
using u32x2 = vec<u32, 2>;
using u32x3 = vec<u32, 3>;
using u32x4 = vec<u32, 4>;
using u64x2 = vec<u64, 2>;
using u64x3 = vec<u64, 3>;
using u64x4 = vec<u64, 4>;
using i8x2 = vec<i8, 2>;
using i8x3 = vec<i8, 3>;
using i8x4 = vec<i8, 4>;
using i16x2 = vec<i16, 2>;
using i16x3 = vec<i16, 3>;
using i16x4 = vec<i16, 4>;
using i32x2 = vec<i32, 2>;
using i32x3 = vec<i32, 3>;
using i32x4 = vec<i32, 4>;
using i64x2 = vec<i64, 2>;
using i64x3 = vec<i64, 3>;
using i64x4 = vec<i64, 4>;
using f16x2 = vec<f16, 2>;
using f16x3 = vec<f16, 3>;
using f16x4 = vec<f16, 4>;
using f32x2 = vec<f32, 2>;
using f32x3 = vec<f32, 3>;
using f32x4 = vec<f32, 4>;
using f64x2 = vec<f64, 2>;
using f64x3 = vec<f64, 3>;
using f64x4 = vec<f64, 4>;
using boolx2 = vec<bool, 2>;
using boolx3 = vec<bool, 3>;
using boolx4 = vec<bool, 4>;

using f32x2x2 = mat<2, 2>;
using f32x2x3 = mat<2, 3>;
using f32x2x4 = mat<2, 4>;
using f32x3x2 = mat<3, 2>;
using f32x3x3 = mat<3, 3>;
using f32x3x4 = mat<3, 4>;
using f32x4x2 = mat<4, 2>;
using f32x4x3 = mat<4, 3>;
using f32x4x4 = mat<4, 4>;
