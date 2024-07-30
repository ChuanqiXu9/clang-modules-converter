; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown -mattr=+sse2,+mmx | FileCheck %s

; Originally from PR2687, but things don't work that way any more.
; there are no MMX instructions here; we use XMM.

define <2 x double> @a(<2 x i32> %x) nounwind {
; CHECK-LABEL: a:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cvtdq2pd %xmm0, %xmm0
; CHECK-NEXT:    retl
entry:
  %y = sitofp <2 x i32> %x to <2 x double>
  ret <2 x double> %y
}

define <2 x i32> @b(<2 x double> %x) nounwind {
; CHECK-LABEL: b:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cvttpd2dq %xmm0, %xmm0
; CHECK-NEXT:    retl
entry:
  %y = fptosi <2 x double> %x to <2 x i32>
  ret <2 x i32> %y
}

; This is how to get MMX instructions.

define <2 x double> @a2(<1 x i64> %x) nounwind {
; CHECK-LABEL: a2:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    pushl %ebp
; CHECK-NEXT:    movl %esp, %ebp
; CHECK-NEXT:    andl $-8, %esp
; CHECK-NEXT:    subl $8, %esp
; CHECK-NEXT:    movl 8(%ebp), %eax
; CHECK-NEXT:    movl 12(%ebp), %ecx
; CHECK-NEXT:    movl %ecx, {{[0-9]+}}(%esp)
; CHECK-NEXT:    movl %eax, (%esp)
; CHECK-NEXT:    cvtpi2pd (%esp), %xmm0
; CHECK-NEXT:    movl %ebp, %esp
; CHECK-NEXT:    popl %ebp
; CHECK-NEXT:    retl
entry:
  %y = tail call <2 x double> @llvm.x86.sse.cvtpi2pd(<1 x i64> %x)
  ret <2 x double> %y
}

define <1 x i64> @b2(<2 x double> %x) nounwind {
; CHECK-LABEL: b2:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    pushl %ebp
; CHECK-NEXT:    movl %esp, %ebp
; CHECK-NEXT:    andl $-8, %esp
; CHECK-NEXT:    subl $8, %esp
; CHECK-NEXT:    cvttpd2pi %xmm0, %mm0
; CHECK-NEXT:    movq %mm0, (%esp)
; CHECK-NEXT:    movl (%esp), %eax
; CHECK-NEXT:    movl {{[0-9]+}}(%esp), %edx
; CHECK-NEXT:    movl %ebp, %esp
; CHECK-NEXT:    popl %ebp
; CHECK-NEXT:    retl
entry:
  %y = tail call <1 x i64> @llvm.x86.sse.cvttpd2pi (<2 x double> %x)
  ret <1 x i64> %y
}

declare <2 x double> @llvm.x86.sse.cvtpi2pd(<1 x i64>)
declare <1 x i64> @llvm.x86.sse.cvttpd2pi(<2 x double>)