// Same with basic-rewrite-to-modules-01.cpp, except we don't keep traditional ABI.
// RUN: rm -rf %t
// RUN: split-file %s %t
//
// RUN: sed "s|__WORK_DIR__|%/t|g" %t/config.yml.in > %t/config.yml
// RUN: clang-modules-converter --config=%t/config.yml
// RUN: cat %t/include/a.h | FileCheck %t/include/a.check
// RUN: cat %t/include/a.cppm | FileCheck %t/include/a.cppm.check
// RUN: cat %t/a.cppm | FileCheck %t/a.cppm.check
// RUN: cat %t/use_a.cpp | FileCheck %t/use_a.cpp.check
// RUN: cat %t/b.cpp | FileCheck %t/b.cpp.check

//--- config.yml.in
modules:
  - name: a
    path: a.cppm
    headers: include/a.h
  - name: b
    path: b.cppm
    headers: include/b.h
    srcs: b.cpp
third_party_modules:
  - name: third_party
    headers: .*/third_party/.*.h
mode: rewrite-headers-to-module-units
controlling_macro: USE_MODULES
is_std_module_available: false
default_compile_commands: -I__WORK_DIR__ -I__WORK_DIR__/include
srcs_to_rewrite:
  - "*.cpp"
keep_traditional_abi: false

//--- include/b.h
#pragma once

//--- include/a.h
#ifndef A_H
#define A_H
// Comments before preamble
// Next Comments before preamble
#include "b.h"
#include "third/third_party/third.h"

int a;
#endif // A_H

//--- include/a.cppm.check
// CHECK-NOT: WARNING
// CHECK-NOT: #ifndef A_H
// CHECK: export module a.a;
// CHECK: import b;
// CHECK: import third_party;
// CHECK: #define USE_MODULES
// CHECK: export {
// CHECK: #include "a.h"
// CHECK: }

//--- third/third_party/third.h
#pragma once

//--- use_a.cpp
// File Comments
#include "a.h"
#include "b.h"
#include "third/third_party/third.h"

int body = a;

//--- use_a.cpp.check
// CHECK: File Comments
// CHECK: import a.a;
// CHECK-NEXT: import b.b;
// CHECK-NEXT: import third_party;
//
// CHECK: int body = a;

//--- non-interesting.h
#pragma once

//--- b.cpp
#include "a.h"
#include "b.h"
#include "third/third_party/third.h"
#include "non-interesting.h"

int b = 43;

//--- b.cpp.check
// CHECK: module;
// CHECK: // WARNING:
// CHECK: #include "non-interesting.h"
// CHECK: module b;
// CHECK: import a;
// CHECK: import b.b;
// CHECK: import third_party;
// CHECK: int b = 43;

//--- include/a.check
// CHECK: // Comments before preamble
// CHECK-NEXT: // Next Comments before preamble
// CHECK-NEXT: #ifndef USE_MODULES
// CHECK-NEXT: #include "b.h"
// CHECK: #endif // USE_MODULES
// CHECK: int a;

//--- a.cppm.check
// CHECK: export module a;
// CHECK: export import a.a;
