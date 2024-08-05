
//===- Log.h ----------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_TOOLS_MODULES_CONVERTER_LOG_H
#define CLANG_TOOLS_MODULES_CONVERTER_LOG_H

#include "llvm/Support/Debug.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FormatAdapters.h"
#include "llvm/Support/FormatVariadic.h"

namespace converter {

namespace detail {
enum class Level { Debug, Info, Warning, Error };
inline void logImpl(Level L, const llvm::formatv_object_base &Message) {
  switch (L) {
  case Level::Info:
    llvm::errs().changeColor(llvm::raw_ostream::GREEN);
    llvm::errs() << "Info: ";
    llvm::errs().resetColor();
    break;

  case Level::Warning:
    llvm::errs().changeColor(llvm::raw_ostream::YELLOW);
    llvm::errs() << "Warn: ";
    llvm::errs().resetColor();
    break;

  case Level::Error:
    llvm::errs().changeColor(llvm::raw_ostream::RED);
    llvm::errs() << "Error: ";
    llvm::errs().resetColor();
    break;

  case Level::Debug:
    llvm::errs().changeColor(llvm::raw_ostream::MAGENTA);
    llvm::errs() << "Debug: ";
    llvm::errs().resetColor();
    break;
  }

  llvm::errs() << Message << "\n";
}
} // namespace detail

template <typename... T> void log(const char *Fmt, T &&...Vals) {
  detail::logImpl(detail::Level::Info,
                  llvm::formatv(Fmt, std::forward<T>(Vals)...));
}

template <typename... T> void log_if(bool Cond, const char *Fmt, T &&...Vals) {
  if (Cond)
    log(Fmt, std::forward<T>(Vals)...);
}

template <typename... T> void elog(const char *Fmt, T &&...Vals) {
  detail::logImpl(detail::Level::Error,
                  llvm::formatv(Fmt, std::forward<T>(Vals)...));
}
template <typename... T> void elog_if(bool Cond, const char *Fmt, T &&...Vals) {
  if (Cond)
    elog(Fmt, std::forward<T>(Vals)...);
}

template <typename... T> void wlog(const char *Fmt, T &&...Vals) {
  detail::logImpl(detail::Level::Warning,
                  llvm::formatv(Fmt, std::forward<T>(Vals)...));
}
template <typename... T> void wlog_if(bool Cond, const char *Fmt, T &&...Vals) {
  if (Cond)
    wlog(Fmt, std::forward<T>(Vals)...);
}

template <typename... T> void dlog(const char *Fmt, T &&...Vals) {
  detail::logImpl(detail::Level::Debug,
                  llvm::formatv(Fmt, std::forward<T>(Vals)...));
}

template <typename... T> void dlog_if(bool Cond, const char *Fmt, T &&...Vals) {
  if (Cond)
    dlog(Fmt, std::forward<T>(Vals)...);
}

} // namespace converter

#endif
