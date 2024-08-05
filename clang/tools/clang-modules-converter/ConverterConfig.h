//===- ConverterConfig.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_TOOLS_MODULES_CONVERTER_CONVERTER_CONFIG_H
#define CLANG_TOOLS_MODULES_CONVERTER_CONVERTER_CONFIG_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"

#include <memory>
#include <optional>
#include <string>

namespace clang {
namespace tooling {
class CompilationDatabase;
}
} // namespace clang

namespace converter {

class Included;

enum class ConvertingMode {
  Invalid,

  // The header based project will still be header based,
  // but it provide the modules by wrapping headers.
  HeaderWrapper,

  // The header based project will be converted into a module based
  // project. And these headers will be converted into primary module
  // units. It'll be the responsibility of the user to make these
  // module units as private in build system's scripts.
  RewriteHeadersToModules,

  // The header based project will be converted into a module based
  // project. And these headers will be converted into partition units.
  RewriteHeadersToPartitionUnits,
};

// For better readability to split PathRegexPatterns and PathWildcardPatterns.
// We use PathWildcardPatterns to search files but PathRegexPatterns is only
// used to match a given file path.
using PathPatterns = llvm::SmallVector<std::string>;
using PathRegexPatterns = PathPatterns;
using PathWildcardPatterns = PathPatterns;

struct ThirdPartyModuleConfig {
  std::string Name;

  PathRegexPatterns Headers;
};

struct ModuleConfig {
  std::string Name;

  std::string Path;

  PathWildcardPatterns Headers;
  PathWildcardPatterns ExcludedHeaders;
  PathWildcardPatterns Srcs;
  PathWildcardPatterns ExcludedSrcs;

  // The prefix map for generated module units.
  // For example, if we're going to genrate a module unit from
  //
  //    <root-dir>/include/a.h
  //
  // By default, we going to generate the module units in:
  //
  //    <root-dir>/include/a.cppm
  //
  // if we want to generate the module units in other path like
  // 'module', we can map the prefix 'include' to 'module'. Then
  // the generated module unit will be in:
  //
  //    <root-dir>/module/a.cppm
  std::vector<std::pair<std::string, std::string>>
      PrefixMapForGeneratedModuleUnits;
};

class ConverterConfig {
public:
  using ModulesCondfigTy = llvm::SmallVector<ModuleConfig, 16>;

private:
  // The root of the project to be converting.
  std::string RootDir;

  llvm::SmallVector<ThirdPartyModuleConfig> ThirdPartyModulesConfigs;
  ModulesCondfigTy ModulesConfig;

  PathWildcardPatterns SrcsToRewrite;
  PathWildcardPatterns SrcsExcludedToRewrite;

  // Controlling Macro to show the modules mode is enabled.
  std::string ControllingMacro;

  // Whether or not to remain headers by default.
  bool RemainHeaders = true;

  // Whether or not to keep traditional (non-modules) ABI.
  bool KeepTraditionalABI = true;

  ConvertingMode Mode;

  std::unique_ptr<clang::tooling::CompilationDatabase> CDB;

  // In case we failed to find command line in CompilationDatabase,
  // use the default command line.
  std::vector<std::string> DefaultCommandLine;

  // std::nullopt implies user's didn't specify it.
  // We didn't provide a default value for this since we think users
  // should know about it. For this one, we shouldn't make silent decision
  // for users.
  std::optional<bool> IsStdModuleAvailable;

  // The path of std module path to be generated. Only meaningful with
  // IsStdModuleAvailable==true. Defaults to <RootDir>/std.cppm
  std::string StdModulePath;

  // The std header we met during the process.
  mutable llvm::StringSet<> UsedStdHeaders;

  // ConverterConfig should only be created by LoadConfig.
  ConverterConfig() = default;

  void loadDefaultValues(llvm::StringRef ConfigFile,
                         llvm::StringRef CompilationDatabasePath);

  bool validateConfig();

public:
  static std::optional<ConverterConfig> LoadConfig(llvm::StringRef ConfigFile);

  clang::tooling::CompilationDatabase *getCompilationDatabase() const {
    return CDB.get();
  }

  const ModulesCondfigTy &getModulesConfig() const { return ModulesConfig; }

  llvm::StringRef getRootDir() const { return RootDir; }

  llvm::StringRef getControllingMacro() const { return ControllingMacro; }

  const PathWildcardPatterns &getSrcsToRewrite() const { return SrcsToRewrite; }
  const PathWildcardPatterns &getSrcsExcludedToRewrite() const {
    return SrcsExcludedToRewrite;
  }

  const std::vector<std::string> &getDefaultCommandLine() const {
    return DefaultCommandLine;
  }

  bool isRemainHeaders() const { return RemainHeaders; }
  bool isKeepTraditionalABI() const { return KeepTraditionalABI; }

  bool isHeaderWrapper() const { return Mode == ConvertingMode::HeaderWrapper; }
  bool isRewriteHeadersToModules() const {
    return Mode == ConvertingMode::RewriteHeadersToModules;
  }
  bool isRewriteHeadersToPartitionUnits() const {
    return Mode == ConvertingMode::RewriteHeadersToPartitionUnits;
  }

  // Return an empty string if Path matches no third party module config.
  llvm::StringRef getMappedThirdPartyModule(const Included &Path) const;
  bool hasThirdPartyModule(const Included &Path) const {
    return !getMappedThirdPartyModule(Path).empty();
  }

  bool isStdModuleAvailable() const {
    return IsStdModuleAvailable && *IsStdModuleAvailable;
  }

  llvm::StringRef getStdModulePath() const;

  bool isStdModuleUsed() const { return !UsedStdHeaders.empty(); }

  const llvm::StringSet<> &getStdModuleUsedHeaders() const {
    return UsedStdHeaders;
  }

  std::string getIncludesStringForStdHeaders() const;

  /// Return \param Prefix itself if it is not mapped.
  std::string
  getMappedPathForModuleUnitsOfHeaders(llvm::StringRef ModuleName,
                                       llvm::StringRef Prefix) const;
};

} // namespace converter

#endif