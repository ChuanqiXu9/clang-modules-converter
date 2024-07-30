; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --version 5
; RUN: opt -S --passes=slp-vectorizer -mtriple=s390x-unknown-linux-gnu -mcpu=z16 -slp-threshold=-10 < %s | FileCheck %s

define i1 @test(i64 %0, i64 %1, ptr %2) {
; CHECK-LABEL: define i1 @test(
; CHECK-SAME: i64 [[TMP0:%.*]], i64 [[TMP1:%.*]], ptr [[TMP2:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  [[ENTRY:.*:]]
; CHECK-NEXT:    [[GEP44:%.*]] = getelementptr i8, ptr null, i64 [[TMP0]]
; CHECK-NEXT:    [[GEP45:%.*]] = getelementptr i8, ptr null, i64 [[TMP1]]
; CHECK-NEXT:    [[GEP48:%.*]] = getelementptr i8, ptr null, i64 [[TMP0]]
; CHECK-NEXT:    [[GEP49:%.*]] = getelementptr i8, ptr null, i64 [[TMP1]]
; CHECK-NEXT:    [[TMP3:%.*]] = insertelement <2 x ptr> poison, ptr [[GEP44]], i32 0
; CHECK-NEXT:    [[TMP4:%.*]] = insertelement <2 x ptr> [[TMP3]], ptr [[GEP48]], i32 1
; CHECK-NEXT:    [[TMP5:%.*]] = insertelement <2 x ptr> poison, ptr [[GEP45]], i32 0
; CHECK-NEXT:    [[TMP6:%.*]] = insertelement <2 x ptr> [[TMP5]], ptr [[GEP49]], i32 1
; CHECK-NEXT:    [[TMP7:%.*]] = icmp ult <2 x ptr> [[TMP4]], [[TMP6]]
; CHECK-NEXT:    [[TMP8:%.*]] = select <2 x i1> [[TMP7]], <2 x ptr> [[TMP4]], <2 x ptr> [[TMP6]]
; CHECK-NEXT:    [[TMP9:%.*]] = insertelement <2 x ptr> poison, ptr [[TMP2]], i32 0
; CHECK-NEXT:    [[TMP10:%.*]] = shufflevector <2 x ptr> [[TMP9]], <2 x ptr> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP11:%.*]] = icmp ult <2 x ptr> [[TMP8]], [[TMP10]]
; CHECK-NEXT:    [[TMP12:%.*]] = extractelement <2 x i1> [[TMP11]], i32 0
; CHECK-NEXT:    [[TMP13:%.*]] = extractelement <2 x i1> [[TMP11]], i32 1
; CHECK-NEXT:    [[RES:%.*]] = and i1 [[TMP12]], [[TMP13]]
; CHECK-NEXT:    ret i1 [[RES]]
;
entry:
  %gep44 = getelementptr i8, ptr null, i64 %0
  %gep45 = getelementptr i8, ptr null, i64 %1
  %4 = icmp ult ptr %gep44, %gep45
  %umin = select i1 %4, ptr %gep44, ptr %gep45
  %gep48 = getelementptr i8, ptr null, i64 %0
  %gep49 = getelementptr i8, ptr null, i64 %1
  %5 = icmp ult ptr %gep48, %gep49
  %umin50 = select i1 %5, ptr %gep48, ptr %gep49
  %b095 = icmp ult ptr %umin, %2
  %b196 = icmp ult ptr %umin50, %2
  %res = and i1 %b095, %b196
  ret i1 %res
}