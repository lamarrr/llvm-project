; RUN: llvm-link -S %s %p/Inputs/target-ext-remap.ll -o - | FileCheck %s
;
; A named pointee can have a different body in the linked module. Remap the
; target extension type's type parameters along with the containing struct.

%record = type { i32 }
@anchor = global %record zeroinitializer

; CHECK: %record = type { i32 }
; CHECK: %params = type { target("spirv.$TypedPointerType", %record.0, 14) }
; CHECK: %record.0 = type { [2 x float] }
; CHECK: @g = addrspace(13) global %params zeroinitializer
