//===- ConverterConfig.cpp -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ConverterConfig.h"
#include "InterestingFile.h"
#include "Log.h"

#include "clang/Tooling/JSONCompilationDatabase.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/YAMLParser.h"
#include <optional>

using namespace converter;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;

static bool handleString(yaml::Node *Node, const char *FieldName,
                         std::string &Ret) {
  auto *SN = dyn_cast<yaml::ScalarNode>(Node);
  if (!SN) {
    elog("{0} must be a string!", FieldName);
    return false;
  }

  SmallString<32> Storage;
  Ret = SN->getValue(Storage).str();
  return true;
}

static bool handlePathPatterns(yaml::Node *Node, const char *FieldName,
                               PathPatterns &Ret) {
  auto *SN = dyn_cast<yaml::ScalarNode>(Node);
  if (SN) {
    SmallString<32> Storage;
    Ret.push_back(SN->getValue(Storage).str());
    return true;
  }

  auto *Sequence = dyn_cast<yaml::SequenceNode>(Node);
  if (!Sequence) {
    elog("{0} should be either a string or a list of strings!", FieldName);
    return false;
  }

  for (yaml::Node &N : *Sequence) {
    auto *Item = dyn_cast<yaml::ScalarNode>(&N);
    if (!Item) {
      elog("the item of {0} must be string!", FieldName);
      return false;
    }

    SmallString<32> ItemStorage;
    StringRef ItemStr = Item->getValue(ItemStorage);
    Ret.push_back(ItemStr.str());
  }

  return true;
}

static bool handleBool(yaml::Node *Node, const char *FieldName, bool &Ret) {
  auto *SN = dyn_cast<yaml::ScalarNode>(Node);
  if (!SN) {
    elog("{0} must be a string!", FieldName);
    return false;
  }

  SmallString<32> Storage;
  StringRef RawValue = SN->getValue(Storage);

  enum BoolValue { Invalid, True, False };
  BoolValue V = llvm::StringSwitch<BoolValue>(RawValue)
                    .CaseLower("true", True)
                    .CaseLower("1", True)
                    .CaseLower("false", False)
                    .CaseLower("0", False)
                    .Default(Invalid);

  if (V == Invalid) {
    elog("failed to parse boolean field {0}!", FieldName);
    return false;
  }

  Ret = V == True;
  return true;
}

static bool
handleThirdPartyModulesConfig(yaml::Node *Node,
                              llvm::SmallVector<ThirdPartyModuleConfig> &Ret) {
  auto *Sequence = dyn_cast<yaml::SequenceNode>(Node);
  if (!Sequence) {
    elog("'third_party_modules' should be a sequence!");
    return false;
  }

  for (yaml::Node &N : *Sequence) {
    auto *ModuleNode = dyn_cast<yaml::MappingNode>(&N);
    if (!ModuleNode) {
      elog("the item of 'third_party_modules' must be a map!");
      return false;
    }

    Ret.push_back(ThirdPartyModuleConfig{});

    for (yaml::KeyValueNode &ModuleKV : *ModuleNode) {
      auto *ModuleKey = dyn_cast<yaml::ScalarNode>(ModuleKV.getKey());
      if (!ModuleKey) {
        elog("the key of item in 'third_party_modules' is expected to be "
             "string");
        return false;
      }

      SmallString<32> ValueStorage;
      StringRef KeyStr = ModuleKey->getValue(ValueStorage);

      if (KeyStr == "name") {
        if (!handleString(ModuleKV.getValue(), "name", Ret.back().Name))
          return false;

        continue;
      }

      if (KeyStr == "headers") {
        if (!handlePathPatterns(ModuleKV.getValue(), "headers",
                                Ret.back().Headers))
          return false;

        continue;
      }

      elog("Unknown field for third_party_modules: {0}", KeyStr);
    }
  }

  return true;
}

static bool handleModulesConfig(yaml::Node *Node,
                                ConverterConfig::ModulesCondfigTy &Ret) {
  auto *Sequence = dyn_cast<yaml::SequenceNode>(Node);
  if (!Sequence) {
    elog("'modules' should be a sequence!");
    return false;
  }

  for (yaml::Node &N : *Sequence) {
    auto *ModuleNode = dyn_cast<yaml::MappingNode>(&N);
    if (!ModuleNode) {
      elog("the item of 'modules' must be a map!");
      return false;
    }

    Ret.push_back(ModuleConfig{});

    for (yaml::KeyValueNode &ModuleKV : *ModuleNode) {
      auto *ModuleKey = dyn_cast<yaml::ScalarNode>(ModuleKV.getKey());
      if (!ModuleKey) {
        elog("the key of item in 'modules' is expected to be a string");
        return false;
      }

      SmallString<32> ValueStorage;
      StringRef KeyStr = ModuleKey->getValue(ValueStorage);

      if (KeyStr == "name") {
        if (!handleString(ModuleKV.getValue(), "name", Ret.back().Name))
          return false;

        continue;
      }

      if (KeyStr == "path") {
        if (!handleString(ModuleKV.getValue(), "path", Ret.back().Path))
          return false;

        continue;
      }

      if (KeyStr == "headers") {
        if (!handlePathPatterns(ModuleKV.getValue(), "headers",
                                Ret.back().Headers))
          return false;

        continue;
      }

      if (KeyStr == "excluded_headers") {
        if (!handlePathPatterns(ModuleKV.getValue(), "excluded_headers",
                                Ret.back().ExcludedHeaders))
          return false;

        continue;
      }

      if (KeyStr == "srcs") {
        if (!handlePathPatterns(ModuleKV.getValue(), "srcs", Ret.back().Srcs))
          return false;

        continue;
      }

      if (KeyStr == "excluded_srcs") {
        if (!handlePathPatterns(ModuleKV.getValue(), "excluded_srcs",
                                Ret.back().ExcludedSrcs))
          return false;

        continue;
      }

      if (KeyStr == "prefix_map_of_module_units_for_headers") {
        SmallVector<std::string> PrefixMaps;
        if (!handlePathPatterns(ModuleKV.getValue(),
                                "prefix_map_of_module_units_for_headers",
                                PrefixMaps))
          return false;

        for (StringRef PrefixMap : PrefixMaps) {
          if (!PrefixMap.count(":")) {
            wlog("The field of 'prefix_map_of_module_units_for_headers' must "
                 "contain a ':' to split the key and value field: {0}",
                 PrefixMap);
            continue;
          }

          auto [Key, Value] = PrefixMap.split(":");

          Ret.back().PrefixMapForGeneratedModuleUnits.push_back(
              {Key.str(), Value.str()});
        }

        continue;
      }

      elog("Unknown field for modules: {0}", KeyStr);
    }

    if (Ret.back().Name.empty()) {
      elog("Name is a required field for modules!");
      return false;
    }

    if (Ret.back().Path.empty()) {
      elog("Path is a required field for modules!");
      return false;
    }
  }

  return true;
}

// We wrap the header names inside `^` `$` pairs since we
// don't want it to mismatch other headers.
#define STD_HEADER(HEADER_NAME) "^" #HEADER_NAME "$",
static std::string BuiltinStdHeaderRegexs[] = {
#include "std_headers_list.def"
    "^__not_exist__$"};

std::optional<ConverterConfig>
ConverterConfig::LoadConfig(llvm::StringRef ConfigFile) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> FileOrErr =
      llvm::MemoryBuffer::getFile(ConfigFile, /*IsText=*/true);
  if (std::error_code EC = FileOrErr.getError()) {
    elog("Error reading file: {0}", EC.message());
    return std::nullopt;
  }

  std::unique_ptr<llvm::MemoryBuffer> Buffer = std::move(*FileOrErr);
  llvm::StringRef FileContent = Buffer->getBuffer();

  llvm::SourceMgr SMgr;
  yaml::Stream stream(FileContent, SMgr);

  yaml::document_iterator di = stream.begin();
  if (di == stream.end()) {
    elog("empty config file");
    return std::nullopt;
  }

  yaml::Node *n = di->getRoot();
  if (!n) {
    elog("failed to parse config file");
    return std::nullopt;
  }

  auto *MN = dyn_cast<yaml::MappingNode>(n);
  if (!MN) {
    elog("failed to parse config file");
    return std::nullopt;
  }

  // Load it actually after we parsed the config file since we may need the
  // information about the root dir to load it.
  std::string CDBPath;

  ConverterConfig Config;

  for (yaml::KeyValueNode &KV : *MN) {
    auto *Key = dyn_cast<yaml::ScalarNode>(KV.getKey());
    if (!Key) {
      elog("key value nodes expected from config file");
      return std::nullopt;
    }

    SmallString<32> ValueStorage;
    StringRef KeyStr = Key->getValue(ValueStorage);
    if (KeyStr == "root_dir") {
      if (!handleString(KV.getValue(), "root_dir", Config.RootDir))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "srcs_to_rewrite") {
      if (!handlePathPatterns(KV.getValue(), "srcs_to_rewrite",
                              Config.SrcsToRewrite))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "srcs_excluded_to_rewrite") {
      if (!handlePathPatterns(KV.getValue(), "srcs_excluded_to_rewrite",
                              Config.SrcsExcludedToRewrite))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "mode") {
      SmallString<32> Storage;
      auto *SN = dyn_cast<yaml::ScalarNode>(KV.getValue());
      if (!SN) {
        elog("Expect a scalar node from 'mode'");
        return std::nullopt;
      }

      ConvertingMode Mode =
          StringSwitch<ConvertingMode>(SN->getValue(Storage))
              .Case("header-wrapper", ConvertingMode::HeaderWrapper)
              .Case("rewrite-headers-to-module-units",
                    ConvertingMode::RewriteHeadersToModules)
              .Case("rewrite-headers-to-partitions",
                    ConvertingMode::RewriteHeadersToPartitionUnits)
              .Default(ConvertingMode::Invalid);

      if (Mode == ConvertingMode::Invalid) {
        elog("invalid value for 'mode'. Valid values are: 'header-wrapper', "
             "'rewrite-headers-to-module-units', "
             "'rewrite-headers-to-partitions'");
        return std::nullopt;
      }

      Config.Mode = Mode;

      continue;
    }

    if (KeyStr == "controlling_macro") {
      if (!handleString(KV.getValue(), "controlling_macro",
                        Config.ControllingMacro))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "remain_headers") {
      if (!handleBool(KV.getValue(), "remain_headers", Config.RemainHeaders))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "keep_traditional_abi") {
      if (!handleBool(KV.getValue(), "keep_traditional_abi",
                      Config.KeepTraditionalABI))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "is_std_module_available") {
      bool Value;
      if (!handleBool(KV.getValue(), "is_std_module_available", Value))
        return std::nullopt;

      Config.IsStdModuleAvailable = Value;

      continue;
    }

    if (KeyStr == "std_module_path") {
      if (!handleString(KV.getValue(), "std_module_path", Config.StdModulePath))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "third_party_modules") {
      if (!handleThirdPartyModulesConfig(KV.getValue(),
                                         Config.ThirdPartyModulesConfigs))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "modules") {
      if (!handleModulesConfig(KV.getValue(), Config.ModulesConfig))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "compilation_database") {
      if (!handleString(KV.getValue(), "compilation_database", CDBPath))
        return std::nullopt;

      continue;
    }

    if (KeyStr == "default_compile_commands") {
      std::string Cmdline;
      if (!handleString(KV.getValue(), "default_compile_commands", Cmdline))
        return std::nullopt;

      llvm::SmallVector<StringRef> Commands;
      StringRef(Cmdline).split(Commands, " ");

      for (StringRef Cmd : Commands)
        Config.DefaultCommandLine.push_back(Cmd.str());

      continue;
    }

    elog("Unknwon field {0}", KeyStr);
  }

  Config.loadDefaultValues(ConfigFile, CDBPath);

  if (!Config.validateConfig()) {
    elog("Loaded Config is not valid.");
    return std::nullopt;
  }

  return Config;
}

void ConverterConfig::loadDefaultValues(StringRef ConfigFile,
                                        StringRef CDBPath) {
  if (RootDir.empty()) {
    llvm::SmallString<256> AbsConfigFileDir(ConfigFile);

    if (!llvm::sys::path::is_absolute(ConfigFile)) {
      llvm::SmallString<256> CurrentPath;
      std::error_code ec = llvm::sys::fs::current_path(CurrentPath);
      if (ec) {
        elog("'root_dir' is not specified. Trying to get the current path but "
             "failed: {0}",
             ec.message());
      }

      llvm::sys::fs::make_absolute(CurrentPath, AbsConfigFileDir);
    }

    llvm::sys::path::remove_filename(AbsConfigFileDir);
    RootDir = (std::string)AbsConfigFileDir;
    log("'root_dir' not specified, use the path of config file as root dir: "
        "{0}",
        RootDir);
  }

  ThirdPartyModulesConfigs.push_back(ThirdPartyModuleConfig{
      "std", PathPatterns(ArrayRef<std::string>(BuiltinStdHeaderRegexs))});

  if (IsStdModuleAvailable && !*IsStdModuleAvailable) {
    if (StdModulePath.empty()) {
      SmallString<256> StdModulePathStorage(RootDir);
      llvm::sys::path::append(StdModulePathStorage, "std.cppm");
      StdModulePath = (std::string)StdModulePathStorage;
    }
  }

  // If CDBPath is not provided, we will use the default command line.
  if (CDBPath.empty())
    return;

  SmallString<256> CDBAbsPath(CDBPath);
  if (!llvm::sys::path::is_absolute(CDBPath)) {
    CDBAbsPath = getRootDir();
    llvm::sys::path::append(CDBAbsPath, CDBPath);
  }

  std::string ErrorMessage;
  CDB = tooling::JSONCompilationDatabase::loadFromFile(
      CDBAbsPath, ErrorMessage, tooling::JSONCommandLineSyntax::AutoDetect);

  if (!CDB) {
    elog("failed to load compilation database from {0} due to {1}", CDBAbsPath,
         ErrorMessage);
    return;
  }

  log("Loaded Compilation Database from {0}", CDBAbsPath);
}

bool ConverterConfig::validateConfig() {
  if (ControllingMacro.empty()) {
    elog("'controlling_macro' is required.");
    return false;
  }

  if (Mode == ConvertingMode::Invalid) {
    elog("'mode' is required.");
    return false;
  }

  if (!IsStdModuleAvailable) {
    elog("'is_std_module_available' is required.");
    return false;
  }

  if (!StdModulePath.empty() && *IsStdModuleAvailable)
    wlog("'std_module_path' is only meaningful with 'is_std_module_available' "
         "equals to true.");

  for (auto &MC : ModulesConfig) {
    if (!MC.Srcs.empty()) {
      if (Mode == ConvertingMode::HeaderWrapper) {
        wlog("Srcs of module {0} are not meaningful "
             "in HeaderWrapper mode",
             MC.Name);
      }

      if (!KeepTraditionalABI)
        wlog("It may be not good to introduce module implementation units if "
             "we keep the traditional ABI.");
    }

    if (!MC.PrefixMapForGeneratedModuleUnits.empty())
      if (Mode == ConvertingMode::HeaderWrapper)
        wlog("field 'prefix_map_of_module_units_for_headers' in module {0} are "
             "not meaningful "
             "in HeaderWrapper mode",
             MC.Name);

    if (llvm::sys::path::is_absolute(MC.Path)) {
      elog("The path {0} should be a relative path to the root dir!", MC.Path);
      return false;
    }
  }

  if (!KeepTraditionalABI && Mode == ConvertingMode::HeaderWrapper)
    wlog("'keep_traditional_abi' is not meaningful in header wrapper mode.\n");

  return true;
}

StringRef
ConverterConfig::getMappedThirdPartyModule(const Included &Path) const {
  for (const ThirdPartyModuleConfig &C : ThirdPartyModulesConfigs) {
    for (StringRef HeaderPattern : C.Headers) {
      llvm::Regex R(HeaderPattern);
      if (R.match(Path.IncludedText)) {
        if (C.Name == "std")
          UsedStdHeaders.insert(Path.IncludedText);

        return C.Name;
      }
    }
  }

  return "";
}

std::string ConverterConfig::getIncludesStringForStdHeaders() const {
  SetVector<StringRef> StdHeaders(std::begin(BuiltinStdHeaderRegexs),
                                  std::end(BuiltinStdHeaderRegexs));

  auto GetStdHeaderName = [](StringRef HeaderNameRegex) {
    StringRef Header = HeaderNameRegex;
    if (Header.starts_with("^"))
      Header = Header.drop_front(1);
    if (Header.ends_with("$"))
      Header = Header.drop_back(1);
    return Header;
  };

  // Handle user specified std headers if any.
  for (const ThirdPartyModuleConfig &C : ThirdPartyModulesConfigs) {
    if (C.Name != "std")
      continue;

    for (StringRef Header : C.Headers)
      StdHeaders.insert(Header);
  }

  std::string Ret;
  llvm::raw_string_ostream OS(Ret);
  for (StringRef Header : StdHeaders) {
    StringRef HeaderName = GetStdHeaderName(Header);
    // We'd like to generate the std module on demand.
    if (!UsedStdHeaders.count(HeaderName))
      continue;

    OS << "#include <";
    OS << HeaderName;
    OS << ">\n";
  }

  return Ret;
}

std::string
ConverterConfig::getMappedPathForModuleUnitsOfHeaders(StringRef ModuleName,
                                                      StringRef Prefix) const {
  // Perform a linear search in ModulesConfig. It might not be a problem by
  // assuming the number of modules won't be a lot.
  for (const auto &MC : ModulesConfig) {
    if (MC.Name != ModuleName)
      continue;

    for (const auto [OldPrefix, NewPrefix] :
         MC.PrefixMapForGeneratedModuleUnits) {
      if (Prefix.starts_with(OldPrefix)) {
        SmallString<256> Mapped(Prefix);
        llvm::sys::path::replace_path_prefix(Mapped, OldPrefix, NewPrefix);
        return (std::string)Mapped;
      }
    }
  }

  return Prefix.str();
}

StringRef ConverterConfig::getStdModulePath() const {
  dlog_if(isStdModuleAvailable(), "we shouldn't try to generate std module if "
                                  "std module is already available.");

  return StdModulePath;
}
