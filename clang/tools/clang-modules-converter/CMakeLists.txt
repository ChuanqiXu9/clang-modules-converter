set(LLVM_LINK_COMPONENTS
  Core
  Option
  Support
  TargetParser
)

add_clang_tool(clang-modules-converter
  ConverterConfig.cpp
  InterestingFile.cpp
  ModulesConverter.cpp
  ProcessInterestingFiles.cpp
  Rewriter.cpp
  StdModuleGenerator.cpp
)

set(CONVERTER_CLANG_LIB_DEPS
  clangBasic
  clangDriver
  clangFrontend
  clangRewrite
  clangLex
  clangTooling
  )

clang_target_link_libraries(clang-modules-converter
  PRIVATE
  ${CONVERTER_CLANG_LIB_DEPS}
  )

