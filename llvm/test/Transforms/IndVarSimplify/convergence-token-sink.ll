; RUN: opt -passes='loop(indvars)' -verify-each -S %s | FileCheck %s

; A convergence.entry token used only after a loop must stay in the function
; entry block. Sinking loop-invariant instructions to the exit would otherwise
; move it into %exit and violate the convergence verifier.

declare token @llvm.experimental.convergence.entry() convergent
declare void @use() convergent

define void @keep_entry_token(i32 %limit) convergent {
; CHECK-LABEL: define void @keep_entry_token(
; CHECK: entry:
; CHECK-NEXT: %tok = call token @llvm.experimental.convergence.entry()
; CHECK: exit:
; CHECK-NOT: call token @llvm.experimental.convergence.entry()
; CHECK: call void @use() [ "convergencectrl"(token %tok) ]
entry:
  %tok = call token @llvm.experimental.convergence.entry()
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %next, %body ]
  %more = icmp ult i32 %i, %limit
  br i1 %more, label %body, label %exit

body:
  %next = add nuw i32 %i, 1
  br label %loop

exit:
  call void @use() [ "convergencectrl"(token %tok) ]
  ret void
}
