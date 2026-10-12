// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 \
// RUN:   -fshader-host-math -I %S/../../lib/Headers -emit-llvm -o - %s \
// RUN:   | FileCheck %s

// CHECK-DAG: shl <2 x i32>
// CHECK-DAG: shl <3 x i32>
// CHECK-DAG: shl <4 x i32>
// CHECK-DAG: shl <2 x i64>
// CHECK-DAG: ashr <2 x i32>
// CHECK-DAG: or <2 x i1>

#include <spirv/types.hpp>
using namespace spirv;

u32x2 shift2(u32x2 first, u32x2 second) { return first << second; }
u32x3 shift3(u32x3 first, u32x3 second) { return first << second; }
u32x4 shift4(u32x4 first, u32x4 second) { return first << second; }
u64x2 shift64(u64x2 first, u64x2 second) { return first << second; }
i32x2 signed_shift(i32x2 first, i32x2 second) { return first >> second; }
b8x2 logical_or(b8x2 first, b8x2 second) { return first || second; }
