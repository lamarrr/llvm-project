; RUN: opt -passes=simplifycfg -verify-each -S %s | FileCheck %s
; RUN: opt -passes=jump-threading -verify-each -S %s | FileCheck %s
; RUN: opt -passes=always-inline -verify-each -S %s | FileCheck %s --check-prefix=INLINE
;
; Peeling/unrolling may leave a used convergence.loop in an acyclic block.
; Merging its predecessor must not put a convergent call before the token.

declare token @llvm.experimental.convergence.entry()
declare token @llvm.experimental.convergence.loop()
declare void @barrier() convergent
declare void @ordinary()

define void @barrier_before_loop(i1 %condition) convergent alwaysinline {
; CHECK-LABEL: define void @barrier_before_loop(
; CHECK: preheader:
; CHECK: call void @barrier() [ "convergencectrl"(token %entry.token) ]
; CHECK-NEXT: br label %header
; CHECK: header:
; CHECK: %loop.token = call token @llvm.experimental.convergence.loop()
; CHECK-NEXT: call void @barrier() [ "convergencectrl"(token %loop.token) ]
entry:
  %entry.token = call token @llvm.experimental.convergence.entry()
  br i1 %condition, label %preheader, label %exit

preheader:
  call void @barrier() [ "convergencectrl"(token %entry.token) ]
  br label %header

header:
  %loop.token = call token @llvm.experimental.convergence.loop() [ "convergencectrl"(token %entry.token) ]
  call void @barrier() [ "convergencectrl"(token %loop.token) ]
  ret void

exit:
  ret void
}

define void @caller(i1 %condition) convergent {
; INLINE-LABEL: define void @caller(
; INLINE: preheader.i:
; INLINE: call void @barrier() [ "convergencectrl"(token %token) ]
; INLINE-NEXT: br label %header.i
; INLINE: header.i:
; INLINE: [[LOOP:%.*]] = call token @llvm.experimental.convergence.loop()
; INLINE-NEXT: call void @barrier() [ "convergencectrl"(token [[LOOP]]) ]
entry:
  %token = call token @llvm.experimental.convergence.entry()
  call void @barrier_before_loop(i1 %condition) [ "convergencectrl"(token %token) ]
  ret void
}

; An ordinary call does not need a new convergence boundary.
define void @ordinary_before_loop(i1 %condition) convergent {
; CHECK-LABEL: define void @ordinary_before_loop(
; CHECK: call void @ordinary()
; CHECK-NEXT: %loop.token = call token @llvm.experimental.convergence.loop()
; CHECK-NEXT: call void @barrier() [ "convergencectrl"(token %loop.token) ]
entry:
  %entry.token = call token @llvm.experimental.convergence.entry()
  br i1 %condition, label %preheader, label %exit

preheader:
  call void @ordinary()
  br label %header

header:
  %loop.token = call token @llvm.experimental.convergence.loop() [ "convergencectrl"(token %entry.token) ]
  call void @barrier() [ "convergencectrl"(token %loop.token) ]
  ret void

exit:
  ret void
}
