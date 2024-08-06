
# Intro

A clang-based helper tool to rewrite a header based C++ project into a module based one.

The tool provides three styles to rewrite the project:

- Rewrite a header based project to make it provide the module interfaces,
  but still keep the project as header based one. Which is knwon as header
  wrappers style.
- Rewrite a header based project to a module based one, but still provid
  the original headers. This style is helpful when we decide to develop
  new features in modules and like to keep the old interfaces.
- Rewrite a header based project to module based one completely without
  keeping the headers. This may be wanted if no users of the project would
  use the header any more as expected.

Note this tool is **not** expected to run continuously like clang-format. This tool is only expected to save some trivial works to refactor the projects.

# Examples

The small examples can be found in `clang/test/ClangModulesConverter` and 
a real world example can be found in [Header Wrapper for async_simple](https://github.com/ChuanqiXu9/async_simple/tree/HeaderWrapper).

# Build from source

This project lives in truncated clang/LLVM's source tree. We can built it by a similar way to [build clang from source](https://llvm.org/docs/GettingStarted.html).

An example maybe:

```
cmake -DLLVM_ENABLE_PROJECTS="clang;" -DCMAKE_BUILD_TYPE=Release ../llvm
make clang-modules-converter -j
```

# How this works

The core ability of the tool is to find the preamble section for each files in the project. The preamble section is the section to introduce dependency of the file. For example,

```C++
// my_lib.h
// Start of preamble
#include <iostream>
#include <string>
#ifdef USE_MAP
#include <map>
#endif
#include "local_headers.h"
// End of preamble

namespace my_lib {
  ...
}
```

Then we can manipulate the preamble section:

```C++
// my_lib.h
#ifndef MY_LIB_USE_MODULES
#include <iostream>
#include <string>
#ifdef USE_BOOST
#include <boost/...>
#endif
#include "local_headers.h"
#endif // MY_LIB_USE_MODULES

namespace my_lib {
  ...
}
```

So the header will only contain its own body when `MY_LIB_USE_MODULES` is defined. Then we can export its body in the moduel interface:

```C++
export module my_lib;
import std; // we detected use of std module
import boost; // we detected use of boost module

#define MY_LIB_USE_MODULES
export extern "C++" {
  #include "my_lib.h"
}
```

Similarly, for source files, after we detected the preamble, we can replace
the preamble of including headers to import corresponding modules.

# Config file syntax

The tool only accepts one argument `--config=` to the path of the config file.

The tool need a yaml config file to work. The fields for the config file are:

- `root_dir`. Optional. The root of the project. If not provided, the path of the config file is considered to be the root of the project.
- `modules`. A list of modules we wish to generate. The fileds of each mdoule are:
  - `name`. The name of the module to be generated.
  - `path`. The path to the module interface unit to be generated.
  - `headers`. A string or a list of strings to describe the path of headers which we wish to rewrite and grap their contents to the module interface.
  - `excluded_headers`. A string or a list of strings to describe the path  of headers shouldn't be in the module interface. 
  - `srcs`. A string or a list of strings to describe the source files which will be rewritten into module implementation units for the module.
  - `excluded_srcs`. A string or a list of strings to describe the path  of source files which wouldn't be module implementation units.
  - `prefix_map_of_module_units_for_headers`. A string or a list of strings to describe the map for the prefix of the path of the generated module units. Each string should contain a `:` symbol. And the path starts with the LHS of `:` will be mapped to the RHS of `:`. This is not meaningful to header wrappers mode. Without the field,
  the module units for the headers are generated in the same directorie, but if the headers live in `include` directory and we wish the generated module units to live in `module` directory, we can specify the value of 
  `prefix_map_of_module_units_for_headers` as `include:module`.
- `third_party_modules`. A list of third party (existing) modules. The fileds are:
  - `name`. The name of the third party module.
  - `headers`. A string or a list of string.  Each string is a regex to describe the included text. 
- `srcs_to_rewrite`. A string or a list of strings to describe the path of sources (generally the files ends with `.cpp` and `.cc`, not headers.) we wish to rewrite to use modules. `*` and `**` wildcards are supported.
- `srcs_excluded_to_rewrite`. A string or a list of strings to describe the path of sources we don't want to rewrite. All sources matched won't be rewritten.
- `mode`. Required. The working mode of the tool. Support values are:
  - `header-wrapper`. The tool will try to rewrite the headers and generate the corresponding modules.
  - `rewrite-headers-to-module-units`. The tool will try to rewrite the headers to module units. Then it is the job of users to control the visibility of these module units.
  - `rewrite-headers-to-partitions`. The tool will try to rewrite the headers to module partitions of the corresponding module.
- `controlling_macro`. A string to describe the modules controlling macro. We use this macro to detect if we're in modules for headers.
- `remain_headers`. A boolean value. By default true. If it is true and the `mode` is not `header-wrapper`, the tool will try to embed the body of the headers into the corresponding module units and remove unused headers.
- `keep_traditional_abi`. A boolean value. By default true. If it is false, the tool will try to generate codes within modules directly, which will be in modules ABI.
- `is_std_module_available`. Required. A boolean value. If the `std` module is avaialable in the environments. If not, the tool will try to generate a std module for you since the std module is pretty important in the ecosystem of an ideal modular world.
- `std_module_path`. A string to describe the path of the generated std module. Only meaningful when `is_std_module_available` is false.
- `compilation_database`. A stirng to describe the path to the compile commands of the project. The tool will use the infomration to preprocess the whole projects.
- `default_compile_commands`. A string to describe the default commands if 
`compilation_database` is not available or we failed to find the some files in `compilation_database`.

An example maybe:

```yaml
modules:
  - name: my_module
    path: modules/async_simple.cppm
    headers: includes/**/*.h
    excluded_headers:
      - "**/test/**"
third_party_modules:
  - name: boost
    headers: boost
mode: header-wrapper
controlling_macro: MY_LIB_USE_MODULES
is_std_module_available: false
srcs_to_rewrite: "**/srcs/*.cc"
srcs_excluded_to_rewrite: "**/test/**"
compilation_database: build/compile_commands.json
std_module_path: third_party/std.cppm
```

# Workflow

A version controlling tool (e.g., git) is expected when using this tool.

A real world example can be found in [Header Wrapper for async_simple](https://github.com/ChuanqiXu9/async_simple/tree/HeaderWrapper).

Abstractly, the workflow of the tool may be:
1. Build the projects first and get the compilation commands. 
2. Write the config file according the information of the project.
3. Invoking `clang-modules-converter` to rewrite the projects initially.
4. Wrtie the modules related things in the build systems.
5. Try to build the modules and fix bugs (highly possible).
6. Refine and polish the generated codes.

The error types I've seen includes:
1. The use of macros.
2. A lot of kinds of error due to implicit or missing includes.

For macros, this tool will try to recognize the macro used and give a 
warning daignostic message for this, like:

```
// There unhandled macro uses found in the body:
//	'assert' defined in /usr/include/assert.h:50:10
```

We hope such information can ease the process of rewriting.

For the example of implcit or missing includes,

```
// a.h
inline int a() { ... }

// b.h
// missing including a.h !
inline int b() { return a(); }
```

We forgot including `a.h` in `b.h` but we didn't find this due to we always include `a.h` before including `b.h`. Then this tool may get an invalid dependency information and generates codes that include `b.h` before `a.h`.

And also,

```
// a.h
#include "third_party1/..." // third_party1 includes third_party2
inline int a() { 
  third_party2 xxx;
}
```

in this case, since `a.h` doesn't include headers from third_party2 directly, the tool may not generate codes to import third_party2.

# Possible Questions & Answers

## Why does the project live in truncated clang/LLVM's source tree?

Since I hope to merge this tool into clang/LLVM and the current form looks easier to merge it.

## Why will the tool generate the std module?

The std module is pretty important in the ideal modular ecosystem. In the ideal modular world, every declaration should live in a single module file. Otherwise we may not get best compile-time performance. But it may not be achieveable without the std module. So this tool tried to generate the std module when it is not available to emphasize the importance of std module.

## Why the tool don't include the headers containing the used macros directly?

Since the header defining the used macros may not be intented to be used directly. For example we have an interface header `interface.h`, and its contents was implemented in several "implementation" headers. But the intention was to make users to include the `interface.h` instead of the corresponding actual file that defining the macros.

The other point is, it will be better to split headers to define macros only instead of copy the headers directly. For example,

```C++
// Common.h

// Sections to define macros
#define ...
#define ...

// Sections to define common functions and classes.
...
```

Then it will be better to split a `CommonMacro.h` from the `Common.h` file. So that the users depends on the macros may not need to include `Common.h` to introduce unnecessary entities.

## How does the tool generate the std module?

The tool will generate the std module on demand. It will scan the use of 
std headers in your project and generate a mock std module based on the use 
of std headers. In another word, if you only uses `<vector>`, the generated
std module may not contain `<set>`.

The key reason to do this is, only the standard library vendors themselves can understand what they provide, even if we have a specification. If we generate the std module based on the specification only, then we will get many errors like, `missing header <stacktrace>`, `missing header <span>`...

Another point is compatiblity. The C++'s ecosystem allows us to mix the use of standard library and the compilers. e.g, we are allowed to use clang with libstdc++. But due to they are in different projects, there might be some gaps to match new features. e.g., there was a time that clang couldn't compile the `<concept>` header provided by libstdc++.

## What if the generated std module is not good?

We can adjust it ourselves. 

If the tool failed to treat some std headers, we can add it outselves by adding it like a third party module:

```yaml
third_party_modules:
  - name: std
    headers: new_std_header
```

If the tool doesn't export needed names, we can update the generated std module directly. Or update the tool itselves.

## Is this tool portable on Windows

When I develop this tool, I tried to make it portable intentionally. But I admit I never test or build this on Windows. So there might be some mismatches. Feedbacks and contributions are highly welcomed.

## What if I want to mark some headers as part of a module interface but not rewriting it?

In this case, we can put the following text in these headers:

```C++
#ifndef ASYNC_SIMPLE_USE_MODULES
#endif // ASYNC_SIMPLE_USE_MODULES
```

Because we won't rewrite the headers that was already converted. The rationale is, if the tool detected such pattern, we assume it is edited or verified by users. Then we should respect users' decisions.

# The structure of the tool

All the tests are in `clang/test/ClangModulesConverter`. The implementation codes are in `clang/tools/clang-modules-converter`. The main file is `clang/tools/clang-modules-converter/ModulesConverter.cpp`.

The tool has 4 functional modules:
- Yaml config file parsing. Implemented in `clang/tools/clang-modules-converter/ConverterConfig.cpp`.
- Interesting file recognizing. Implemented in `clang/tools/clang-modules-converter/InterestingFile.cpp`.
- (Pre)Processing the targeted project to get the information. Implemented in `clang/tools/clang-modules-converter/ProcessInterestingFiles.cpp`.
- Rewriting. Implemented in `Rewriter.cpp`.
  - Additionally, the codes to generate std module is in `clang/tools/clang-modules-converter/StdModuleGenerator.cpp`.
