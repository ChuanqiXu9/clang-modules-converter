// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py UTC_ARGS: --filter "^define |^entry:" --version 2
// RUN: %clang_cc1 -triple riscv32 -emit-llvm %s -o - \
// RUN:   | FileCheck -check-prefixes=ILP32-ILP32F-ILP32D,ILP32-ILP32F,ILP32 %s
// RUN: %clang_cc1 -triple riscv32 -target-feature +f -target-abi ilp32f -emit-llvm %s -o - \
// RUN:   | FileCheck -check-prefixes=ILP32-ILP32F-ILP32D,ILP32F-ILP32D,ILP32-ILP32F,ILP32F %s
// RUN: %clang_cc1 -triple riscv32 -target-feature +f -target-feature +d -target-abi ilp32d -emit-llvm %s -o - \
// RUN:   | FileCheck -check-prefixes=ILP32-ILP32F-ILP32D,ILP32F-ILP32D,ILP32D %s

// RUN: %clang_cc1 -triple riscv64 -emit-llvm %s -o - \
// RUN:   | FileCheck -check-prefixes=LP64-LP64F-LP64D,LP64-LP64F,LP64 %s
// RUN: %clang_cc1 -triple riscv64 -target-feature +f -target-abi lp64f -emit-llvm %s -o - \
// RUN:   | FileCheck -check-prefixes=LP64-LP64F-LP64D,LP64F-LP64D,LP64-LP64F,LP64F %s
// RUN: %clang_cc1 -triple riscv64 -target-feature +f -target-feature +d -target-abi lp64d -emit-llvm %s -o - \
// RUN:   | FileCheck -check-prefixes=LP64-LP64F-LP64D,LP64F-LP64D,LP64D %s

#include <stdint.h>

// Ensure that fields inherited from a parent struct are treated in the same
// way as fields directly in the child for the purposes of RISC-V ABI rules.

struct parent1_int32_s {
  int32_t i1;
};

struct child1_int32_s : parent1_int32_s {
  int32_t i2;
};

// ILP32-ILP32F-ILP32D-LABEL: define dso_local [2 x i32] @_Z30int32_int32_struct_inheritance14child1_int32_s
// ILP32-ILP32F-ILP32D-SAME: ([2 x i32] [[A_COERCE:%.*]]) #[[ATTR0:[0-9]+]] {
// ILP32-ILP32F-ILP32D:  entry:
//
// LP64-LP64F-LP64D-LABEL: define dso_local i64 @_Z30int32_int32_struct_inheritance14child1_int32_s
// LP64-LP64F-LP64D-SAME: (i64 [[A_COERCE:%.*]]) #[[ATTR0:[0-9]+]] {
// LP64-LP64F-LP64D:  entry:
//
struct child1_int32_s int32_int32_struct_inheritance(struct child1_int32_s a) {
  return a;
}

struct parent2_int32_s {
  int32_t i1;
};

struct child2_float_s : parent2_int32_s {
  float f1;
};

// ILP32-LABEL: define dso_local [2 x i32] @_Z30int32_float_struct_inheritance14child2_float_s
// ILP32-SAME: ([2 x i32] [[A_COERCE:%.*]]) #[[ATTR0]] {
// ILP32:  entry:
//
// ILP32F-ILP32D-LABEL: define dso_local { i32, float } @_Z30int32_float_struct_inheritance14child2_float_s
// ILP32F-ILP32D-SAME: (i32 [[TMP0:%.*]], float [[TMP1:%.*]]) #[[ATTR0]] {
// ILP32F-ILP32D:  entry:
//
// LP64-LABEL: define dso_local i64 @_Z30int32_float_struct_inheritance14child2_float_s
// LP64-SAME: (i64 [[A_COERCE:%.*]]) #[[ATTR0]] {
// LP64:  entry:
//
// LP64F-LP64D-LABEL: define dso_local { i32, float } @_Z30int32_float_struct_inheritance14child2_float_s
// LP64F-LP64D-SAME: (i32 [[TMP0:%.*]], float [[TMP1:%.*]]) #[[ATTR0]] {
// LP64F-LP64D:  entry:
//
struct child2_float_s int32_float_struct_inheritance(struct child2_float_s a) {
  return a;
}

struct parent3_float_s {
  float f1;
};

struct child3_int64_s : parent3_float_s {
  int64_t i1;
};

// ILP32-ILP32F-ILP32D-LABEL: define dso_local void @_Z30float_int64_struct_inheritance14child3_int64_s
// ILP32-ILP32F-ILP32D-SAME: (ptr dead_on_unwind noalias writable sret([[STRUCT_CHILD3_INT64_S:%.*]]) align 8 [[AGG_RESULT:%.*]], ptr noundef [[A:%.*]]) #[[ATTR0]] {
// ILP32-ILP32F-ILP32D:  entry:
//
// LP64-LABEL: define dso_local [2 x i64] @_Z30float_int64_struct_inheritance14child3_int64_s
// LP64-SAME: ([2 x i64] [[A_COERCE:%.*]]) #[[ATTR0]] {
// LP64:  entry:
//
// LP64F-LP64D-LABEL: define dso_local { float, i64 } @_Z30float_int64_struct_inheritance14child3_int64_s
// LP64F-LP64D-SAME: (float [[TMP0:%.*]], i64 [[TMP1:%.*]]) #[[ATTR0]] {
// LP64F-LP64D:  entry:
//
struct child3_int64_s float_int64_struct_inheritance(struct child3_int64_s a) {
  return a;
}

struct parent4_double_s {
  double d1;
};

struct child4_double_s : parent4_double_s {
  double d1;
};

// ILP32-ILP32F-LABEL: define dso_local void @_Z32double_double_struct_inheritance15child4_double_s
// ILP32-ILP32F-SAME: (ptr dead_on_unwind noalias writable sret([[STRUCT_CHILD4_DOUBLE_S:%.*]]) align 8 [[AGG_RESULT:%.*]], ptr noundef [[A:%.*]]) #[[ATTR0]] {
// ILP32-ILP32F:  entry:
//
// ILP32D-LABEL: define dso_local { double, double } @_Z32double_double_struct_inheritance15child4_double_s
// ILP32D-SAME: (double [[TMP0:%.*]], double [[TMP1:%.*]]) #[[ATTR0]] {
// ILP32D:  entry:
//
// LP64-LP64F-LABEL: define dso_local [2 x i64] @_Z32double_double_struct_inheritance15child4_double_s
// LP64-LP64F-SAME: ([2 x i64] [[A_COERCE:%.*]]) #[[ATTR0]] {
// LP64-LP64F:  entry:
//
// LP64D-LABEL: define dso_local { double, double } @_Z32double_double_struct_inheritance15child4_double_s
// LP64D-SAME: (double [[TMP0:%.*]], double [[TMP1:%.*]]) #[[ATTR0]] {
// LP64D:  entry:
//
struct child4_double_s double_double_struct_inheritance(struct child4_double_s a) {
  return a;
}

// When virtual inheritance is used, the resulting struct isn't eligible for
// passing in registers.

struct parent5_virtual_s {
  int32_t i1;
};

struct child5_virtual_s : virtual parent5_virtual_s {
  float f1;
};

// ILP32-ILP32F-ILP32D-LABEL: define dso_local void @_Z38int32_float_virtual_struct_inheritance16child5_virtual_s
// ILP32-ILP32F-ILP32D-SAME: (ptr dead_on_unwind noalias writable sret([[STRUCT_CHILD5_VIRTUAL_S:%.*]]) align 4 [[AGG_RESULT:%.*]], ptr noundef [[A:%.*]]) #[[ATTR0]] {
// ILP32-ILP32F-ILP32D:  entry:
//
// LP64-LP64F-LP64D-LABEL: define dso_local void @_Z38int32_float_virtual_struct_inheritance16child5_virtual_s
// LP64-LP64F-LP64D-SAME: (ptr dead_on_unwind noalias writable sret([[STRUCT_CHILD5_VIRTUAL_S:%.*]]) align 8 [[AGG_RESULT:%.*]], ptr noundef [[A:%.*]]) #[[ATTR0]] {
// LP64-LP64F-LP64D:  entry:
//
struct child5_virtual_s int32_float_virtual_struct_inheritance(struct child5_virtual_s a) {
  return a;
}

// Check for correct lowering in the presence of diamond inheritance.

struct parent6_float_s {
  float f1;
};

struct child6a_s : parent6_float_s {
};

struct child6b_s : parent6_float_s {
};

struct grandchild_6_s : child6a_s, child6b_s {
};

// ILP32F-ILP64D: define{{.*}} { float, float } @_Z38float_float_diamond_struct_inheritance14grandchild_6_s(float %0, float %1)
// ILP32-LABEL: define dso_local [2 x i32] @_Z38float_float_diamond_struct_inheritance14grandchild_6_s
// ILP32-SAME: ([2 x i32] [[A_COERCE:%.*]]) #[[ATTR0]] {
// ILP32:  entry:
//
// ILP32F-ILP32D-LABEL: define dso_local { float, float } @_Z38float_float_diamond_struct_inheritance14grandchild_6_s
// ILP32F-ILP32D-SAME: (float [[TMP0:%.*]], float [[TMP1:%.*]]) #[[ATTR0]] {
// ILP32F-ILP32D:  entry:
//
// LP64-LABEL: define dso_local i64 @_Z38float_float_diamond_struct_inheritance14grandchild_6_s
// LP64-SAME: (i64 [[A_COERCE:%.*]]) #[[ATTR0]] {
// LP64:  entry:
//
// LP64F-LP64D-LABEL: define dso_local { float, float } @_Z38float_float_diamond_struct_inheritance14grandchild_6_s
// LP64F-LP64D-SAME: (float [[TMP0:%.*]], float [[TMP1:%.*]]) #[[ATTR0]] {
// LP64F-LP64D:  entry:
//
struct grandchild_6_s float_float_diamond_struct_inheritance(struct grandchild_6_s a) {
  return a;
}

// NOTE: These prefixes are unused. Do not add tests below this line:
//// NOTE: These prefixes are unused and the list is autogenerated. Do not add tests below this line:
// ILP32F: {{.*}}
// LP64F: {{.*}}