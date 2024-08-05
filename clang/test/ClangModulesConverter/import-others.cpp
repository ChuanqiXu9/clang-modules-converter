// RUN: rm -rf %t
// RUN: split-file %s %t
//
// RUN: clang-modules-converter --config=%t/config.yml
// RUN: cat %t/a.h | FileCheck %t/a.check
// RUN: cat %t/a.cppm | FileCheck %t/a.cppm.check
// RUN: cat %t/a.cpp | FileCheck %t/a.cpp.check
// RUN: cat %t/b.cpp | FileCheck %t/b.cpp.check

//--- config.yml
modules:
  - name: a
    path: a.cppm
    headers: a.h
  - name: b
    path: b.cppm
    headers: b.h
third_party_modules:
  - name: third_party
    headers: .*/third_party/.*.h
mode: header-wrapper
controlling_macro: USE_MODULES
is_std_module_available: false
srcs_to_rewrite:
  - "*.cpp"
srcs_excluded_to_rewrite:
  - b.cpp

//--- b.h
#pragma once

//--- a.h
#ifndef A_H
#define A_H
// Comments before preamble
// Next Comments before preamble
#include "b.h"
#include "third/third_party/third.h"

int a;
#endif // A_H

//--- third/third_party/third.h
#pragma once

//--- a.cpp
// File Comments
#include "a.h"
#include "b.h"
#include "third/third_party/third.h"

int body = a;

//--- a.cpp.check
// CHECK: File Comments
// CHECK: import a;
// CHECK-NEXT: import b;
// CHECK-NEXT: import third_party;
//
// CHECK: int body = a;

//--- b.cpp
// Test srcs_excluded_to_rewrite
#include "a.h"
#include "b.h"
#include "third/third_party/third.h"

//--- b.cpp.check
// CHECK: #include "a.h"
// CHECK-NEXT: #include "b.h"
// CHECK-NEXT: #include "third/third_party/third.h"

//--- a.check
// CHECK: // Comments before preamble
// CHECK-NEXT: // Next Comments before preamble
// CHECK-NEXT: #ifndef USE_MODULES
// CHECK-NEXT: #include "b.h"
// CHECK: #endif // USE_MODULES
// CHECK: int a;

//--- a.cppm.check
// CHECK: export module a;
// CHECK: import b;
// CHECK: import third_party;
// CHECK: #define USE_MODULES
// CHECK: export extern "C++" {
// CHECK:   #include "a.h"
// CHECK: }
