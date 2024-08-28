//===- ProcessInterestingFiles.cpp ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ConverterConfig.h"
#include "InterestingFile.h"
#include "Log.h"

#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/Token.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/TargetParser/Host.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using namespace converter;

namespace {
struct ProcessedInfoProducer : public PPCallbacks {
  // The canoncial absolute name of all includes we seen during
  // the preprocessing. No matter if it is directly included or not,
  // or in preamble or not.
  SmallVector<std::string> AllIncludes;

  SmallVector<std::string> DirectlyImported;
  // Only the includes in the preamble. See the comments for ProcessedInfo.
  SmallVector<Included> DirectlyIncluded;

  SmallVector<converter::MacroUse> DirectlyUsedMacros;

  SourceManager &SMgr;

  SourceLocation PreambleEnd;

  StringRef ModulesControllingMacro;

  bool IsModulesControllingMacroDetected = false;

  ProcessedInfoProducer(SourceManager &SMgr, unsigned PreambleEndOffset,
                        StringRef ModulesControllingMacro)
      : SMgr(SMgr), ModulesControllingMacro(ModulesControllingMacro) {
    PreambleEnd = SourceLocation::getFromRawEncoding(PreambleEndOffset + 1);
  }

  bool isPreambleParsed(SourceLocation Loc) const { return Loc >= PreambleEnd; }

  void moduleImport(SourceLocation ImportLoc, ModuleIdPath Path,
                    const Module *Imported) override {
    // We don't care import in included headers.
    if (!SMgr.isWrittenInMainFile(ImportLoc))
      return;

    if (Path.empty())
      return;

    if (!Path[0].first)
      return;

    DirectlyImported.push_back(Path[0].first->getName().str());
  }

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          OptionalFileEntryRef File, StringRef SearchPath,
                          StringRef RelativePath, const Module *SuggestedModule,
                          bool ModuleImported,
                          SrcMgr::CharacteristicKind FileType) override {
    wlog_if(!File, "Failed to get the actual file for including '{0}' in {1}",
            FileName, IncludeTok.getLocation().printToString(SMgr));

    if (File)
      AllIncludes.push_back(File->getNameAsRequested().str());

    // Then, we don't care include in included headers.
    if (!SMgr.isWrittenInMainFile(HashLoc))
      return;

    std::string CanonicalName = File ? File->getNameAsRequested().str() : "";
    Included Inc{CanonicalName, FileName.str(), IsAngled};
    DirectlyIncluded.push_back(std::move(Inc));
  }

  // Try to find the modules controlling macro
  void Ifndef(SourceLocation Loc, const Token &MacroNameTok,
              const MacroDefinition &MD) override {
    if (!SMgr.isWrittenInMainFile(Loc))
      return;

    // FIXME: Is this possible?
    if (!MacroNameTok.is(tok::identifier))
      return;

    IdentifierInfo *II = MacroNameTok.getIdentifierInfo();
    if (II && II->getName() == ModulesControllingMacro)
      IsModulesControllingMacroDetected = true;
  }

  // Collect macro informations to give better diagnostics.
  void MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD,
                    SourceRange Range, const MacroArgs *Args) override {
    // We don't need builtin macros in diagnostic messages.
    MacroInfo *MI = MD.getMacroInfo();
    if (!MI || MI->isBuiltinMacro())
      return;

    SourceLocation MacroLoc = MacroNameTok.getLocation();
    // We don't care the macro use in headers.
    if (!SMgr.isWrittenInMainFile(MacroLoc))
      return;

    // It is fine if the macro expands in the preamble.
    if (!isPreambleParsed(MacroLoc))
      return;

    SourceLocation DefLoc = MI->getDefinitionLoc();
    // If the macro is defined in the main file. Then it should be fine.
    if (SMgr.isWrittenInMainFile(DefLoc))
      return;

    // FIXME: Is this possible?
    if (!MacroNameTok.is(tok::identifier))
      return;
    IdentifierInfo *II = MacroNameTok.getIdentifierInfo();

    auto PresumedDefLoc = SMgr.getPresumedLoc(DefLoc);
    // We don't care it if its def loc is invalid. This is possible if
    // the macro is predefined.
    if (PresumedDefLoc.isInvalid())
      return;

    // Filter out the predefined macros.
    if (PresumedDefLoc.getFilename() == StringRef("<built-in>"))
      return;

    // Otherwise, the macro are from the preamble.
    DirectlyUsedMacros.push_back(
        {II->getName().str(), PresumedDefLoc.getFilename(),
         MacroNameTok.getLocation().printToString(SMgr),
         DefLoc.printToString(SMgr)});
  }
};

PreambleOffsets ParsePreamble(StringRef Filename, const LangOptions &LangOpts) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> BufOfError =
      llvm::MemoryBuffer::getFile(Filename, /*IsText=*/true);
  if (std::error_code EC = BufOfError.getError()) {
    elog("failed to get contents from {0}: {1}", Filename, EC.message());
    return PreambleOffsets();
  }

  const SourceLocation::UIntTy StartOffset = 1;
  SourceLocation FileLoc = SourceLocation::getFromRawEncoding(StartOffset);
  StringRef Buffer = (*BufOfError)->getBuffer();
  Lexer TheLexer(FileLoc, LangOpts, Buffer.begin(), Buffer.begin(),
                 Buffer.end());
  TheLexer.SetCommentRetentionState(true);

  SourceLocation ControllingMacroDefStartCands;
  SourceLocation ControllingMacroDefEndCands;
  SourceLocation ControllingMacroEndCands;
  // A stack helper to find the '#endif' part for the possible controlling
  // macro. It will push preprocessor directives like `#if`, `#ifdef` and
  // `#ifndef` and pop when it sees '#endif'.
  // The ControllingMacroDefStartCands will pushed to it when
  // ControllingMacroDefStartCands is set. Given the nature of controlling
  // macro, the HashIfStack will only be empty if controlling macro deosn't
  // exist or we just saw the '#endif' part for the possible controlling macro.
  SmallVector<SourceLocation> HashIfStack;

  Token Tok;
  // Try to match the controlling macro pattern:
  //  #ifndef CONTROLLING_MACRO
  //  #define CONTROLLING_MACRO
  while (true) {
    TheLexer.LexFromRawLexer(Tok);

    if (Tok.is(tok::comment))
      continue;

    // FIXME: There are too many similar patterns here. Can we try
    // to improve that?
    if (Tok.isAtStartOfLine() && Tok.is(tok::hash)) {
      ControllingMacroDefStartCands = Tok.getLocation();
      TheLexer.LexFromRawLexer(Tok);

      if (!Tok.is(tok::raw_identifier) || Tok.needsCleaning())
        break;

      if (Tok.getRawIdentifier() != "ifndef")
        break;

      TheLexer.LexFromRawLexer(Tok);
      if (!Tok.is(tok::raw_identifier) || Tok.needsCleaning())
        break;

      StringRef ControllingMacroCands = Tok.getRawIdentifier();
      TheLexer.LexFromRawLexer(Tok);

      if (!Tok.isAtStartOfLine() || !Tok.is(tok::hash))
        break;

      TheLexer.LexFromRawLexer(Tok);
      if (!Tok.is(tok::raw_identifier) || Tok.needsCleaning())
        break;

      if (Tok.getRawIdentifier() != "define")
        break;

      TheLexer.LexFromRawLexer(Tok);
      if (!Tok.is(tok::raw_identifier) || Tok.needsCleaning())
        break;

      if (Tok.getRawIdentifier() != ControllingMacroCands)
        break;

      HashIfStack.push_back(ControllingMacroDefStartCands);
      TheLexer.LexFromRawLexer(Tok);
      ControllingMacroDefEndCands = Tok.getLocation();
      break;
    }

    break;
  }

  SourceLocation PreambleStart;

  // If we failed to find the possible controlling macro during the process,
  // use the location of the candidate as the start of the preamble.
  if (HashIfStack.empty() && ControllingMacroDefStartCands.isValid()) {
    PreambleStart = ControllingMacroDefStartCands;
    ControllingMacroDefStartCands = SourceLocation();
    // Reset the state for the lexer.
    TheLexer.seek(PreambleStart.getRawEncoding() - 1, /*IsAtStartOfLine=*/true);
    TheLexer.LexFromRawLexer(Tok);
  }

  // Find the start of the preamble.
  bool MayParsingGMF = false;
  for (; PreambleStart.isInvalid() && !Tok.is(tok::eof);
       TheLexer.LexFromRawLexer(Tok)) {
    if (Tok.is(tok::comment))
      continue;

    // We think the declaration of the global module fragment (GMF), 'module;'
    // is not part of the preamble. So we'tried to skip it.
    if (Tok.is(tok::raw_identifier) && !Tok.needsCleaning()) {
      StringRef Keyword = Tok.getRawIdentifier();
      if (Keyword == "module") {
        MayParsingGMF = true;
        continue;
      }
    }

    // We parsed a GMF declaration.
    if (Tok.is(tok::semi) && MayParsingGMF) {
      MayParsingGMF = false;
      continue;
    }

    // We meet something other than the comment, stop.
    PreambleStart = Tok.getLocation();
    break;
  }

  bool InPreprocessorDirective = false;
  // We record the location of just touched comments. So that we won't
  // get confused about the comments after preamble. Thet should be the
  // comments for the body.
  SourceLocation JustLexedCommentLocation;
  // Find the end of the preamble.
  for (; !Tok.is(tok::eof); TheLexer.LexFromRawLexer(Tok)) {
    if (InPreprocessorDirective) {
      // Ignore anything if we're still in the preprocessor's directive.
      if (!Tok.isAtStartOfLine())
        continue;

      JustLexedCommentLocation = SourceLocation();
      InPreprocessorDirective = false;
    }

    if (Tok.is(tok::comment)) {
      if (JustLexedCommentLocation.isInvalid())
        JustLexedCommentLocation = Tok.getLocation();
      continue;
    }

    if (Tok.isAtStartOfLine() && Tok.is(tok::hash)) {
      SourceLocation HashLoc = Tok.getLocation();
      InPreprocessorDirective = true;
      TheLexer.LexFromRawLexer(Tok);

      if (Tok.is(tok::raw_identifier) && !Tok.needsCleaning()) {
        StringRef Keyword = Tok.getRawIdentifier();

        if (!HashIfStack.empty() && llvm::StringSwitch<bool>(Keyword)
                                        .Case("if", true)
                                        .Case("ifdef", true)
                                        .Case("ifndef", true)
                                        .Default(false)) {
          HashIfStack.push_back(HashLoc);
        }

        if (!HashIfStack.empty() && Keyword == "endif") {
          HashIfStack.pop_back();
          if (HashIfStack.empty())
            ControllingMacroEndCands = HashLoc;
        }

        if (llvm::StringSwitch<bool>(Keyword)
                .Case("include", true)
                .Case("__include_macros", true)
                .Case("define", true)
                .Case("undef", true)
                .Case("line", true)
                .Case("error", true)
                .Case("pragma", true)
                .Case("import", true)
                .Case("include_next", true)
                .Case("warning", true)
                .Case("ident", true)
                .Case("sccs", true)
                .Case("assert", true)
                .Case("unassert", true)
                .Case("if", true)
                .Case("ifdef", true)
                .Case("ifndef", true)
                .Case("elif", true)
                .Case("elifdef", true)
                .Case("elifndef", true)
                .Case("else", true)
                .Case("endif", true)
                .Default(false))
          continue;
      }
    }

    // We treat 'import' as a preprocessor directives too.
    if (Tok.isAtStartOfLine() && Tok.is(tok::raw_identifier) &&
        !Tok.needsCleaning()) {
      InPreprocessorDirective = true;

      if (Tok.getRawIdentifier() == "import") {
        TheLexer.LexFromRawLexer(Tok);
        if (Tok.is(tok::raw_identifier) && !Tok.needsCleaning()) {
          continue;
        }
      }
    }

    // If we meet any other thing, it implies that the preamble is finished.
    break;
  }

  SourceLocation PreambleEnd = JustLexedCommentLocation.isValid()
                                   ? JustLexedCommentLocation
                                   : Tok.getLocation();

  // Try to lex the file from ControllingMacroEndCands to the end to
  // verify if the controlling macro are real controlling macro.
  // e.g., we won't see any thing meaningful after the `#endif` part.
  if (ControllingMacroEndCands.isValid()) {
    TheLexer.seek(ControllingMacroEndCands.getRawEncoding() - 1,
                  /*IsAtStartOfLine=*/true);
    for (TheLexer.LexFromRawLexer(Tok); !Tok.is(tok::eof);
         TheLexer.LexFromRawLexer(Tok)) {
      if (Tok.is(tok::comment))
        continue;

      // We meet other things than eof and comments. So we're not seeing the
      // real controlling macro.

      // Then treat the location of the "controlling macro" as the start of the
      // preamble.
      PreambleStart = ControllingMacroDefStartCands;
      ControllingMacroDefStartCands = SourceLocation();
      ControllingMacroDefEndCands = SourceLocation();
      ControllingMacroEndCands = SourceLocation();
      break;
    }
  } else if (!HashIfStack.empty()) {
    // Then we're seeing a controlling macro candidate but not the `#endif`.
    // Maybe we break during lexing the end of the preamble.
    //
    // Try to finish lexing the file to find the `#endif` for the controlling
    // macro.
    for (; !Tok.is(tok::eof); TheLexer.LexFromRawLexer(Tok)) {
      if (Tok.is(tok::comment))
        continue;

      // If we already find a #endif candidate for the controlling macro,
      // we shouldn't see any thing after it except the comments. Then,
      // this is not a true `#endif` for the controlling macro.
      if (ControllingMacroEndCands.isValid()) {
        ControllingMacroEndCands = SourceLocation();
        break;
      }

      if (Tok.isAtStartOfLine() && Tok.is(tok::hash)) {
        SourceLocation HashLoc = Tok.getLocation();
        TheLexer.LexFromRawLexer(Tok);

        if (Tok.is(tok::raw_identifier) && !Tok.needsCleaning()) {
          StringRef Keyword = Tok.getRawIdentifier();

          if (!HashIfStack.empty() && llvm::StringSwitch<bool>(Keyword)
                                          .Case("if", true)
                                          .Case("ifdef", true)
                                          .Case("ifndef", true)
                                          .Default(false)) {
            HashIfStack.push_back(HashLoc);
          }

          if (!HashIfStack.empty() && Keyword == "endif") {
            HashIfStack.pop_back();
            if (HashIfStack.empty())
              ControllingMacroEndCands = HashLoc;
          }
        }
      }
    }
  }

  PreambleOffsets Ret;
  // We need to minus 1 here since source location reserves 0 as invalid.
  Ret.OffsetOfPreambleStart = PreambleStart.getRawEncoding() - 1;
  Ret.OffsetOfPreambleEnd = PreambleEnd.getRawEncoding() - 1;

  if (ControllingMacroEndCands.isValid()) {
    Ret.OffsetOfControllingMacroDefStart =
        ControllingMacroDefStartCands.getRawEncoding() - 1;
    Ret.OffsetOfControllingMacroDefEnd =
        ControllingMacroDefEndCands.getRawEncoding() - 1;
    Ret.OffsetOfControllingMacroEnd =
        ControllingMacroEndCands.getRawEncoding() - 1;
  }

  return Ret;
}

// FIXME: We don't use createInvocation from Utils.h since that implementation
// is dirty to add -fsyntax-only all the time, which is not needed for us. We
// only need preprocessing.
std::unique_ptr<CompilerInvocation>
createPreprocessInvocation(const std::vector<std::string> &CommandLine) {
  assert(!CommandLine.empty());
  auto Diags = CompilerInstance::createDiagnostics(new DiagnosticOptions);

  SmallVector<const char *, 16> Args;
  for (const std::string &S : CommandLine)
    Args.push_back(S.c_str());

  // FIXME: We shouldn't have to pass in the path info.
  clang::driver::Driver TheDriver(Args[0], llvm::sys::getDefaultTargetTriple(),
                                  *Diags, "clang LLVM compiler");

  TheDriver.setProbePrecompiled(false);

  std::unique_ptr<clang::driver::Compilation> C(
      TheDriver.BuildCompilation(Args));
  if (!C)
    return nullptr;

  const clang::driver::JobList &Jobs = C->getJobs();
  if (Jobs.size() != 1) {
    SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    Diags->Report(diag::err_fe_expected_compiler_job) << OS.str();
    return nullptr;
  }
  auto Cmd = llvm::find_if(Jobs, [](const clang::driver::Command &Cmd) {
    return StringRef(Cmd.getCreator().getName()) == "clang";
  });
  if (Cmd == Jobs.end()) {
    Diags->Report(diag::err_fe_expected_clang_command);
    return nullptr;
  }

  auto CI = std::make_unique<CompilerInvocation>();
  if (!CompilerInvocation::CreateFromArgs(*CI, Cmd->getArguments(), *Diags,
                                          Args[0]))
    return nullptr;
  return CI;
}

} // namespace

ProcessedInfo InterestingFileManager::preprocessFile(
    StringRef Filename, tooling::CompileCommand Cmd,
    SmallVector<std::string> *AllIncludedFiles) {
  // Add the resource dir manually. Otherwise we may look our resource dir
  // from the invoked compiler, which may not have resource dir we want.
  Cmd.CommandLine.push_back("-I");
  llvm::SmallString<256> ResourceDir(clang::driver::Driver::GetResourcesPath(
      llvm::sys::fs::getMainExecutable(nullptr, nullptr)));
  llvm::sys::path::append(ResourceDir, "include");
  Cmd.CommandLine.push_back((std::string)ResourceDir.str());

  std::unique_ptr<CompilerInvocation> CI =
      createPreprocessInvocation(Cmd.CommandLine);
  if (!CI) {
    std::string CommandLine;
    for (auto &Cl : Cmd.CommandLine)
      CommandLine += Cl + " ";
    elog("failed to create compiler invocation from command line: {0}",
         CommandLine);
    return ProcessedInfo::getFailedInfo();
  }

  FileSystemOptions FileMgrOpts;
  FileManager FileMgr(FileMgrOpts);
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags =
      CompilerInstance::createDiagnostics(new DiagnosticOptions());
  if (!Diags) {
    dlog("Failed to create diagnostic engine for {0}", Filename);
    return ProcessedInfo::getFailedInfo();
  }
  SourceManager SourceMgr(*Diags, FileMgr);
  llvm::Expected<FileEntryRef> MaybeFileRef = FileMgr.getFileRef(Filename);
  if (auto E = MaybeFileRef.takeError()) {
    elog("failed to get file entry from {0}: {1}", Filename,
         toString(std::move(E)));
    return ProcessedInfo::getFailedInfo();
  }

  SourceMgr.setMainFileID(
      SourceMgr.getOrCreateFileID(*MaybeFileRef, clang::SrcMgr::C_User));

  IntrusiveRefCntPtr<TargetInfo> Target =
      TargetInfo::CreateTargetInfo(*Diags, CI->TargetOpts);
  HeaderSearch HeaderInfo(CI->getHeaderSearchOptsPtr(), SourceMgr, *Diags,
                          CI->getLangOpts(), Target.get());

  TrivialModuleLoader ModLoader;
  Preprocessor PP(CI->getPreprocessorOptsPtr(), *Diags, CI->getLangOpts(),
                  SourceMgr, HeaderInfo, ModLoader,
                  /*IdentifierInfoLookup=*/nullptr,
                  /*OwnsHeaderSearch=*/false, TU_Prefix);

  PP.Initialize(*Target);
  ApplyHeaderSearchOptions(HeaderInfo, CI->getHeaderSearchOpts(),
                           CI->getLangOpts(), PP.getTargetInfo().getTriple());

  RawPCHContainerReader PCHReader;
  InitializePreprocessor(PP, PP.getPreprocessorOpts(), PCHReader,
                         CI->getFrontendOpts(), CI->getCodeGenOpts());

  // Use raw lexer to get the preamble of the file.
  PreambleOffsets PreambleOffsetInfo =
      ParsePreamble(Filename, CI->getLangOpts());

  PP.addPPCallbacks(std::make_unique<ProcessedInfoProducer>(
      PP.getSourceManager(), PreambleOffsetInfo.OffsetOfPreambleEnd,
      Config.getControllingMacro()));
  Diags->getClient()->BeginSourceFile(CI->getLangOpts(), &PP);
  PP.EnterMainSourceFile();

  while (true) {
    Token Tok;
    PP.Lex(Tok);
    if (Tok.is(tok::eof))
      break;
  }

  Diags->getClient()->EndSourceFile();

  if (Diags->hasErrorOccurred()) {
    std::string CommandLine;
    for (auto &Cl : Cmd.CommandLine)
      CommandLine += Cl + " ";
    elog("Error happened during preprocessing {0}; Commands: {1}", Filename,
         CommandLine);
  }

  ProcessedInfo Info;
  Info.PreambleInfo = PreambleOffsetInfo;

  ProcessedInfoProducer *PPCallBack =
      static_cast<ProcessedInfoProducer *>(PP.getPPCallbacks());

  Info.DirectlyImported = std::move(PPCallBack->DirectlyImported);
  Info.DirectlyIncluded = std::move(PPCallBack->DirectlyIncluded);
  Info.DirectlyUsedMacros = std::move(PPCallBack->DirectlyUsedMacros);
  Info.ModulesControllingMacroDetected =
      PPCallBack->IsModulesControllingMacroDetected;
  Info.ProcessSuccessed = true;

  // Special case, if this file includes nothing, it might be a root headers
  // only containing macro definitions. Then may be it is better to not think
  // it as a preamble.
  //
  // FIXME: Then we should change the definition of preamble to the sequence of
  // includes?
  if (Info.DirectlyIncluded.empty()) {
    wlog("Find no includes in {0}. Treat if there is no preamble.", Filename);
    Info.PreambleInfo.OffsetOfPreambleStart =
        Info.PreambleInfo.OffsetOfPreambleEnd = 0;
  }

  if (AllIncludedFiles)
    *AllIncludedFiles = std::move(PPCallBack->AllIncludes);

  return Info;
}

InterestingFileInfo *
InterestingFileManager::getInterestingFileInfo(StringRef Filename) {
  auto Iter = InterestingFiles.find(Filename);
  if (Iter == InterestingFiles.end())
    return nullptr;
  return &Iter->getValue();
}

// TOOD: The structure of the function is pretty good for concurrency.
void InterestingFileManager::processInterestingFiles() {
  llvm::StringMap<CompileCommand> FoundNonModulesHeaders;
  if (Config.getCompilationDatabase())
    processWithCompilationDatabase(Config.getCompilationDatabase(),
                                   FoundNonModulesHeaders);

  // The first argument of command line arguments must be the compiler.
  CompileCommand DefaultCommand;
  static int Addr;
  std::string ConverterExecutable = llvm::sys::fs::getMainExecutable(
      "clang-modules-converter", (void *)&Addr);
  llvm::SmallString<256> ClangPath(ConverterExecutable);
  llvm::sys::path::remove_filename(ClangPath);
  llvm::sys::path::append(ClangPath, "clang++");
  DefaultCommand.CommandLine.push_back((std::string)ClangPath);
  for (const auto &Cmd : Config.getDefaultCommandLine())
    DefaultCommand.CommandLine.push_back(Cmd);
  DefaultCommand.CommandLine.push_back("-E");

  for (InterestingFileInfo &FI : *this) {
    if (FI.isProcessed())
      continue;

    CompileCommand Command = DefaultCommand;
    if (FI.isHeader())
      Command.CommandLine.push_back("-xc++-header");
    else
      Command.CommandLine.push_back("-xc++");
    Command.CommandLine.push_back((std::string)FI.getName());

    SmallVector<std::string> AllIncluded;
    setProcessedInfo(FI, preprocessFile(FI.getName(), Command, &AllIncluded),
                     Command, FoundNonModulesHeaders);
  }

  for (auto &Iter : FoundNonModulesHeaders) {
    StringRef FileName = Iter.getKey();
    if (NonInterestingFiles.count(FileName))
      continue;

    SmallVector<std::string> AllIncluded;
    preprocessFile(FileName, Iter.getValue(), &AllIncluded);

    for (StringRef Included : AllIncluded)
      NonInterestingFiles[FileName].AllIncludes.insert(Included);
  }
}

void InterestingFileManager::processWithCompilationDatabase(
    clang::tooling::CompilationDatabase *CDB,
    llvm::StringMap<CompileCommand> &FoundNonModulesHeaders) {
  llvm::DenseMap<InterestingFileInfo *, CompileCommand> FoundInterestingFiles;
  for (const CompileCommand &Command : CDB->getAllCompileCommands()) {
    StringRef File = Command.Filename;

    auto *FI = getInterestingFileInfo(File);
    if (FI && FI->isProcessed())
      continue;

    SmallVector<std::string> AllIncluded;
    ProcessedInfo Info = preprocessFile(File, Command, &AllIncluded);

    if (FI)
      setProcessedInfo(*FI, std::move(Info), Command, FoundNonModulesHeaders);

    for (StringRef Header : AllIncluded) {
      if (auto *InterestingHeader = getInterestingFileInfo(Header)) {
        wlog_if(!InterestingHeader->isHeader(),
                "Find non-header {0} when processing {1}", Header, File);
        FoundInterestingFiles.insert({InterestingHeader, Command});
        InterestingHeader->NonInterestingUsers.insert(File);
      }
    }
  }

  for (auto Iter : FoundInterestingFiles) {
    InterestingFileInfo *FI = Iter.first;

    if (FI->isProcessed())
      continue;

    setProcessedInfo(*FI, preprocessFile(FI->getName(), Iter.second),
                     Iter.second, FoundNonModulesHeaders);
  }
}

bool InterestingFileManager::isFileIncludedByNonInterestingFile(
    StringRef NonInterestingFileName, StringRef QueryFile) const {
  auto Iter = NonInterestingFiles.find(NonInterestingFileName);
  if (Iter == NonInterestingFiles.end())
    return false;

  return Iter->getValue().AllIncludes.count(QueryFile);
}

void InterestingFileManager::setProcessedInfo(
    InterestingFileInfo &FI, ProcessedInfo &&Info,
    clang::tooling::CompileCommand Command,
    StringMap<clang::tooling::CompileCommand> &FoundNonModulesHeaders) {
  dlog_if(FI.isProcessed(), "We processed {0} twice?", FI.CanonicalFileName);

  FI.Info.emplace(std::move(Info));

  if (!FI.isSuccessfulllyProcessed())
    return;

  for (Included &Inc : FI.Info->DirectlyIncluded) {
    if (InterestingFileInfo *IncludedInfo =
            getInterestingFileInfo(Inc.FileName))
      FI.DirectlyIncludedInterestingInfo.push_back(IncludedInfo);
    else
      FI.DirectlyIncludedNonInterestingInfo.push_back(Inc);
  }

  for (const Included &Inc : FI.DirectlyIncludedNonInterestingInfo)
    if (!Config.hasThirdPartyModule(Inc))
      FoundNonModulesHeaders.insert({Inc.FileName, Command});
}