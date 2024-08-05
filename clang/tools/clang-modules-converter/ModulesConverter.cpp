//===- ModulesConverter.cpp -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ConverterConfig.h"
#include "InterestingFile.h"
#include "Log.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"

using namespace converter;

static llvm::cl::opt<bool> Help("h", llvm::cl::desc("Alias for -help"),
                                llvm::cl::Hidden);

static llvm::cl::OptionCategory
    ModulesConverterHelper("Modules refactor helper options");

static llvm::cl::opt<std::string>
    ConfigYaml("config", llvm::cl::Required,
               llvm::cl::desc("The config file in yaml format."),
               llvm::cl::cat(ModulesConverterHelper));

static const char *Overview =
    "A helper tool to refactor a header-based library into a module-based one.";

int main(int argc, const char **argv) {
  llvm::InitLLVM X(argc, argv);
  llvm::cl::HideUnrelatedOptions(ModulesConverterHelper);
  if (!llvm::cl::ParseCommandLineOptions(argc, argv, /*Overview=*/Overview))
    return 1;

  std::optional<ConverterConfig> Config =
      ConverterConfig::LoadConfig(ConfigYaml);
  if (!Config)
    return 1;

  InterestingFileManager InterestingFiles =
      InterestingFileManager::calculateInterestingFiles(*Config);
  InterestingFiles.processInterestingFiles();

  rewriteInterestingFile(InterestingFiles, *Config);
  return 0;
}
