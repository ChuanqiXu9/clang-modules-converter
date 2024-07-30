; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --version 4
; RUN: opt -passes=slp-vectorizer -S -mtriple=x86_64-unknown-linux -mcpu=skylake < %s | FileCheck %s

define i1 @test(i1 %cmp5.not.31) {
; CHECK-LABEL: define i1 @test(
; CHECK-SAME: i1 [[CMP5_NOT_31:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = insertelement <4 x i1> <i1 poison, i1 false, i1 false, i1 false>, i1 [[CMP5_NOT_31]], i32 0
; CHECK-NEXT:    [[TMP1:%.*]] = select <4 x i1> [[TMP0]], <4 x i32> zeroinitializer, <4 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP2:%.*]] = mul <4 x i32> [[TMP1]], <i32 2, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[TMP3:%.*]] = call i32 @llvm.vector.reduce.add.v4i32(<4 x i32> [[TMP2]])
; CHECK-NEXT:    [[TMP4:%.*]] = and i32 [[TMP3]], 0
; CHECK-NEXT:    [[CMP_NOT_I_I:%.*]] = icmp eq i32 [[TMP4]], 0
; CHECK-NEXT:    ret i1 [[CMP_NOT_I_I]]
;
entry:
  %add7.31 = select i1 %cmp5.not.31, i32 0, i32 0
  %add18 = select i1 false, i32 0, i32 0
  %add19 = add i32 %add18, %add7.31
  %add18.1 = select i1 false, i32 0, i32 0
  %add19.1 = add i32 %add18.1, %add19
  %add18.4 = select i1 false, i32 0, i32 0
  %add19.4 = add i32 %add18.4, %add19.1
  %add19.31 = add i32 %add7.31, %add19.4
  %0 = and i32 %add19.31, 0
  %cmp.not.i.i = icmp eq i32 %0, 0
  ret i1 %cmp.not.i.i
}