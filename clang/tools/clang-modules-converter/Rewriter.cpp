//===- Rewriter.cpp ---------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ConverterConfig.h"
#include "InterestingFile.h"
#include "Log.h"
#include "StdModuleGenerator.h"

#include "clang/Rewrite/Core/RewriteBuffer.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/Twine.h"

using namespace converter;
using namespace llvm;
using namespace clang;

namespace {

class Rewriter {
  RewriteBuffer Buffer;
  unsigned BufferSize;
  StringRef FileName;
  bool ErrorHappens;

public:
  Rewriter(StringRef FileName) : FileName(FileName) {
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> BufOfError =
        llvm::MemoryBuffer::getFile(FileName, /*IsText=*/true);
    if (std::error_code EC = BufOfError.getError()) {
      elog("failed to get contents from {0} during rewriting: {1}!", FileName,
           EC.message());
      ErrorHappens = true;
      return;
    }

    StringRef Contents = (*BufOfError)->getBuffer();
    BufferSize = Contents.size();
    Buffer.Initialize(Contents);
    ErrorHappens = false;
  }
  ~Rewriter() {
    ErrorHappens = true;
    if (Buffer.size() == 0)
      return;

    std::error_code EC;
    llvm::raw_fd_ostream OS(FileName, EC);
    if (EC) {
      elog("Failed to open {0} due to {1}", FileName, EC.message());
      return;
    }

    Buffer.write(OS);
    ErrorHappens = false;
  }

  void InsertTextBefore(unsigned OrigOffset, StringRef Str) {
    if (ErrorHappens)
      return;

    Buffer.InsertTextBefore(OrigOffset, Str);
  }

  void InsertTextAfter(unsigned OrigOffset, StringRef Str) {
    if (ErrorHappens)
      return;

    Buffer.InsertTextAfter(OrigOffset, Str);
  }

  void ReplaceText(unsigned OrigOffset, unsigned OrigLength, StringRef NewStr) {
    if (ErrorHappens)
      return;

    Buffer.ReplaceText(OrigOffset, OrigLength, NewStr);
  }

  void InsertTextAtLast(StringRef Str) {
    if (ErrorHappens)
      return;

    Buffer.InsertTextBefore(BufferSize - 1, Str);
  }

  bool rewriteSuccessfully() { return !ErrorHappens; }
};

class RewriterManager {
  InterestingFileManager &InterestingFiles;
  const ConverterConfig &Config;

  bool checkPreambleInfo(const InterestingFileInfo &FInfo);

  using ModulesSet = SmallSetVector<StringRef, 8>;
  /// Format the section of imports needed in the new version of
  /// the interesting file.
  ///
  /// If \param CurrentWritingModule is set, the returned string wouldn't
  /// contain the corresponding name.
  std::string getImportsSection(const InterestingFileInfo &FInfo);
  std::string getImportsSection(const SmallVector<InterestingFileInfo *> &Files,
                                StringRef CurrentWritingModule);

  // Get the module name to appear in an import statement.
  // This primary handles the case to import a partition.
  StringRef getModuleNameForImportingHeader(const InterestingFileInfo &Header,
                                            StringRef CurrentModuleName);

  /// If \param SkipMacroUseInFiles is true, the macro defined in \param Files
  /// are ignored. This is helpful in header wrapper mode where the
  /// \param Files will be included in topological order. Then the macro
  /// defined in \param Files should be resolved.
  std::string
  getMacroWarningStr(const SmallVector<InterestingFileInfo *> &Files,
                     bool SkipMacroUseInFiles = false);
  std::string getMacroWarningStr(const InterestingFileInfo &FInfo);

  std::string getUnhandledNonInterestingIncludes(
      const SmallVector<InterestingFileInfo *> &Files);
  std::string
  getUnhandledNonInterestingIncludes(const InterestingFileInfo &FInfo);

  // Get or create a module name from a name of an interesting header.
  // Given the logical module name for the interesting header is 'M'.
  // The module name for the header should be:
  //
  //    M[Concat][HeaderModulePart]
  //
  // Where the [Concat] should be '.' if the config mode is
  // "rewrite-headers-to-module-units" or ':' if the config mode is
  // "rewrite-headers-to-partitions". If the [Concat] is
  // ':', then the generated module for the header is actually a partition
  // module interface unit for the corresponding primary module interface.
  //
  // The [HeaderModulePart] should generally be the filename of the header
  // without the suffix. But if there are duplicated header names, only the
  // first header that comes first can get the name in general form. All other
  // later coming header with duplicated filename will get the name based on
  // their path. For example, if we have two headers:
  //
  //  <root-dir>/impl/a.h
  //  <root-dir>/impl2/a.h
  //
  // And the first one comes first, then it gets the name `M.a`.  And then the
  // second one can only get a name like `M.resolved.impl2.a`.
  StringRef getOrCreateModuleNameForHeader(const InterestingFileInfo &Header);

  // Helpers for implementing getOrCreateModuleNameForHeader.
  StringMap<std::string> HeaderNameCache;
  StringSet<> GeneratedModuleNames;

  // Calculate the module unit path that the corresponding header will be
  // converted to.
  SmallString<256>
  getModuleUnitPathForHeader(const InterestingFileInfo &Header);

  // If we're not in header wrappers mode and we're not required to keep
  // the headers, remove the headers in the end.
  llvm::DenseSet<const InterestingFileInfo *> HeadersToBeRemoved;

  void removeHeaders();

  /// Get the file name to be included in \param CurrentFile for \param
  /// ToBeInclude.
  ///
  /// We need this e.g., we're going to include a file under the same directory,
  /// then generally we don't like to type the full path but only the file name.
  std::string getFileNameToInclude(StringRef CurrentFile,
                                   StringRef ToBeInclude);

public:
  RewriterManager(InterestingFileManager &InterestingFiles,
                  const ConverterConfig &Config)
      : InterestingFiles(InterestingFiles), Config(Config) {}

  ~RewriterManager() { removeHeaders(); }

  void rewriteHeaders(const InterestingFileInfo &FInfo);

  void rewriteSrcs(const InterestingFileInfo &FInfo);

  void generateModuleInterfacesAsHeaderWrapper(
      StringRef ModuleName, StringRef ModulePath,
      const SmallVector<InterestingFileInfo *> &Headers);

  // Emit the std module if it is not available.
  void emitStdModule();

  void generateModuleInterfaceForHeaders(const InterestingFileInfo &FInfo);
  void generatePrimaryModuleInterfaceFromHeaders(
      StringRef ModuleName, StringRef ModulePath,
      const SmallVector<InterestingFileInfo *> &Headers);
};

void RewriterManager::removeHeaders() {
  if (Config.isRemainHeaders())
    return;

  for (const InterestingFileInfo *File : HeadersToBeRemoved) {
    // We can't do anything if the header is used by uninteresting users.
    if (File->hasUninterestingUsers()) {
      // Just show an uninteresting users. It may be too verbose if we
      // showed all of the non-interesting users.
      log("{0} has uninteresting users. e.g., {1}", File->getName(),
          File->getAnUninterestingUser());
      continue;
    }

    llvm::sys::fs::remove(File->getName());
    log("removed {0}", File->getName());
  }
}

bool RewriterManager::checkPreambleInfo(const InterestingFileInfo &FInfo) {
  if (FInfo.getOffsetOfPreambleEnd() < FInfo.getOffsetOfPreambleStart()) {
    dlog("The offset of preamble end should not be less than the start: {0}",
         FInfo.getName());
    return false;
  }

  if (FInfo.getOffsetOfPreambleEnd() == FInfo.getOffsetOfPreambleStart()) {
    log("No preamble found in {0}. No need to rewrite", FInfo.getName());
    return false;
  }

  return true;
}

void RewriterManager::rewriteHeaders(const InterestingFileInfo &FInfo) {
  if (!checkPreambleInfo(FInfo))
    return;

  if (FInfo.hasModulesControllingMacroAlready()) {
    log("NOTE: Skip rewriting {0} due to it is already controlled by modules "
        "macro.",
        FInfo.getName());
    return;
  }

  Rewriter RWriter(FInfo.getName());
  std::string InsertBeforePreamble =
      "#ifndef " + (std::string)Config.getControllingMacro() + "\n";
  RWriter.InsertTextBefore(FInfo.getOffsetOfPreambleStart(),
                           InsertBeforePreamble);
  std::string InsertAfterPreamble =
      "#endif // " + (std::string)Config.getControllingMacro() + "\n\n";
  RWriter.InsertTextBefore(FInfo.getOffsetOfPreambleEnd(), InsertAfterPreamble);

  log_if(RWriter.rewriteSuccessfully(), "Rewrote header: {0}", FInfo.getName());
}

std::string RewriterManager::getImportsSection(
    const SmallVector<InterestingFileInfo *> &Files,
    StringRef CurrentWritingModule) {
  ModulesSet ModulesToImport;

  for (InterestingFileInfo *FInfo : Files) {
    for (InterestingFileInfo *File :
         FInfo->getDirectlyIncludedInterestingInfo()) {
      StringRef ModuleName = File->getModuleName();

      // If the headers has its own module units, consider how to import it
      // properly.
      if (!Config.isHeaderWrapper())
        ModuleName =
            getModuleNameForImportingHeader(*File, CurrentWritingModule);

      if (!ModuleName.empty())
        ModulesToImport.insert(ModuleName);
    }

    for (const Included &Inc : FInfo->getDirectlyIncludedNonInterestingInfo()) {
      StringRef ThirdPartyModuleName = Config.getMappedThirdPartyModule(Inc);
      if (ThirdPartyModuleName.empty())
        continue;

      ModulesToImport.insert(ThirdPartyModuleName);
    }

    for (StringRef ModuleName : FInfo->getDirectlyImported())
      ModulesToImport.insert(ModuleName);
  }

  std::string Imports;
  for (StringRef ModuleName : ModulesToImport)
    if (ModuleName != CurrentWritingModule) {
      Imports += "import " + (std::string)ModuleName + ";\n";
    }

  return Imports;
}

std::string
RewriterManager::getImportsSection(const InterestingFileInfo &FInfo) {
  return getImportsSection(SmallVector<InterestingFileInfo *>(
                               1, const_cast<InterestingFileInfo *>(&FInfo)),
                           FInfo.getModuleName());
}

StringRef RewriterManager::getModuleNameForImportingHeader(
    const InterestingFileInfo &Header, StringRef CurrentModuleName) {
  // If we're going to import the module with a different logical module, return
  // the logical module name directly instead of the actual module name.
  if (!CurrentModuleName.empty() && CurrentModuleName != Header.getModuleName())
    return Header.getModuleName();

  StringRef ModuleName = getOrCreateModuleNameForHeader(Header);

  // Handle partitions.
  if (size_t Idx = ModuleName.find(':'); Idx != StringRef::npos) {
    dlog_if(ModuleName.split(':').first != CurrentModuleName,
            "We can only import a partition in the corresponding primary "
            "module interface unit.");
    StringRef PrimaryModuleName = ModuleName.split(':').first;
    // If we're importing a partition within the same module, remove the primary
    // module interface part.
    if (PrimaryModuleName == CurrentModuleName)
      ModuleName = ModuleName.substr(Idx);
    else
      // Otherwise, if other TU require need the partition, import the
      // primary module interface.
      ModuleName = PrimaryModuleName;
  }

  return ModuleName;
}

void RewriterManager::rewriteSrcs(const InterestingFileInfo &FInfo) {
  if (!checkPreambleInfo(FInfo))
    return;

  dlog_if(
      FInfo.isHeader(),
      "rewriteSrcs shouldn't be called with headers. Now it is called with {0}",
      FInfo.getName());
  wlog_if((bool)FInfo.getOffsetOfControllingMacroDefStart(),
          "We don't expect controlling macro in srcs: {0}", FInfo.getName());

  Rewriter RWriter(FInfo.getName());

  std::string InsertBeforePreamble;
  llvm::raw_string_ostream OS(InsertBeforePreamble);

  OS << getUnhandledNonInterestingIncludes(FInfo) << getMacroWarningStr(FInfo);

  // If this src should be in a module, then makes this src
  // as an implementation module unit.
  //
  // FIXME: Should we check the case that this file is implementation
  // module unit?
  if (!FInfo.getModuleName().empty()) {
    if (!InsertBeforePreamble.empty())
      InsertBeforePreamble = "module;\n" + InsertBeforePreamble;

    OS << "module " << FInfo.getModuleName() << ";\n";
  }

  RWriter.InsertTextBefore(FInfo.getOffsetOfPreambleStart(),
                           InsertBeforePreamble);
  RWriter.ReplaceText(FInfo.getOffsetOfPreambleStart(),
                      FInfo.getOffsetOfPreambleEnd() -
                          FInfo.getOffsetOfPreambleStart(),
                      getImportsSection(FInfo));

  if (!FInfo.getModuleName().empty() && Config.isKeepTraditionalABI()) {
    RWriter.InsertTextAfter(FInfo.getOffsetOfPreambleEnd(),
                            "extern \"C++\" {\n");
    RWriter.InsertTextAtLast("\n} // extern \"C++\"");
  }

  log_if(RWriter.rewriteSuccessfully(), "Rewrote srcs: {0}", FInfo.getName());
}

std::string RewriterManager::getMacroWarningStr(
    const SmallVector<InterestingFileInfo *> &Files, bool SkipMacroUseInFiles) {
  std::string WarningStr = "// There unhandled macro uses found in the body:\n";

  StringSet<> WarnedMacros;
  StringSet<> SkippedMacros;

  // The set of headers which wouldn't be converted modules.
  StringSet<> NonModulesHeaders;

  for (InterestingFileInfo *File : Files)
    for (const Included &Inc : File->getDirectlyIncludedNonInterestingInfo())
      if (!Config.hasThirdPartyModule(Inc))
        NonModulesHeaders.insert(Inc.FileName);

  for (InterestingFileInfo *File : Files) {
    for (const MacroUse &MU : File->getDirectlyMacroUses()) {
      if (WarnedMacros.count(MU.MacroName) || SkippedMacros.count(MU.MacroName))
        continue;

      if (SkipMacroUseInFiles &&
          llvm::any_of(Files, [&MU](InterestingFileInfo *File) {
            return MU.DefFileName == File->getName();
          })) {
        SkippedMacros.insert(MU.MacroName);
        continue;
      }

      // Since we're going to include the non interesting file which is not
      // mapped to third party modules, and if the file defining the macro is
      // included nby the non
      if (llvm::any_of(NonModulesHeaders, [this, &MU](auto &StringSetIter) {
            if (StringSetIter.getKey() == MU.DefFileName)
              return true;
            return InterestingFiles.isFileIncludedByNonInterestingFile(
                StringSetIter.getKey(), MU.DefFileName);
          })) {
        SkippedMacros.insert(MU.MacroName);
        continue;
      }

      llvm::raw_string_ostream OS(WarningStr);
      OS << "//\t'" << MU.MacroName << "' defined in " << MU.DefLoc << "\n";
      WarnedMacros.insert(MU.MacroName);
    }
  }

  if (WarnedMacros.empty())
    return "";

  return WarningStr;
}
std::string
RewriterManager::getMacroWarningStr(const InterestingFileInfo &FInfo) {
  return getMacroWarningStr(SmallVector<InterestingFileInfo *>(
      1, const_cast<InterestingFileInfo *>(&FInfo)));
}

std::string RewriterManager::getUnhandledNonInterestingIncludes(
    const SmallVector<InterestingFileInfo *> &Files) {
  std::string Ret;
  llvm::raw_string_ostream OS(Ret);

  OS << R"cpp(// WARNING: Detected unhandled non interesting includes.
// It is not suggested mix includes and imports from the compiler's
// perspective. Since it may introduce redeclarations within different
// translation units and the compiler is not able to handle such patterns
// efficiently.
//
// See https://clang.llvm.org/docs/StandardCPlusPlusModules.html#performance-tips
)cpp";

  StringSet<> Handled;

  for (InterestingFileInfo *FInfo : Files) {
    for (const Included &Inc : FInfo->getDirectlyIncludedNonInterestingInfo()) {
      StringRef ThirdPartyModuleName = Config.getMappedThirdPartyModule(Inc);
      if (!ThirdPartyModuleName.empty())
        continue;

      if (Handled.count(Inc.IncludedText))
        continue;

      OS << "#include ";
      if (Inc.IsAngle)
        OS << "<";
      else
        OS << "\"";

      OS << Inc.IncludedText;

      if (Inc.IsAngle)
        OS << ">\n";
      else
        OS << "\"\n";

      Handled.insert(Inc.IncludedText);
    }
  }

  if (Handled.empty())
    return "";

  return Ret;
}
std::string RewriterManager::getUnhandledNonInterestingIncludes(
    const InterestingFileInfo &FInfo) {
  return getUnhandledNonInterestingIncludes(SmallVector<InterestingFileInfo *>(
      1, const_cast<InterestingFileInfo *>(&FInfo)));
}

std::string getRelativePath(StringRef AbsPath, StringRef RootDir) {
  if (!llvm::sys::path::is_absolute(AbsPath)) {
    dlog("incorrect argument {0} for getRelativePath; it should be an absolute "
         "path!",
         AbsPath);
    return "";
  }

  llvm::SmallString<256> Path(AbsPath);
  llvm::sys::path::replace_path_prefix(Path, RootDir, "");
  return llvm::sys::path::relative_path(Path).str();
}

std::string RewriterManager::getFileNameToInclude(StringRef CurrentFile,
                                                  StringRef ToBeInclude) {
  SmallString<256> CurrentFileDirName(CurrentFile);
  llvm::sys::path::remove_filename(CurrentFileDirName);

  SmallString<256> IncludedDirName(ToBeInclude);
  llvm::sys::path::remove_filename(IncludedDirName);

  // If they are in the same directory, include the name of the file direcrtly.
  if (CurrentFileDirName == IncludedDirName)
    return llvm::sys::path::filename(ToBeInclude).str();

  // If the file to be included lives in a subdir of the current file, we can
  // return a relative path to the current file.
  if (IncludedDirName.starts_with(CurrentFileDirName))
    return getRelativePath(ToBeInclude, CurrentFileDirName);

  // TODO: We can offer a configurable map for the users to adjust the prefixes.

  // Otherwise, the case is complex. Return the relative path to root dir.
  return getRelativePath(ToBeInclude, Config.getRootDir());
}

void RewriterManager::generateModuleInterfacesAsHeaderWrapper(
    StringRef ModuleName, StringRef ModulePath,
    const SmallVector<InterestingFileInfo *> &Headers) {
  SmallString<256> AbsModulePath(Config.getRootDir());
  llvm::sys::path::append(AbsModulePath, ModulePath);

  wlog_if(llvm::sys::fs::exists(AbsModulePath),
          "{0} already exists, it will be overriden.", AbsModulePath);

  std::error_code EC;
  llvm::raw_fd_ostream OS(AbsModulePath, EC);
  if (EC) {
    elog("Failed to open {0} due to {1}", AbsModulePath, EC.message());
    return;
  }

  OS << getMacroWarningStr(Headers, /*SkipMacroUseInFiles=*/true);

  std::string NonInterestingIncludes =
      getUnhandledNonInterestingIncludes(Headers);
  if (!NonInterestingIncludes.empty())
    OS << "module;\n" << NonInterestingIncludes << "\n";

  OS << "export module " << ModuleName << ";\n";

  OS << getImportsSection(Headers, ModuleName);

  OS << "#define " << Config.getControllingMacro() << "\n";
  OS << "export extern \"C++\" {\n";
  for (InterestingFileInfo *Header : Headers) {
    // Always emit 2 spaces as indents, users can fix this by clang-format.
    OS << "  #include \""
       << getFileNameToInclude(AbsModulePath, Header->getName()) << "\"\n";
  }
  OS << "}\n";

  log("Generated module interface {0} at {1}", ModuleName, AbsModulePath);
}

void topologicalSort(SmallVector<InterestingFileInfo *> &Files) {
  if (Files.empty())
    return;

  SmallVector<InterestingFileInfo *> WorkList = std::move(Files);
  Files.clear();

  DenseSet<InterestingFileInfo *> Visited;
  // Since the \param Files may refer to files not in the \param Files,
  // we use this helper to filter such cases.
  DenseSet<InterestingFileInfo *> FilesSet(WorkList.begin(), WorkList.end());

  auto RecursiveTraveseHelper = [&](InterestingFileInfo *Info,
                                    auto Helper) -> void {
    Visited.insert(Info);

    for (InterestingFileInfo *IncludedInfo :
         Info->getDirectlyIncludedInterestingInfo()) {
      if (!Visited.count(IncludedInfo))
        Helper(IncludedInfo, Helper);
    }

    if (FilesSet.count(Info))
      Files.push_back(Info);
  };

  for (InterestingFileInfo *Info : WorkList) {
    if (!Visited.count(Info))
      RecursiveTraveseHelper(Info, RecursiveTraveseHelper);
  }
}

void RewriterManager::emitStdModule() {
  StringRef StdModulePath = Config.getStdModulePath();

  wlog_if(llvm::sys::fs::exists(StdModulePath),
          "{0} already exists, it will be overriden.", StdModulePath);

  std::error_code EC;
  llvm::raw_fd_ostream OS(StdModulePath, EC);
  if (EC) {
    elog("Failed to open {0} due to {1}", StdModulePath, EC.message());
    return;
  }

  OS << R"cpp(// FIXME: This is a workaround implementation for std module
// to ease the use of modules. Please replace this with the official std module
// when that is available.
)cpp";

  OS << "module;\n";

  OS << Config.getIncludesStringForStdHeaders();

  OS << R"cpp(#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-module-identifier"
#endif

export module std;

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
)cpp";

  GenerateStdModuleBodyOnNeed(Config.getStdModuleUsedHeaders(), OS);

  log("Generated the mock std module at {0}", StdModulePath);
}

StringRef RewriterManager::getOrCreateModuleNameForHeader(
    const InterestingFileInfo &FInfo) {
  dlog_if(!FInfo.isHeader(),
          "We should only call getOrCreateModuleNameForHeader for headers but "
          "now we're calling {0}",
          FInfo.getName());
  if (Config.isHeaderWrapper()) {
    dlog("getOrCreateModuleNameForHeader shouldn't be called within header "
         "wrapper mode");
    return "";
  }

  auto Iter = HeaderNameCache.find(FInfo.getName());
  if (Iter != HeaderNameCache.end())
    return Iter->getValue();

  std::string ModuleNameForHeaderCand;
  llvm::raw_string_ostream OS(ModuleNameForHeaderCand);
  OS << FInfo.getModuleName();
  if (Config.isRewriteHeadersToModules())
    OS << ".";
  else if (Config.isRewriteHeadersToPartitionUnits())
    OS << ":";
  else
    dlog("Unimgaged mode?");

  StringRef Filename =
      llvm::sys::path::filename(FInfo.getName()).rsplit('.').first;
  std::string SimpleModuleName = ModuleNameForHeaderCand + Filename.str();

  // This is the first one for the name.
  if (!GeneratedModuleNames.count(SimpleModuleName)) {
    GeneratedModuleNames.insert(SimpleModuleName);
    auto Iter =
        HeaderNameCache.insert({FInfo.getName(), SimpleModuleName}).first;
    return Iter->getValue();
  }

  // Then we have resolve it.
  OS << ".resolved";

  std::string RelativePath =
      getRelativePath(FInfo.getName(), Config.getRootDir());
  SmallVector<StringRef> Parts;
  // Remove the suffix at first
  StringRef(RelativePath)
      .rsplit('.')
      .first.split(Parts, llvm::sys::path::get_separator());

  for (StringRef Part : Parts)
    OS << "." << Part;

  dlog_if(GeneratedModuleNames.count(ModuleNameForHeaderCand),
          "We shouldn't resolve two different headers to the same name {0}. "
          "The resolve algorithm need to be improved.",
          FInfo.getName());

  GeneratedModuleNames.insert(ModuleNameForHeaderCand);
  return HeaderNameCache.insert({FInfo.getName(), ModuleNameForHeaderCand})
      .first->getValue();
}

SmallString<256>
RewriterManager::getModuleUnitPathForHeader(const InterestingFileInfo &Header) {
  SmallString<256> AbsModulePath(Config.getRootDir());

  std::string RelativePath =
      getRelativePath(Header.getName(), Config.getRootDir());
  std::string MappedPath = Config.getMappedPathForModuleUnitsOfHeaders(
      Header.getModuleName(), RelativePath);
  llvm::sys::path::append(AbsModulePath, MappedPath);
  llvm::sys::path::replace_extension(AbsModulePath, "cppm");

  return AbsModulePath;
}

void RewriterManager::generateModuleInterfaceForHeaders(
    const InterestingFileInfo &FInfo) {
  SmallString<256> AbsModulePath = getModuleUnitPathForHeader(FInfo);

  wlog_if(llvm::sys::fs::exists(AbsModulePath),
          "{0} already exists, it will be overriden.", AbsModulePath);

  std::error_code EC;
  llvm::raw_fd_ostream OS(AbsModulePath, EC);
  if (EC) {
    elog("Failed to open {0} due to {1}", AbsModulePath, EC.message());
    return;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> BufOfError =
      llvm::MemoryBuffer::getFile(FInfo.getName(), /*IsText=*/true);
  if (std::error_code EC = BufOfError.getError()) {
    elog("failed to get contents from {0} during rewriting: {1}!",
         FInfo.getName(), EC.message());
    return;
  }
  StringRef HeaderBuffer = (*BufOfError)->getBuffer();

  if (std::optional<unsigned> CMDef =
          FInfo.getOffsetOfControllingMacroDefStart()) {
    OS << HeaderBuffer.slice(0, *CMDef);

    std::optional<unsigned> CMDefEnd =
        FInfo.getOffsetOfControllingMacroDefEnd();
    dlog_if(!CMDefEnd,
            "The OffsetOfControllingMacroDefStart and "
            "OffsetOfControllingMacroDefEnd of {0} are not paired.",
            FInfo.getName());
    OS << HeaderBuffer.slice(*CMDefEnd, FInfo.getOffsetOfPreambleStart());
  } else
    OS << HeaderBuffer.slice(0, FInfo.getOffsetOfPreambleStart());

  OS << getMacroWarningStr(FInfo);

  std::string NonInterestingIncludes =
      getUnhandledNonInterestingIncludes(FInfo);
  if (!NonInterestingIncludes.empty())
    OS << "module;\n" << NonInterestingIncludes << "\n";

  StringRef ModuleNameForHeader = getOrCreateModuleNameForHeader(FInfo);
  OS << "export module " << ModuleNameForHeader << ";\n";

  OS << getImportsSection(FInfo);

  if (Config.isRemainHeaders())
    OS << "#define " << Config.getControllingMacro() << "\n";

  OS << "export ";
  if (Config.isKeepTraditionalABI())
    OS << "extern \"C++\" ";
  OS << "{\n";

  if (Config.isRemainHeaders()) {
    OS << "#include \"" << getFileNameToInclude(AbsModulePath, FInfo.getName())
       << "\"\n";
  } else {
    std::optional<unsigned> EndOfBodyOffset =
        FInfo.getOffsetOfControllingMacroEnd();
    OS << HeaderBuffer.slice(FInfo.getOffsetOfPreambleEnd(),
                             EndOfBodyOffset ? *EndOfBodyOffset
                                             : HeaderBuffer.size());
  }

  OS << "}\n";

  HeadersToBeRemoved.insert(&FInfo);

  log("Generated module interface '{0}' for header {1} at {2}",
      ModuleNameForHeader, FInfo.getName(), AbsModulePath);
}

void RewriterManager::generatePrimaryModuleInterfaceFromHeaders(
    StringRef ModuleName, StringRef ModulePath,
    const SmallVector<InterestingFileInfo *> &Headers) {
  SmallString<256> AbsModulePath(Config.getRootDir());
  llvm::sys::path::append(AbsModulePath, ModulePath);

  wlog_if(llvm::sys::fs::exists(AbsModulePath),
          "{0} already exists, it will be overriden.", AbsModulePath);

  std::error_code EC;
  llvm::raw_fd_ostream OS(AbsModulePath, EC);
  if (EC) {
    elog("Failed to open {0} due to {1}", AbsModulePath, EC.message());
    return;
  }

  OS << "export module " << ModuleName << ";\n";
  for (InterestingFileInfo *Header : Headers) {
    OS << "export import "
       << getModuleNameForImportingHeader(*Header, ModuleName) << ";\n";
  }

  log("Generated module interface '{0}' at {1}", ModuleName, AbsModulePath);
}
} // namespace

void converter::rewriteInterestingFile(InterestingFileManager &InterestingFiles,
                                       const ConverterConfig &Config) {
  RewriterManager RManager(InterestingFiles, Config);

  // Map module name to interesting headers, which will be included in a
  // generated module interface.
  StringMap<SmallVector<InterestingFileInfo *>> InterestingHeaderMap;
  for (InterestingFileInfo &FI : InterestingFiles) {
    if (!FI.isSuccessfulllyProcessed()) {
      wlog("Skip {0} due to it is not successfully processed.", FI.getName());
      continue;
    }
    if (FI.isHeader()) {
      if (Config.isRemainHeaders() || FI.hasUninterestingUsers()) {
        if (FI.getModuleName().empty()) {
          elog("All interesting headers should be in a module: {0}",
               FI.getName());
          continue;
        }
        RManager.rewriteHeaders(FI);
      }
      if (!Config.isHeaderWrapper())
        RManager.generateModuleInterfaceForHeaders(FI);
    }

    if (!FI.isHeader())
      RManager.rewriteSrcs(FI);

    if (FI.isHeader())
      InterestingHeaderMap[FI.getModuleName()].push_back(&FI);
  }

  if (Config.isHeaderWrapper()) {
    for (const ModuleConfig &MC : Config.getModulesConfig()) {
      topologicalSort(InterestingHeaderMap[MC.Name]);
      RManager.generateModuleInterfacesAsHeaderWrapper(
          MC.Name, MC.Path, InterestingHeaderMap[MC.Name]);
    }
  } else {
    for (const ModuleConfig &MC : Config.getModulesConfig()) {
      RManager.generatePrimaryModuleInterfaceFromHeaders(
          MC.Name, MC.Path, InterestingHeaderMap[MC.Name]);
    }
  }

  // If the std module is not available and the std module are needed,
  // let's try to emit a std module to workaround it.
  if (Config.isStdModuleUsed() && !Config.isStdModuleAvailable()) {
    RManager.emitStdModule();
  }
}
