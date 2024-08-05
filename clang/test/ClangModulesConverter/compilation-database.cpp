// RUN: rm -rf %t
// RUN: split-file %s %t
//
// RUN: sed "s|__WORK_DIR__|%/t|g" %t/compilation_database.json.in > %t/compilation_database.json
// RUN: clang-modules-converter --config=%t/config.yml
// RUN: cat %t/srcs/use.cc | FileCheck %t/srcs/use.cc.check
// RUN: cat %t/include/a.h | FileCheck %t/include/a.check
// RUN: cat %t/module/a.cppm | FileCheck %t/module/a.cppm.check
// RUN: cat %t/a.cppm | FileCheck %t/a.cppm.check

//--- config.yml
modules:
  - name: a
    path: a.cppm
    headers: include/a.h
    prefix_map_of_module_units_for_headers: "include:module"
  - name: b
    path: b.cppm
    headers: include/b.h
    prefix_map_of_module_units_for_headers: "include:module"
third_party_modules:
  - name: third_party
    headers: third_party/.*.h
mode: rewrite-headers-to-module-units
controlling_macro: USE_MODULES
is_std_module_available: false
compilation_database: compilation_database.json
srcs_to_rewrite:
  - "srcs/*.cc"
keep_traditional_abi: false

//--- compilation_database.json.in
[
{
  "directory": "__WORK_DIR__",
  "command": "clang++ -std=c++20 __WORK_DIR__/srcs/use.cc -I__WORK_DIR__/include -I__WORK_DIR__ -DUSE_B -c -o use.o",
  "file": "__WORK_DIR__/srcs/use.cc",
}
]

//--- include/not_interested.h
#pragma once

//--- include/b.h
#pragma once

//--- third_party/third.h
#pragma once

//--- include/a.h
// Comments at the top of file
#ifndef A_H
#define A_H
// Comments before preamble
// Next Comments before preamble
#ifdef USE_B
#include "b.h"
#endif // USE_B
#include "third_party/third.h"
#include "not_interested.h"

int a;
#endif // A_H

//--- include/a.check
// CHECK: // Comments at the top of file
// CHECK: // Comments before preamble
// CHECK-NEXT: // Next Comments before preamble
// CHECK-NEXT: #ifndef USE_MODULES
// CHECK-NEXT: #ifdef USE_B
// CHECK-NEXT: #include "b.h"
// CHECK-NEXT: #endif
// CHECK-NEXT: #include "third_party/third.h"
// CHECK-NEXT: #include "not_interested.h"
// CHECK: #endif // USE_MODULES
// CHECK: int a;

//--- module/a.cppm.check
// CHECK: // Comments at the top of file
// CHECK-NOT: #ifndef A_H
// CHECK: module;
// CHECK: // WARNING:
// CHECK: #include "not_interested.h"
// CHECK: export module a.a;
// CHECK: import b;
// CHECK: import third_party;
// CHECK: #define USE_MODULES
// CHECK: export {
// CHECK: #include "include/a.h"
// CHECK: }

//--- a.cppm.check
// CHECK: export module a;
// CHECK: export import a.a;

//--- srcs/use.cc
#ifdef USE_B
#include "b.h"
#endif // USE_B
#include "a.h"
// #include "b.h"
#include "third_party/third.h"
#include "not_interested.h"

int use = a;


//--- srcs/use.cc.check
// CHECK: // WARNING: Detected unhandled non interesting includes.
// CHECK: #include "not_interested.h"
// CHECK: import b.b;
// CHECK: import a.a;
// CHECK: import third_party;
// CHECK: int use = a;
