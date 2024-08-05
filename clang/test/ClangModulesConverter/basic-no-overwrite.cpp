// RUN: rm -rf %t
// RUN: split-file %s %t
//
// RUN: clang-modules-converter --config=%t/config.yml
// RUN: cat %t/a.h | FileCheck %t/a.check
// RUN: cat %t/b.h | FileCheck %t/b.check
// RUN: cat %t/a.cppm | FileCheck %t/a.cppm.check

//--- config.yml
modules:
  - name: a
    path: a.cppm
    headers:
      - a.h
      - b.h
mode: header-wrapper
controlling_macro: USE_MODULES
is_std_module_available: false

//--- b.h
#ifndef FAKE_CONTROLLING_MACRO
#define FAKE_CONTROLLING_MACRO
#include "c.h"
#endif // FAKE_CONTROLLING_MACRO
#include "d.h"

//--- c.h
#pragma once

//--- d.h
#pragma once

//--- a.h
// Comments in the top of the file.
#ifndef A_H
#define A_H
// Comments before preamble
// Next Comments before preamble
#ifndef USE_MODULES
#include "b.h"
#endif // USE_MODULES

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
// CHECK-NOT: #ifndef USE_MODULES
// CHECK-NEXT: #include "b.h"
// CHECK: #endif // USE_MODULES
// CHECK-NOT: #ifndef USE_MODULES
// CHECK: // Comments for a
// CHECK: int a;
// CHECK: #endif // A_H

//--- a.cppm.check
// CHECK: export module a;
// CHECK: #define USE_MODULES
// CHECK: export extern "C++" {
// CHECK:   #include "a.h"
// CHECK: }

//--- b.check
// CHECK: #ifndef USE_MODULES
// CHECK-NEXT: #ifndef FAKE_CONTROLLING_MACRO
// CHECK-NEXT: #define FAKE_CONTROLLING_MACRO
// CHECK-NEXT: #include "c.h"
// CHECK-NEXT: #endif // FAKE_CONTROLLING_MACRO
// CHECK-NEXT: #include "d.h"
// CHECK: #endif // USE_MODULES
