// RUN: rm -rf %t
// RUN: split-file %s %t
//
// RUN: clang-modules-converter --config=%t/config.yml
// RUN: cat %t/a.cppm | FileCheck %t/a.cppm.check
// RUN: cat %t/a.cpp | FileCheck %t/a.cpp.check

//--- config.yml
modules:
  - name: a
    path: a.cppm
    headers: a.h
  - name: b
    path: b.cppm
    headers: b.h
mode: header-wrapper
controlling_macro: USE_MODULES
is_std_module_available: false
srcs_to_rewrite: a.cpp

//--- c.h
#pragma once
#define B_VALUE = 44

//--- b.h
#pragma once
#define VALUE = 43

int b = B_VALUE;

//--- a.h
#ifndef A_H
#define A_H
// Comments before preamble
// Next Comments before preamble
#include "b.h"

int a = VALUE;
#endif // A_H

//--- a.cppm.check

// CHECK: // There unhandled macro uses found in the body:
// CHECK-NOT: 'B_VALUE'
// CHECK: // 'VALUE' defined in {{.*}}b.h
// CHECK: export module a;
// CHECK: import b;
// CHECK: #define USE_MODULES
// CHECK: export extern "C++" {
// CHECK:   #include "a.h"
// CHECK: }

//--- a.cpp
#include "b.h"

int a_impl = VALUE;

//--- a.cpp.check

// CHECK: // There unhandled macro uses found in the body:
// CHECK: // 'VALUE' defined in {{.*}}b.h
// CHECK: import b;
// CHECK: int a_impl = VALUE;
