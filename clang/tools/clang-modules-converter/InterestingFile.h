//===- InterestingFile.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_TOOLS_MODULES_CONVERTER_INTERESTING_FILE_H
#define CLANG_TOOLS_MODULES_CONVERTER_INTERESTING_FILE_H

#include "Log.h"

#include "clang/Tooling/CompilationDatabase.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/iterator.h"

#include <string>

namespace converter {

class ConverterConfig;

class InterestingFileManager;

struct Included {
  // The canonical absolute path of the file being included.
  std::string FileName;
  // The directly included text. e.g., if we see `#include "foo/../bar.h"` in
  // the file, we will record "\"foo/../bar.h\"" here.
  std::string IncludedText;

  // Whether the file name was enclosed in angle brackets;
  // otherwise, it was enclosed in quotes.
  bool IsAngle;
};

// A use of macro detected during processing.
struct MacroUse {
  std::string MacroName;

  // The file defines the macro in canonical absolute path.
  std::string DefFileName;

  // The point of loc of using the the macro. Generated from
  // `clang::SourceLoction::printToString()`.
  std::string UseLoc;

  // The point of loc of defining the the macro. Generated from
  // `clang::SourceLoction::printToString()`.
  std::string DefLoc;
};

struct PreambleOffsets {
  // The offset of start and end for the preamble section in the file.
  // The preamble here means the section of includings and other
  // preprocessor directives. But `#pragma once` and guard-if
  // should not be counted here.
  unsigned OffsetOfPreambleStart = 0, OffsetOfPreambleEnd = 0;

  // The offsets of start, end of the definition of the controlling macro,
  // and the offset for the '#endif' directive for the controlling macro.
  // All of them should be 0 if controlling macro doesn't exist
  // in the corresponding file.
  std::optional<unsigned> OffsetOfControllingMacroDefStart,
      OffsetOfControllingMacroDefEnd, OffsetOfControllingMacroEnd;
};

struct ProcessedInfo {
  PreambleOffsets PreambleInfo;

  // The module name of directly imported in this file.
  llvm::SmallVector<std::string> DirectlyImported;
  // The directly includes in the preamble
  llvm::SmallVector<Included> DirectlyIncluded;

  // The directly used macros in the body of the file.
  llvm::SmallVector<MacroUse> DirectlyUsedMacros;

  // If we detected the modules controlling macro during the processing.
  // If yes, maybe we need to skip rewriting the file.
  bool ModulesControllingMacroDetected = false;

  // Whether the ProcessedInfo are successfully processed.
  bool ProcessSuccessed;

  ProcessedInfo() = default;
  ProcessedInfo(ProcessedInfo &&Info)
      : PreambleInfo(std::move(Info.PreambleInfo)),
        DirectlyImported(std::move(Info.DirectlyImported)),
        DirectlyIncluded(std::move(Info.DirectlyIncluded)),
        DirectlyUsedMacros(std::move(Info.DirectlyUsedMacros)),
        ModulesControllingMacroDetected(Info.ModulesControllingMacroDetected),
        ProcessSuccessed(Info.ProcessSuccessed) {}

  static ProcessedInfo getFailedInfo() {
    ProcessedInfo Info;
    Info.ProcessSuccessed = false;
    return Info;
  }
};

// Represents a file, either headers or a source file.
class InterestingFileInfo {
  // The name of the file on disk.
  std::string CanonicalFileName;

  // The name of the module that this file belongs to or should belong
  // to.
  std::string ModuleName;

  // Whether or not this file is a header.
  bool IsHeader;

  // Only meaningful for headers.
  // Records all the canonical name of users that is not interesting.
  llvm::StringSet<> NonInterestingUsers;

  // The information only available after being processed.
  std::optional<ProcessedInfo> Info;

  // The directly included InterestingFileInfo*. This makes it easier for the
  // rewriter to handle infomations.
  // This is computed by InterestingFileManager, after processed all of the
  // headers.
  llvm::SmallVector<InterestingFileInfo *> DirectlyIncludedInterestingInfo;
  // The directly included non interesting files. This is computed by
  // InterestingFileManager, after processed all of the headers.
  llvm::SmallVector<Included> DirectlyIncludedNonInterestingInfo;

  InterestingFileInfo(llvm::StringRef CanonicalName, llvm::StringRef ModuleName,
                      bool IsHeader)
      : CanonicalFileName(CanonicalName.str()), ModuleName(ModuleName.str()),
        IsHeader(IsHeader) {}

  friend class InterestingFileManager;

public:
  bool isHeader() const { return IsHeader; }

  bool isProcessed() const { return (bool)Info; }

  bool isSuccessfulllyProcessed() const {
    return Info && Info->ProcessSuccessed;
  }

  llvm::StringRef getName() const { return CanonicalFileName; }
  llvm::StringRef getModuleName() const { return ModuleName; }

  unsigned getOffsetOfPreambleStart() const {
    if (!Info) {
      dlog("We are querying preamble start info without processing the file.");
      return 0;
    }
    return Info->PreambleInfo.OffsetOfPreambleStart;
  }

  unsigned getOffsetOfPreambleEnd() const {
    if (!Info) {
      dlog("We are querying preamble end info without processing the file.");
      return 0;
    }
    return Info->PreambleInfo.OffsetOfPreambleEnd;
  }

  std::optional<unsigned> getOffsetOfControllingMacroDefStart() const {
    if (!Info) {
      dlog("We are querying controlling macro def start info without "
           "processing the file.");
      return 0;
    }
    return Info->PreambleInfo.OffsetOfControllingMacroDefStart;
  }

  std::optional<unsigned> getOffsetOfControllingMacroDefEnd() const {
    if (!Info) {
      dlog("We are querying controlling macro def end info without processing "
           "the file.");
      return 0;
    }
    return Info->PreambleInfo.OffsetOfControllingMacroDefEnd;
  }

  std::optional<unsigned> getOffsetOfControllingMacroEnd() const {
    if (!Info) {
      dlog("We are querying controlling macro end info without processing the "
           "file.");
      return 0;
    }
    return Info->PreambleInfo.OffsetOfControllingMacroEnd;
  }

  bool hasUninterestingUsers() const { return !NonInterestingUsers.empty(); }
  // Get any uninteresting user. Used for diagnostics.
  llvm::StringRef getAnUninterestingUser() const {
    if (!hasUninterestingUsers())
      return "";

    return NonInterestingUsers.begin()->getKey();
  }

  const llvm::SmallVector<InterestingFileInfo *> &
  getDirectlyIncludedInterestingInfo() const {
    return DirectlyIncludedInterestingInfo;
  }

  const llvm::SmallVector<Included> &
  getDirectlyIncludedNonInterestingInfo() const {
    return DirectlyIncludedNonInterestingInfo;
  }

  const llvm::SmallVector<MacroUse> &getDirectlyMacroUses() const {
    if (!Info) {
      dlog("We are querying macro uses without processing the "
           "file.");
      static llvm::SmallVector<MacroUse> Empty;
      return Empty;
    }
    return Info->DirectlyUsedMacros;
  }

  bool hasModulesControllingMacroAlready() const {
    if (!Info) {
      dlog("We are querying modules controlling info without processing the "
           "file.");
      return false;
    }
    return Info->ModulesControllingMacroDetected;
  }

  const llvm::SmallVector<std::string> &getDirectlyImported() {
    if (!Info) {
      dlog("We are querying imports information without processing the "
           "file.");
      static llvm::SmallVector<std::string> Empty;
      return Empty;
    }
    return Info->DirectlyImported;
  }
};

struct NonInterestingFileInfo {
  // The set of all headers included by this file.
  // Used to help the diagnostic for macros.
  llvm::StringSet<> AllIncludes;
};

class InterestingFileManager {
  using InterestingFilesTy = llvm::StringMap<InterestingFileInfo>;
  llvm::StringMap<InterestingFileInfo> InterestingFiles;

  llvm::StringMap<NonInterestingFileInfo> NonInterestingFiles;

  const ConverterConfig &Config;

  // We should only get InterestingFileManager from
  // `InterestingFileManager::CalculateInterestingFiles`.
  InterestingFileManager(const ConverterConfig &Config) : Config(Config) {}

  void addHeaders(llvm::StringRef ModuleName, const llvm::StringSet<> &Headers);

  void addSrcs(llvm::StringRef ModuleName, const llvm::StringSet<> &Srcs);

  void processWithCompilationDatabase(
      clang::tooling::CompilationDatabase *,
      llvm::StringMap<clang::tooling::CompileCommand> &FoundNonModulesHeaders);
  /// Process the specified file with the commands in \param Cmd.
  ///
  /// \param AllIncludedFiles. If set, we will assign the canonical names
  /// of all included headers to it.
  ProcessedInfo
  preprocessFile(llvm::StringRef FileName, clang::tooling::CompileCommand Cmd,
                 llvm::SmallVector<std::string> *AllIncludedFiles = nullptr);

  void setProcessedInfo(
      InterestingFileInfo &FI, ProcessedInfo &&Info,
      clang::tooling::CompileCommand Command,
      llvm::StringMap<clang::tooling::CompileCommand> &FoundNonModulesHeaders);

  class Iterator {
    InterestingFilesTy::iterator underlying;

  public:
    using value_type = InterestingFileInfo &;
    using reference = value_type &;
    using pointer = value_type &;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;

    Iterator(InterestingFilesTy::iterator I) : underlying(I) {}

    reference operator*() const { return underlying->getValue(); }
    value_type &operator->() const { return underlying->getValue(); }

    Iterator &operator++() {
      underlying++;
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    friend bool operator==(Iterator x, Iterator y) {
      return x.underlying == y.underlying;
    }

    friend bool operator!=(Iterator x, Iterator y) {
      return x.underlying != y.underlying;
    }
  };

public:
  static InterestingFileManager
  calculateInterestingFiles(const ConverterConfig &Config);

  // Implemented in ProcessInterestingFiles.cpp.
  void processInterestingFiles();

  InterestingFileInfo *getInterestingFileInfo(llvm::StringRef Filename);

  Iterator begin() { return Iterator(InterestingFiles.begin()); }

  Iterator end() { return Iterator(InterestingFiles.end()); }

  bool
  isFileIncludedByNonInterestingFile(llvm::StringRef NonInterestingFileName,
                                     llvm::StringRef QueryFile) const;
};

// Implemented in Rewrtier.cpp.
void rewriteInterestingFile(InterestingFileManager &InterestingFiles,
                            const ConverterConfig &Config);
} // namespace converter

#endif
