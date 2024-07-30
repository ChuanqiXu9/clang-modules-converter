; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py UTC_ARGS: --version 5
; RUN: llc < %s -wasm-keep-registers | FileCheck %s

target triple = "wasm32-unknown-unknown"

define i8 @scmp.8.8(i8 signext %x, i8 signext %y) nounwind {
; CHECK-LABEL: scmp.8.8:
; CHECK:         .functype scmp.8.8 (i32, i32) -> (i32)
; CHECK-NEXT:  # %bb.0:
; CHECK-NEXT:    local.get $push4=, 0
; CHECK-NEXT:    local.get $push3=, 1
; CHECK-NEXT:    i32.gt_s $push1=, $pop4, $pop3
; CHECK-NEXT:    local.get $push6=, 0
; CHECK-NEXT:    local.get $push5=, 1
; CHECK-NEXT:    i32.lt_s $push0=, $pop6, $pop5
; CHECK-NEXT:    i32.sub $push2=, $pop1, $pop0
; CHECK-NEXT:    # fallthrough-return
  %1 = call i8 @llvm.scmp(i8 %x, i8 %y)
  ret i8 %1
}

define i8 @scmp.8.16(i16 signext %x, i16 signext %y) nounwind {
; CHECK-LABEL: scmp.8.16:
; CHECK:         .functype scmp.8.16 (i32, i32) -> (i32)
; CHECK-NEXT:  # %bb.0:
; CHECK-NEXT:    local.get $push4=, 0
; CHECK-NEXT:    local.get $push3=, 1
; CHECK-NEXT:    i32.gt_s $push1=, $pop4, $pop3
; CHECK-NEXT:    local.get $push6=, 0
; CHECK-NEXT:    local.get $push5=, 1
; CHECK-NEXT:    i32.lt_s $push0=, $pop6, $pop5
; CHECK-NEXT:    i32.sub $push2=, $pop1, $pop0
; CHECK-NEXT:    # fallthrough-return
  %1 = call i8 @llvm.scmp(i16 %x, i16 %y)
  ret i8 %1
}

define i8 @scmp.8.32(i32 %x, i32 %y) nounwind {
; CHECK-LABEL: scmp.8.32:
; CHECK:         .functype scmp.8.32 (i32, i32) -> (i32)
; CHECK-NEXT:  # %bb.0:
; CHECK-NEXT:    local.get $push4=, 0
; CHECK-NEXT:    local.get $push3=, 1
; CHECK-NEXT:    i32.gt_s $push1=, $pop4, $pop3
; CHECK-NEXT:    local.get $push6=, 0
; CHECK-NEXT:    local.get $push5=, 1
; CHECK-NEXT:    i32.lt_s $push0=, $pop6, $pop5
; CHECK-NEXT:    i32.sub $push2=, $pop1, $pop0
; CHECK-NEXT:    # fallthrough-return
  %1 = call i8 @llvm.scmp(i32 %x, i32 %y)
  ret i8 %1
}

define i8 @scmp.8.64(i64 %x, i64 %y) nounwind {
; CHECK-LABEL: scmp.8.64:
; CHECK:         .functype scmp.8.64 (i64, i64) -> (i32)
; CHECK-NEXT:  # %bb.0:
; CHECK-NEXT:    local.get $push4=, 0
; CHECK-NEXT:    local.get $push3=, 1
; CHECK-NEXT:    i64.gt_s $push1=, $pop4, $pop3
; CHECK-NEXT:    local.get $push6=, 0
; CHECK-NEXT:    local.get $push5=, 1
; CHECK-NEXT:    i64.lt_s $push0=, $pop6, $pop5
; CHECK-NEXT:    i32.sub $push2=, $pop1, $pop0
; CHECK-NEXT:    # fallthrough-return
  %1 = call i8 @llvm.scmp(i64 %x, i64 %y)
  ret i8 %1
}

define i8 @scmp.8.128(i128 %x, i128 %y) nounwind {
; CHECK-LABEL: scmp.8.128:
; CHECK:         .functype scmp.8.128 (i64, i64, i64, i64) -> (i32)
; CHECK-NEXT:    .local i32
; CHECK-NEXT:  # %bb.0:
; CHECK-NEXT:    local.get $push10=, 0
; CHECK-NEXT:    local.get $push9=, 2
; CHECK-NEXT:    i64.gt_u $push4=, $pop10, $pop9
; CHECK-NEXT:    local.get $push12=, 1
; CHECK-NEXT:    local.get $push11=, 3
; CHECK-NEXT:    i64.gt_s $push3=, $pop12, $pop11
; CHECK-NEXT:    local.get $push14=, 1
; CHECK-NEXT:    local.get $push13=, 3
; CHECK-NEXT:    i64.eq $push8=, $pop14, $pop13
; CHECK-NEXT:    local.tee $push7=, 4, $pop8
; CHECK-NEXT:    i32.select $push5=, $pop4, $pop3, $pop7
; CHECK-NEXT:    local.get $push16=, 0
; CHECK-NEXT:    local.get $push15=, 2
; CHECK-NEXT:    i64.lt_u $push1=, $pop16, $pop15
; CHECK-NEXT:    local.get $push18=, 1
; CHECK-NEXT:    local.get $push17=, 3
; CHECK-NEXT:    i64.lt_s $push0=, $pop18, $pop17
; CHECK-NEXT:    local.get $push19=, 4
; CHECK-NEXT:    i32.select $push2=, $pop1, $pop0, $pop19
; CHECK-NEXT:    i32.sub $push6=, $pop5, $pop2
; CHECK-NEXT:    # fallthrough-return
  %1 = call i8 @llvm.scmp(i128 %x, i128 %y)
  ret i8 %1
}

define i32 @scmp.32.32(i32 %x, i32 %y) nounwind {
; CHECK-LABEL: scmp.32.32:
; CHECK:         .functype scmp.32.32 (i32, i32) -> (i32)
; CHECK-NEXT:  # %bb.0:
; CHECK-NEXT:    local.get $push4=, 0
; CHECK-NEXT:    local.get $push3=, 1
; CHECK-NEXT:    i32.gt_s $push1=, $pop4, $pop3
; CHECK-NEXT:    local.get $push6=, 0
; CHECK-NEXT:    local.get $push5=, 1
; CHECK-NEXT:    i32.lt_s $push0=, $pop6, $pop5
; CHECK-NEXT:    i32.sub $push2=, $pop1, $pop0
; CHECK-NEXT:    # fallthrough-return
  %1 = call i32 @llvm.scmp(i32 %x, i32 %y)
  ret i32 %1
}

define i32 @scmp.32.64(i64 %x, i64 %y) nounwind {
; CHECK-LABEL: scmp.32.64:
; CHECK:         .functype scmp.32.64 (i64, i64) -> (i32)
; CHECK-NEXT:  # %bb.0:
; CHECK-NEXT:    local.get $push4=, 0
; CHECK-NEXT:    local.get $push3=, 1
; CHECK-NEXT:    i64.gt_s $push1=, $pop4, $pop3
; CHECK-NEXT:    local.get $push6=, 0
; CHECK-NEXT:    local.get $push5=, 1
; CHECK-NEXT:    i64.lt_s $push0=, $pop6, $pop5
; CHECK-NEXT:    i32.sub $push2=, $pop1, $pop0
; CHECK-NEXT:    # fallthrough-return
  %1 = call i32 @llvm.scmp(i64 %x, i64 %y)
  ret i32 %1
}

define i64 @scmp.64.64(i64 %x, i64 %y) nounwind {
; CHECK-LABEL: scmp.64.64:
; CHECK:         .functype scmp.64.64 (i64, i64) -> (i64)
; CHECK-NEXT:  # %bb.0:
; CHECK-NEXT:    local.get $push5=, 0
; CHECK-NEXT:    local.get $push4=, 1
; CHECK-NEXT:    i64.gt_s $push1=, $pop5, $pop4
; CHECK-NEXT:    local.get $push7=, 0
; CHECK-NEXT:    local.get $push6=, 1
; CHECK-NEXT:    i64.lt_s $push0=, $pop7, $pop6
; CHECK-NEXT:    i32.sub $push2=, $pop1, $pop0
; CHECK-NEXT:    i64.extend_i32_s $push3=, $pop2
; CHECK-NEXT:    # fallthrough-return
  %1 = call i64 @llvm.scmp(i64 %x, i64 %y)
  ret i64 %1
}