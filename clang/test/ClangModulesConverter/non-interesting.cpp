// RUN: rm -rf %t
// RUN: split-file %s %t
//
// RUN: sed "s|WORK_DIR|%/t|g" %t/config.yml.in > %t/config.yml
// RUN: clang-modules-converter --config=%t/config.yml
// RUN: cat %t/a.h | FileCheck %t/a.check
// RUN: cat %t/a.cppm | FileCheck %t/a.cppm.check
// RUN: cat %t/a.cpp | FileCheck %t/a.cpp.check

//--- config.yml.in
modules:
  - name: a
    path: a.cppm
    headers:
      - a.h
  - name: b
    path: b.cppm
    headers:
      - b.h
mode: header-wrapper
controlling_macro: USE_MODULES
is_std_module_available: false
srcs_to_rewrite: a.cpp

//--- non-interesting.h
#pragma once

//--- b.h
#pragma once

//--- a.h
// Comments in the top of the file.
#ifndef A_H
#define A_H
// Comments before preamble
// Next Comments before preamble
#include "b.h"
#include "non-interesting.h"

// Comments for a
int a;
#endif // A_H

//--- a.check
// CHECK: // Comments in the top of the file.
// CHECK-NEXT: #ifndef A_H
// CHECK-NEXT: #define A_H
// CHECK-NEXT: // Comments before preamble
// CHECK-NEXT: // Next Comments before preamble
// CHECK-NEXT: #ifndef USE_MODULES
// CHECK-NEXT: #include "b.h"
// CHECK: #endif // USE_MODULES
// CHECK: // Comments for a
// CHECK: int a;
// CHECK: #endif // A_H

//--- a.cpp
#include "b.h"
#include "non-interesting.h"

int a;

//--- a.cpp.check
// CHECK: // WARNING: Detected unhandled non interesting includes.
// CHECK: import b;
// CHECK: int a; 

//--- a.cppm.check
// CHECK: module;
// CHECK: // WARNING: Detected unhandled non interesting includes.
// CHECK: #include "non-interesting.h"
// CHECK: export module a;
// CHECK: import b;
// CHECK: #define USE_MODULES
// CHECK: export extern "C++" {
// CHECK:   #include "a.h"
// CHECK: }
