; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py UTC_ARGS: --version 4
; RUN: llc < %s -mtriple=avr -mcpu=atmega328 -O1 -verify-machineinstrs | FileCheck %s

define internal i8 @main() {
; CHECK-LABEL: main:
; CHECK:       ; %bb.0: ; %bb0
; CHECK-NEXT:    push r2
; CHECK-NEXT:    push r3
; CHECK-NEXT:    push r4
; CHECK-NEXT:    push r5
; CHECK-NEXT:    push r6
; CHECK-NEXT:    push r7
; CHECK-NEXT:    push r8
; CHECK-NEXT:    push r9
; CHECK-NEXT:    push r10
; CHECK-NEXT:    push r11
; CHECK-NEXT:    push r12
; CHECK-NEXT:    push r13
; CHECK-NEXT:    push r14
; CHECK-NEXT:    push r15
; CHECK-NEXT:    push r16
; CHECK-NEXT:    push r17
; CHECK-NEXT:    push r28
; CHECK-NEXT:    push r29
; CHECK-NEXT:    in r28, 61
; CHECK-NEXT:    in r29, 62
; CHECK-NEXT:    sbiw r28, 13
; CHECK-NEXT:    in r0, 63
; CHECK-NEXT:    cli
; CHECK-NEXT:    out 62, r29
; CHECK-NEXT:    out 63, r0
; CHECK-NEXT:    out 61, r28
; CHECK-NEXT:    ldi r16, 0
; CHECK-NEXT:    ldi r17, 0
; CHECK-NEXT:    ldi r18, -1
; CHECK-NEXT:    ;APP
; CHECK-NEXT:    ldi r24, 123
; CHECK-NEXT:    ;NO_APP
; CHECK-NEXT:    std Y+1, r24 ; 1-byte Folded Spill
; CHECK-NEXT:    movw r24, r28
; CHECK-NEXT:    adiw r24, 6
; CHECK-NEXT:    std Y+3, r25 ; 2-byte Folded Spill
; CHECK-NEXT:    std Y+2, r24 ; 2-byte Folded Spill
; CHECK-NEXT:    movw r8, r16
; CHECK-NEXT:    movw r6, r16
; CHECK-NEXT:    movw r4, r16
; CHECK-NEXT:    movw r2, r16
; CHECK-NEXT:    rjmp .LBB0_2
; CHECK-NEXT:  .LBB0_1: ; %bb1
; CHECK-NEXT:    ; in Loop: Header=BB0_2 Depth=1
; CHECK-NEXT:    andi r30, 1
; CHECK-NEXT:    ldd r31, Y+4 ; 1-byte Folded Reload
; CHECK-NEXT:    dec r31
; CHECK-NEXT:    cpi r30, 0
; CHECK-NEXT:    movw r8, r18
; CHECK-NEXT:    movw r6, r20
; CHECK-NEXT:    movw r4, r22
; CHECK-NEXT:    movw r2, r24
; CHECK-NEXT:    mov r18, r31
; CHECK-NEXT:    brne .LBB0_2
; CHECK-NEXT:    rjmp .LBB0_4
; CHECK-NEXT:  .LBB0_2: ; %bb1
; CHECK-NEXT:    ; =>This Inner Loop Header: Depth=1
; CHECK-NEXT:    std Y+4, r18 ; 1-byte Folded Spill
; CHECK-NEXT:    movw r18, r8
; CHECK-NEXT:    movw r20, r6
; CHECK-NEXT:    movw r22, r4
; CHECK-NEXT:    movw r24, r2
; CHECK-NEXT:    ldi r26, 10
; CHECK-NEXT:    ldi r27, 0
; CHECK-NEXT:    movw r10, r26
; CHECK-NEXT:    movw r12, r16
; CHECK-NEXT:    movw r14, r16
; CHECK-NEXT:    call __udivdi3
; CHECK-NEXT:    std Y+13, r25
; CHECK-NEXT:    std Y+12, r24
; CHECK-NEXT:    std Y+11, r23
; CHECK-NEXT:    std Y+10, r22
; CHECK-NEXT:    std Y+9, r21
; CHECK-NEXT:    std Y+8, r20
; CHECK-NEXT:    std Y+7, r19
; CHECK-NEXT:    std Y+6, r18
; CHECK-NEXT:    ldd r30, Y+2 ; 2-byte Folded Reload
; CHECK-NEXT:    ldd r31, Y+3 ; 2-byte Folded Reload
; CHECK-NEXT:    ;APP
; CHECK-NEXT:    ;NO_APP
; CHECK-NEXT:    ldi r30, 1
; CHECK-NEXT:    cp r8, r1
; CHECK-NEXT:    cpc r9, r1
; CHECK-NEXT:    cpc r6, r16
; CHECK-NEXT:    cpc r7, r17
; CHECK-NEXT:    cpc r4, r16
; CHECK-NEXT:    cpc r5, r17
; CHECK-NEXT:    cpc r2, r16
; CHECK-NEXT:    cpc r3, r17
; CHECK-NEXT:    breq .LBB0_3
; CHECK-NEXT:    rjmp .LBB0_1
; CHECK-NEXT:  .LBB0_3: ; %bb1
; CHECK-NEXT:    ; in Loop: Header=BB0_2 Depth=1
; CHECK-NEXT:    mov r30, r1
; CHECK-NEXT:    rjmp .LBB0_1
; CHECK-NEXT:  .LBB0_4: ; %bb3
; CHECK-NEXT:    ldd r24, Y+1 ; 1-byte Folded Reload
; CHECK-NEXT:    std Y+5, r24
; CHECK-NEXT:    movw r24, r28
; CHECK-NEXT:    adiw r24, 5
; CHECK-NEXT:    ;APP
; CHECK-NEXT:    ;NO_APP
; CHECK-NEXT:    ldd r24, Y+5
; CHECK-NEXT:    adiw r28, 13
; CHECK-NEXT:    in r0, 63
; CHECK-NEXT:    cli
; CHECK-NEXT:    out 62, r29
; CHECK-NEXT:    out 63, r0
; CHECK-NEXT:    out 61, r28
; CHECK-NEXT:    pop r29
; CHECK-NEXT:    pop r28
; CHECK-NEXT:    pop r17
; CHECK-NEXT:    pop r16
; CHECK-NEXT:    pop r15
; CHECK-NEXT:    pop r14
; CHECK-NEXT:    pop r13
; CHECK-NEXT:    pop r12
; CHECK-NEXT:    pop r11
; CHECK-NEXT:    pop r10
; CHECK-NEXT:    pop r9
; CHECK-NEXT:    pop r8
; CHECK-NEXT:    pop r7
; CHECK-NEXT:    pop r6
; CHECK-NEXT:    pop r5
; CHECK-NEXT:    pop r4
; CHECK-NEXT:    pop r3
; CHECK-NEXT:    pop r2
; CHECK-NEXT:    ret
bb0:
  %0 = alloca i64
  %1 = alloca i8
  %2 = tail call i8 asm sideeffect "ldi ${0}, 123", "=&r,~{sreg},~{memory}"()

  br label %bb1

bb1:
  %3 = phi i64 [ %5, %bb1 ], [ 0, %bb0 ]
  %4 = phi i8 [ %6, %bb1 ], [ 0, %bb0 ]
  %5 = udiv i64 %3, 10
  %6 = add i8 %4, 1

  store i64 %5, ptr %0
  call void asm sideeffect "", "r,~{memory}"(ptr %0)

  %7 = icmp eq i64 %3, 0
  %8 = icmp eq i8 %6, 0

  br i1 %7, label %bb3, label %bb1

bb3:
  store i8 %2, ptr %1
  call void asm sideeffect "", "r,~{memory}"(ptr %1)

  %9 = load i8, ptr %1

  ret i8 %9
}