// RUN: llvm-mc -triple=x86_64-unknown-unknown -mattr=+avxvnniint8 --show-encoding < %s  | FileCheck %s

// CHECK: vpdpbssd %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x17,0x50,0xe6]
     vpdpbssd %ymm14, %ymm13, %ymm12

// CHECK: vpdpbssd %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x13,0x50,0xe6]
     vpdpbssd %xmm14, %xmm13, %xmm12

// CHECK: vpdpbssd  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x17,0x50,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbssd  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK: vpdpbssd  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x17,0x50,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbssd  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK: vpdpbssd  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x17,0x50,0x25,0x00,0x00,0x00,0x00]
     vpdpbssd  (%rip), %ymm13, %ymm12

// CHECK: vpdpbssd  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x17,0x50,0x24,0x6d,0x00,0xfc,0xff,0xff]
     vpdpbssd  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK: vpdpbssd  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x13,0x50,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbssd  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK: vpdpbssd  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x13,0x50,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbssd  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK: vpdpbssd  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x13,0x50,0x25,0x00,0x00,0x00,0x00]
     vpdpbssd  (%rip), %xmm13, %xmm12

// CHECK: vpdpbssd  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x13,0x50,0x24,0x6d,0x00,0xfe,0xff,0xff]
     vpdpbssd  -512(,%rbp,2), %xmm13, %xmm12

// CHECK: vpdpbssds %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x17,0x51,0xe6]
     vpdpbssds %ymm14, %ymm13, %ymm12

// CHECK: vpdpbssds %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x13,0x51,0xe6]
     vpdpbssds %xmm14, %xmm13, %xmm12

// CHECK: vpdpbssds  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x17,0x51,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbssds  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK: vpdpbssds  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x17,0x51,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbssds  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK: vpdpbssds  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x17,0x51,0x25,0x00,0x00,0x00,0x00]
     vpdpbssds  (%rip), %ymm13, %ymm12

// CHECK: vpdpbssds  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x17,0x51,0x24,0x6d,0x00,0xfc,0xff,0xff]
     vpdpbssds  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK: vpdpbssds  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x13,0x51,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbssds  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK: vpdpbssds  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x13,0x51,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbssds  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK: vpdpbssds  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x13,0x51,0x25,0x00,0x00,0x00,0x00]
     vpdpbssds  (%rip), %xmm13, %xmm12

// CHECK: vpdpbssds  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x13,0x51,0x24,0x6d,0x00,0xfe,0xff,0xff]
     vpdpbssds  -512(,%rbp,2), %xmm13, %xmm12

// CHECK: vpdpbsud %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x16,0x50,0xe6]
     vpdpbsud %ymm14, %ymm13, %ymm12

// CHECK: vpdpbsud %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x12,0x50,0xe6]
     vpdpbsud %xmm14, %xmm13, %xmm12

// CHECK: vpdpbsud  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x16,0x50,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbsud  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK: vpdpbsud  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x16,0x50,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbsud  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK: vpdpbsud  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x16,0x50,0x25,0x00,0x00,0x00,0x00]
     vpdpbsud  (%rip), %ymm13, %ymm12

// CHECK: vpdpbsud  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x16,0x50,0x24,0x6d,0x00,0xfc,0xff,0xff]
     vpdpbsud  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK: vpdpbsud  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x12,0x50,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbsud  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK: vpdpbsud  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x12,0x50,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbsud  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK: vpdpbsud  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x12,0x50,0x25,0x00,0x00,0x00,0x00]
     vpdpbsud  (%rip), %xmm13, %xmm12

// CHECK: vpdpbsud  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x12,0x50,0x24,0x6d,0x00,0xfe,0xff,0xff]
     vpdpbsud  -512(,%rbp,2), %xmm13, %xmm12

// CHECK: vpdpbsuds %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x16,0x51,0xe6]
     vpdpbsuds %ymm14, %ymm13, %ymm12

// CHECK: vpdpbsuds %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x12,0x51,0xe6]
     vpdpbsuds %xmm14, %xmm13, %xmm12

// CHECK: vpdpbsuds  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x16,0x51,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbsuds  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK: vpdpbsuds  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x16,0x51,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbsuds  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK: vpdpbsuds  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x16,0x51,0x25,0x00,0x00,0x00,0x00]
     vpdpbsuds  (%rip), %ymm13, %ymm12

// CHECK: vpdpbsuds  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x16,0x51,0x24,0x6d,0x00,0xfc,0xff,0xff]
     vpdpbsuds  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK: vpdpbsuds  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x12,0x51,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbsuds  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK: vpdpbsuds  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x12,0x51,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbsuds  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK: vpdpbsuds  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x12,0x51,0x25,0x00,0x00,0x00,0x00]
     vpdpbsuds  (%rip), %xmm13, %xmm12

// CHECK: vpdpbsuds  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x12,0x51,0x24,0x6d,0x00,0xfe,0xff,0xff]
     vpdpbsuds  -512(,%rbp,2), %xmm13, %xmm12

// CHECK: vpdpbuud %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x14,0x50,0xe6]
     vpdpbuud %ymm14, %ymm13, %ymm12

// CHECK: vpdpbuud %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x10,0x50,0xe6]
     vpdpbuud %xmm14, %xmm13, %xmm12

// CHECK: vpdpbuud  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x14,0x50,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbuud  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK: vpdpbuud  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x14,0x50,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbuud  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK: vpdpbuud  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x14,0x50,0x25,0x00,0x00,0x00,0x00]
     vpdpbuud  (%rip), %ymm13, %ymm12

// CHECK: vpdpbuud  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x14,0x50,0x24,0x6d,0x00,0xfc,0xff,0xff]
     vpdpbuud  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK: vpdpbuud  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x10,0x50,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbuud  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK: vpdpbuud  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x10,0x50,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbuud  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK: vpdpbuud  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x10,0x50,0x25,0x00,0x00,0x00,0x00]
     vpdpbuud  (%rip), %xmm13, %xmm12

// CHECK: vpdpbuud  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x10,0x50,0x24,0x6d,0x00,0xfe,0xff,0xff]
     vpdpbuud  -512(,%rbp,2), %xmm13, %xmm12

// CHECK: vpdpbuuds %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x14,0x51,0xe6]
     vpdpbuuds %ymm14, %ymm13, %ymm12

// CHECK: vpdpbuuds %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x10,0x51,0xe6]
     vpdpbuuds %xmm14, %xmm13, %xmm12

// CHECK: vpdpbuuds  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x14,0x51,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbuuds  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK: vpdpbuuds  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x14,0x51,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbuuds  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK: vpdpbuuds  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x14,0x51,0x25,0x00,0x00,0x00,0x00]
     vpdpbuuds  (%rip), %ymm13, %ymm12

// CHECK: vpdpbuuds  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x14,0x51,0x24,0x6d,0x00,0xfc,0xff,0xff]
     vpdpbuuds  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK: vpdpbuuds  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x10,0x51,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vpdpbuuds  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK: vpdpbuuds  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x10,0x51,0xa4,0x80,0x23,0x01,0x00,0x00]
     vpdpbuuds  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK: vpdpbuuds  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x10,0x51,0x25,0x00,0x00,0x00,0x00]
     vpdpbuuds  (%rip), %xmm13, %xmm12

// CHECK: vpdpbuuds  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x10,0x51,0x24,0x6d,0x00,0xfe,0xff,0xff]
     vpdpbuuds  -512(,%rbp,2), %xmm13, %xmm12
