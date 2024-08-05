//===- InterestingFile.cpp --------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "InterestingFile.h"
#include "ConverterConfig.h"
#include "Log.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Regex.h"

using namespace llvm;
using namespace converter;

static void replaceAll(std::string &str, const std::string &from,
                       const std::string &to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos +=
        to.length(); // Handles case where 'to' is a substring of 'from'
  }
}

static bool matchPart(StringRef Path, StringRef Pattern) {
  std::string P = Pattern.str();

  // Replace the dots with the dots in regex.
  replaceAll(P, ".", R"cpp(\.)cpp");
  replaceAll(P, "*", ".*");

  P = "^" + P + "$";
  llvm::Regex R(P);
  return R.match(Path);
}

static bool matchRecursively(const SmallVector<StringRef> &FilePathParts,
                             const SmallVector<StringRef> &PatternParts,
                             unsigned FileIndex, unsigned PatternIndex) {
  while (FileIndex < FilePathParts.size() &&
         PatternIndex < PatternParts.size()) {
    if (PatternParts[PatternIndex] == "**") {
      PatternIndex++;
      // ** can match any thing. So if it is in the end, we can always match it.
      if (PatternIndex == PatternParts.size())
        return true;

      // Then tries to match the LHS of patterns with the path recursively.
      while (FileIndex < FilePathParts.size())
        if (matchRecursively(FilePathParts, PatternParts, FileIndex++,
                             PatternIndex))
          return true;

      return false;
    }

    if (!matchPart(FilePathParts[FileIndex], PatternParts[PatternIndex]))
      return false;

    FileIndex++;
    PatternIndex++;
  }

  return FileIndex == FilePathParts.size() &&
         PatternIndex == PatternParts.size();
}

static bool match(StringRef FilePath, StringRef WildcardPattern) {
  SmallVector<StringRef> FilePathParts;
  FilePath.split(FilePathParts, llvm::sys::path::get_separator());
  SmallVector<StringRef> PatternParts;
  WildcardPattern.split(PatternParts, llvm::sys::path::get_separator());
  return matchRecursively(FilePathParts, PatternParts, 0, 0);
}

void findAllMatchedFiles(StringRef RootDir,
                         const PathWildcardPatterns &Patterns,
                         const PathWildcardPatterns &ExcludePatterns,
                         llvm::StringSet<> &Results) {
  if (Patterns.empty())
    return;

  std::error_code EC;
  for (llvm::sys::fs::recursive_directory_iterator F(RootDir, EC), E;
       F != E && !EC; F.increment(EC)) {
    // We don't need to
    if (llvm::sys::fs::is_directory(F->path()))
      continue;

    llvm::SmallString<256> Path(F->path());
    llvm::sys::path::replace_path_prefix(Path, RootDir, "");
    StringRef RelativePath = llvm::sys::path::relative_path(Path);

    // If this file is excluded already, don't try to match.
    if (llvm::any_of(ExcludePatterns, [&](StringRef ExcludePatten) {
          return match(RelativePath, ExcludePatten);
        }))
      continue;

    if (llvm::any_of(Patterns, [&](StringRef Pattern) {
          return match(RelativePath, Pattern);
        }))
      Results.insert(F->path());
  }
}

InterestingFileManager InterestingFileManager::calculateInterestingFiles(
    const ConverterConfig &Config) {
  InterestingFileManager InterestingFiles(Config);
  for (const ModuleConfig &MC : Config.getModulesConfig()) {
    llvm::StringSet<> Headers;
    findAllMatchedFiles(Config.getRootDir(), MC.Headers, MC.ExcludedHeaders,
                        Headers);
    InterestingFiles.addHeaders(MC.Name, Headers);

    llvm::StringSet<> Srcs;
    findAllMatchedFiles(Config.getRootDir(), MC.Srcs, MC.ExcludedSrcs, Srcs);
    InterestingFiles.addSrcs(MC.Name, Srcs);
  }

  llvm::StringSet<> Srcs;
  findAllMatchedFiles(Config.getRootDir(), Config.getSrcsToRewrite(),
                      Config.getSrcsExcludedToRewrite(), Srcs);
  InterestingFiles.addSrcs(/*ModuleName=*/"", Srcs);

  return InterestingFiles;
}

void InterestingFileManager::addSrcs(StringRef ModuleName,
                                     const llvm::StringSet<> &Srcs) {
  for (auto &StrIter : Srcs)
    InterestingFiles.insert(
        {StrIter.getKey(), InterestingFileInfo(StrIter.getKey(), ModuleName,
                                               /*IsHeader=*/false)});
}

void InterestingFileManager::addHeaders(StringRef ModuleName,
                                        const llvm::StringSet<> &Headers) {
  for (auto &HeaderIter : Headers)
    InterestingFiles.insert(
        {HeaderIter.getKey(),
         InterestingFileInfo(HeaderIter.getKey(), ModuleName,
                             /*IsHeader=*/true)});
}
