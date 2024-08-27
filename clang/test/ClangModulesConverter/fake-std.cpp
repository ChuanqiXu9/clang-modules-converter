// RUN: rm -rf %t
// RUN: split-file %s %t
//
// RUN: sed "s|WORK_DIR|%/t|g" %t/config.yml.in > %t/config.yml
// RUN: clang-modules-converter --config=%t/config.yml
// RUN: cat %t/a.h | FileCheck %t/a.check
// RUN: cat %t/a.cppm | FileCheck %t/a.cppm.check
// RUN: cat %t/a.cpp | FileCheck %t/a.cpp.check
// RUN: cat %t/modules/std.cppm | FileCheck %t/modules/std.cppm.check

//--- config.yml.in
modules:
  - name: a
    path: a.cppm
    headers:
      - a.h
mode: header-wrapper
controlling_macro: USE_MODULES
is_std_module_available: false
default_compile_commands: -nostdinc++ -isystem WORK_DIR
srcs_to_rewrite: a.cpp
third_party_modules:
  - name: std
    headers: new_std_header
std_module_path: WORK_DIR/modules/std.cppm

//--- vector
#pragma once

//--- new_std_header
#pragma once

#include <stddef.h>

//--- a.h
// Comments in the top of the file.
#ifndef A_H
#define A_H
// Comments before preamble
// Next Comments before preamble
#include <vector>

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
// CHECK-NEXT: #include <vector>
// CHECK: #endif // USE_MODULES
// CHECK: // Comments for a
// CHECK: int a;
// CHECK: #endif // A_H

//--- a.cpp
#include <new_std_header>

int a;

//--- a.cpp.check
//CHECK: import std;
//CHECK: int a; 

//--- a.cppm.check
// CHECK: export module a;
// CHECK: import std;
// CHECK: #define USE_MODULES
// CHECK: export extern "C++" {
// CHECK:   #include "a.h"
// CHECK: }

//--- modules/std.cppm.check
// CHECK: // FIXME: This is a workaround implementation for std module
// CHECK: module;
// CHECK: #include <vector>
// CHECK: #include <new_std_header>
// CHECK-NOT: #include <set>
// CHECK: export module std;
// CHECK: export namespace std {
// CHECK:   using std::vector;
// CHECK: }
