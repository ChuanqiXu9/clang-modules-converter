; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --version 5
; RUN: opt -passes=loop-distribute -verify-loop-info -verify-dom-info -S < %s | FileCheck %s
;
; Check that followup loop-attributes are applied to the loops after
; loop distribution.
;

define void @f(ptr %a, ptr %b, ptr %c, ptr %d, ptr %e) {
; CHECK-LABEL: define void @f(
; CHECK-SAME: ptr [[A:%.*]], ptr [[B:%.*]], ptr [[C:%.*]], ptr [[D:%.*]], ptr [[E:%.*]]) {
; CHECK-NEXT:  [[ENTRY:.*:]]
; CHECK-NEXT:    br label %[[FOR_BODY_LVER_CHECK:.*]]
; CHECK:       [[FOR_BODY_LVER_CHECK]]:
; CHECK-NEXT:    [[SCEVGEP:%.*]] = getelementptr i8, ptr [[A]], i64 84
; CHECK-NEXT:    [[SCEVGEP1:%.*]] = getelementptr i8, ptr [[C]], i64 80
; CHECK-NEXT:    [[SCEVGEP2:%.*]] = getelementptr i8, ptr [[D]], i64 80
; CHECK-NEXT:    [[SCEVGEP3:%.*]] = getelementptr i8, ptr [[E]], i64 80
; CHECK-NEXT:    [[SCEVGEP4:%.*]] = getelementptr i8, ptr [[B]], i64 80
; CHECK-NEXT:    [[BOUND0:%.*]] = icmp ult ptr [[A]], [[SCEVGEP1]]
; CHECK-NEXT:    [[BOUND1:%.*]] = icmp ult ptr [[C]], [[SCEVGEP]]
; CHECK-NEXT:    [[FOUND_CONFLICT:%.*]] = and i1 [[BOUND0]], [[BOUND1]]
; CHECK-NEXT:    [[BOUND05:%.*]] = icmp ult ptr [[A]], [[SCEVGEP2]]
; CHECK-NEXT:    [[BOUND16:%.*]] = icmp ult ptr [[D]], [[SCEVGEP]]
; CHECK-NEXT:    [[FOUND_CONFLICT7:%.*]] = and i1 [[BOUND05]], [[BOUND16]]
; CHECK-NEXT:    [[CONFLICT_RDX:%.*]] = or i1 [[FOUND_CONFLICT]], [[FOUND_CONFLICT7]]
; CHECK-NEXT:    [[BOUND08:%.*]] = icmp ult ptr [[A]], [[SCEVGEP3]]
; CHECK-NEXT:    [[BOUND19:%.*]] = icmp ult ptr [[E]], [[SCEVGEP]]
; CHECK-NEXT:    [[FOUND_CONFLICT10:%.*]] = and i1 [[BOUND08]], [[BOUND19]]
; CHECK-NEXT:    [[CONFLICT_RDX11:%.*]] = or i1 [[CONFLICT_RDX]], [[FOUND_CONFLICT10]]
; CHECK-NEXT:    [[BOUND012:%.*]] = icmp ult ptr [[C]], [[SCEVGEP4]]
; CHECK-NEXT:    [[BOUND113:%.*]] = icmp ult ptr [[B]], [[SCEVGEP1]]
; CHECK-NEXT:    [[FOUND_CONFLICT14:%.*]] = and i1 [[BOUND012]], [[BOUND113]]
; CHECK-NEXT:    [[CONFLICT_RDX15:%.*]] = or i1 [[CONFLICT_RDX11]], [[FOUND_CONFLICT14]]
; CHECK-NEXT:    br i1 [[CONFLICT_RDX15]], label %[[FOR_BODY_PH_LVER_ORIG:.*]], label %[[FOR_BODY_PH_LDIST1:.*]]
; CHECK:       [[FOR_BODY_PH_LVER_ORIG]]:
; CHECK-NEXT:    br label %[[FOR_BODY_LVER_ORIG:.*]]
; CHECK:       [[FOR_BODY_LVER_ORIG]]:
; CHECK-NEXT:    [[IND_LVER_ORIG:%.*]] = phi i64 [ 0, %[[FOR_BODY_PH_LVER_ORIG]] ], [ [[ADD_LVER_ORIG:%.*]], %[[FOR_BODY_LVER_ORIG]] ]
; CHECK-NEXT:    [[ARRAYIDXA_LVER_ORIG:%.*]] = getelementptr inbounds i32, ptr [[A]], i64 [[IND_LVER_ORIG]]
; CHECK-NEXT:    [[LOADA_LVER_ORIG:%.*]] = load i32, ptr [[ARRAYIDXA_LVER_ORIG]], align 4
; CHECK-NEXT:    [[ARRAYIDXB_LVER_ORIG:%.*]] = getelementptr inbounds i32, ptr [[B]], i64 [[IND_LVER_ORIG]]
; CHECK-NEXT:    [[LOADB_LVER_ORIG:%.*]] = load i32, ptr [[ARRAYIDXB_LVER_ORIG]], align 4
; CHECK-NEXT:    [[MULA_LVER_ORIG:%.*]] = mul i32 [[LOADB_LVER_ORIG]], [[LOADA_LVER_ORIG]]
; CHECK-NEXT:    [[ADD_LVER_ORIG]] = add nuw nsw i64 [[IND_LVER_ORIG]], 1
; CHECK-NEXT:    [[ARRAYIDXA_PLUS_4_LVER_ORIG:%.*]] = getelementptr inbounds i32, ptr [[A]], i64 [[ADD_LVER_ORIG]]
; CHECK-NEXT:    store i32 [[MULA_LVER_ORIG]], ptr [[ARRAYIDXA_PLUS_4_LVER_ORIG]], align 4
; CHECK-NEXT:    [[ARRAYIDXD_LVER_ORIG:%.*]] = getelementptr inbounds i32, ptr [[D]], i64 [[IND_LVER_ORIG]]
; CHECK-NEXT:    [[LOADD_LVER_ORIG:%.*]] = load i32, ptr [[ARRAYIDXD_LVER_ORIG]], align 4
; CHECK-NEXT:    [[ARRAYIDXE_LVER_ORIG:%.*]] = getelementptr inbounds i32, ptr [[E]], i64 [[IND_LVER_ORIG]]
; CHECK-NEXT:    [[LOADE_LVER_ORIG:%.*]] = load i32, ptr [[ARRAYIDXE_LVER_ORIG]], align 4
; CHECK-NEXT:    [[MULC_LVER_ORIG:%.*]] = mul i32 [[LOADD_LVER_ORIG]], [[LOADE_LVER_ORIG]]
; CHECK-NEXT:    [[ARRAYIDXC_LVER_ORIG:%.*]] = getelementptr inbounds i32, ptr [[C]], i64 [[IND_LVER_ORIG]]
; CHECK-NEXT:    store i32 [[MULC_LVER_ORIG]], ptr [[ARRAYIDXC_LVER_ORIG]], align 4
; CHECK-NEXT:    [[EXITCOND_LVER_ORIG:%.*]] = icmp eq i64 [[ADD_LVER_ORIG]], 20
; CHECK-NEXT:    br i1 [[EXITCOND_LVER_ORIG]], label %[[FOR_END_LOOPEXIT:.*]], label %[[FOR_BODY_LVER_ORIG]], !llvm.loop [[LOOP0:![0-9]+]]
; CHECK:       [[FOR_BODY_PH_LDIST1]]:
; CHECK-NEXT:    br label %[[FOR_BODY_LDIST1:.*]]
; CHECK:       [[FOR_BODY_LDIST1]]:
; CHECK-NEXT:    [[IND_LDIST1:%.*]] = phi i64 [ 0, %[[FOR_BODY_PH_LDIST1]] ], [ [[ADD_LDIST1:%.*]], %[[FOR_BODY_LDIST1]] ]
; CHECK-NEXT:    [[ARRAYIDXA_LDIST1:%.*]] = getelementptr inbounds i32, ptr [[A]], i64 [[IND_LDIST1]]
; CHECK-NEXT:    [[LOADA_LDIST1:%.*]] = load i32, ptr [[ARRAYIDXA_LDIST1]], align 4, !alias.scope [[META3:![0-9]+]], !noalias [[META6:![0-9]+]]
; CHECK-NEXT:    [[ARRAYIDXB_LDIST1:%.*]] = getelementptr inbounds i32, ptr [[B]], i64 [[IND_LDIST1]]
; CHECK-NEXT:    [[LOADB_LDIST1:%.*]] = load i32, ptr [[ARRAYIDXB_LDIST1]], align 4, !alias.scope [[META10:![0-9]+]]
; CHECK-NEXT:    [[MULA_LDIST1:%.*]] = mul i32 [[LOADB_LDIST1]], [[LOADA_LDIST1]]
; CHECK-NEXT:    [[ADD_LDIST1]] = add nuw nsw i64 [[IND_LDIST1]], 1
; CHECK-NEXT:    [[ARRAYIDXA_PLUS_4_LDIST1:%.*]] = getelementptr inbounds i32, ptr [[A]], i64 [[ADD_LDIST1]]
; CHECK-NEXT:    store i32 [[MULA_LDIST1]], ptr [[ARRAYIDXA_PLUS_4_LDIST1]], align 4, !alias.scope [[META3]], !noalias [[META6]]
; CHECK-NEXT:    [[EXITCOND_LDIST1:%.*]] = icmp eq i64 [[ADD_LDIST1]], 20
; CHECK-NEXT:    br i1 [[EXITCOND_LDIST1]], label %[[FOR_BODY_PH:.*]], label %[[FOR_BODY_LDIST1]], !llvm.loop [[LOOP12:![0-9]+]]
; CHECK:       [[FOR_BODY_PH]]:
; CHECK-NEXT:    br label %[[FOR_BODY:.*]]
; CHECK:       [[FOR_BODY]]:
; CHECK-NEXT:    [[IND:%.*]] = phi i64 [ 0, %[[FOR_BODY_PH]] ], [ [[ADD:%.*]], %[[FOR_BODY]] ]
; CHECK-NEXT:    [[ADD]] = add nuw nsw i64 [[IND]], 1
; CHECK-NEXT:    [[ARRAYIDXD:%.*]] = getelementptr inbounds i32, ptr [[D]], i64 [[IND]]
; CHECK-NEXT:    [[LOADD:%.*]] = load i32, ptr [[ARRAYIDXD]], align 4, !alias.scope [[META14:![0-9]+]]
; CHECK-NEXT:    [[ARRAYIDXE:%.*]] = getelementptr inbounds i32, ptr [[E]], i64 [[IND]]
; CHECK-NEXT:    [[LOADE:%.*]] = load i32, ptr [[ARRAYIDXE]], align 4, !alias.scope [[META15:![0-9]+]]
; CHECK-NEXT:    [[MULC:%.*]] = mul i32 [[LOADD]], [[LOADE]]
; CHECK-NEXT:    [[ARRAYIDXC:%.*]] = getelementptr inbounds i32, ptr [[C]], i64 [[IND]]
; CHECK-NEXT:    store i32 [[MULC]], ptr [[ARRAYIDXC]], align 4, !alias.scope [[META16:![0-9]+]], !noalias [[META10]]
; CHECK-NEXT:    [[EXITCOND:%.*]] = icmp eq i64 [[ADD]], 20
; CHECK-NEXT:    br i1 [[EXITCOND]], label %[[FOR_END_LOOPEXIT16:.*]], label %[[FOR_BODY]], !llvm.loop [[LOOP17:![0-9]+]]
; CHECK:       [[FOR_END_LOOPEXIT]]:
; CHECK-NEXT:    br label %[[FOR_END:.*]]
; CHECK:       [[FOR_END_LOOPEXIT16]]:
; CHECK-NEXT:    br label %[[FOR_END]]
; CHECK:       [[FOR_END]]:
; CHECK-NEXT:    ret void
;
entry:
  br label %for.body

for.body:
  %ind = phi i64 [ 0, %entry ], [ %add, %for.body ]

  %arrayidxA = getelementptr inbounds i32, ptr %a, i64 %ind
  %loadA = load i32, ptr %arrayidxA, align 4

  %arrayidxB = getelementptr inbounds i32, ptr %b, i64 %ind
  %loadB = load i32, ptr %arrayidxB, align 4

  %mulA = mul i32 %loadB, %loadA

  %add = add nuw nsw i64 %ind, 1
  %arrayidxA_plus_4 = getelementptr inbounds i32, ptr %a, i64 %add
  store i32 %mulA, ptr %arrayidxA_plus_4, align 4

  %arrayidxD = getelementptr inbounds i32, ptr %d, i64 %ind
  %loadD = load i32, ptr %arrayidxD, align 4

  %arrayidxE = getelementptr inbounds i32, ptr %e, i64 %ind
  %loadE = load i32, ptr %arrayidxE, align 4

  %mulC = mul i32 %loadD, %loadE

  %arrayidxC = getelementptr inbounds i32, ptr %c, i64 %ind
  store i32 %mulC, ptr %arrayidxC, align 4

  %exitcond = icmp eq i64 %add, 20
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !0

for.end:
  ret void
}

!0 = distinct !{!0, !1, !2, !3, !4, !5}
!1 = !{!"llvm.loop.distribute.enable", i1 true}
!2 = !{!"llvm.loop.distribute.followup_all", !{!"FollowupAll"}}
!3 = !{!"llvm.loop.distribute.followup_coincident", !{!"FollowupCoincident", i1 false}}
!4 = !{!"llvm.loop.distribute.followup_sequential", !{!"FollowupSequential", i32 8}}
!5 = !{!"llvm.loop.distribute.followup_fallback", !{!"FollowupFallback"}}
;.
; CHECK: [[LOOP0]] = distinct !{[[LOOP0]], [[META1:![0-9]+]], [[META2:![0-9]+]]}
; CHECK: [[META1]] = !{!"FollowupAll"}
; CHECK: [[META2]] = !{!"FollowupFallback"}
; CHECK: [[META3]] = !{[[META4:![0-9]+]]}
; CHECK: [[META4]] = distinct !{[[META4]], [[META5:![0-9]+]]}
; CHECK: [[META5]] = distinct !{[[META5]], !"LVerDomain"}
; CHECK: [[META6]] = !{[[META7:![0-9]+]], [[META8:![0-9]+]], [[META9:![0-9]+]]}
; CHECK: [[META7]] = distinct !{[[META7]], [[META5]]}
; CHECK: [[META8]] = distinct !{[[META8]], [[META5]]}
; CHECK: [[META9]] = distinct !{[[META9]], [[META5]]}
; CHECK: [[META10]] = !{[[META11:![0-9]+]]}
; CHECK: [[META11]] = distinct !{[[META11]], [[META5]]}
; CHECK: [[LOOP12]] = distinct !{[[LOOP12]], [[META1]], [[META13:![0-9]+]]}
; CHECK: [[META13]] = !{!"FollowupSequential", i32 8}
; CHECK: [[META14]] = !{[[META8]]}
; CHECK: [[META15]] = !{[[META9]]}
; CHECK: [[META16]] = !{[[META7]]}
; CHECK: [[LOOP17]] = distinct !{[[LOOP17]], [[META1]], [[META18:![0-9]+]]}
; CHECK: [[META18]] = !{!"FollowupCoincident", i1 false}
;.