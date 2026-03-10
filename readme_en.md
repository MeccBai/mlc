# MLC

## Introduction
This is a handwritten LLVM C-dialect frontend. Although the syntax is similar to C, it cannot compile any .c files. However, it can call functions and structs from .c files through declarations, but does not provide access to global variables. Some improvements have been made in terms of safety.

### Integrated Build Engine
- **Native Dependency Scanning**: Built-in topology-aware algorithm (RequireScanner) completely eliminates the need for external tools like CMake/Makefile.
- **Dual-layer Hash Verification**: Efficient incremental builds based on file content fingerprints and JSON-format symbol table caches, avoiding redundant compilation.
- **Machine-wide Standard Library Cache**: The standard library is compiled once and permanently reused across the entire machine.

### Three-in-One Processing
- **Three-in-One Pipeline**: Lexical, syntactic, and semantic analysis are highly integrated, directly collapsing into AST (Abstract Syntax Tree) generation.
- **Text Stream IR Mapping**: Bypasses the bloated LLVM API by directly generating precise LLVM IR text through high-density string templates.

### Modern Language Features
- **Strong Type System**: Uses explicit types like i8/i16/u32/u64/f32, eliminating C language's ambiguous definitions.
- **Explicit Conversion**: Implicit type conversion is prohibited; function-style casting must be used, e.g., `i32 a = i32(2.8)`.
- **Modern Loops**: Plans to introduce more modern for-loop syntax (not yet implemented).
- **No Preprocessor**: Uses export/import module system; `import std.io;` imports all exported symbols from `std/io.mc`.
- **Mandatory Return Values**: All functions must explicitly return; switch statements must include a default branch.
- **Enum Class**: Uses C++-like enum class syntax for enhanced type safety.
- **No Forward Declaration Restrictions**: Functions can be defined before being called, and struct pointers can reference each other without forward declarations. However, for pure containment, the child type must be defined before the parent type.

### Technology Stack
- Modern C++, fully using modules, no header files except for taskflow and json libraries
- Clean implementation, no third-party dependencies (except taskflow and json libraries)
- Directly generates LLVM IR text, no need to link LLVM libraries, greatly simplifying the build process

### Language Syntax
Refer to the example files in the Example directory.

## Build Instructions

### Recommended Toolchain
- Clang 21
- CMake 4.2.3
- Ninja
- VCPKG

On Windows, you must use LLVM-MinGW; clang-cl has no module support.

### Prerequisites
On Windows, you need to set VCPKG's default triplet to x64-mingw.
</br>
Or manually specify in CMake options:
```
-DVCPKG_DEFAULT_TRIPLET=x64-mingw-static
```
or
```
-DVCPKG_DEFAULT_TRIPLET=x64-mingw-dynamic
```

You need to manually enable standard library module building:
```cmake
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_WARN_DEV OFF CACHE BOOL "" FORCE)
```

You need to replace `d0edc3af-4c50-42ea-a356-e2862fe7a444` with the token for your CMake version, which can be found in the CMake GitHub repository.

If using VCPKG, you also need to include VCPKG. Note that the TRIPLET on Windows must be x64-mingw.

You need to manually modify my CMakeLists.txt file to include your vcpkg or other library paths and standard library module building information.
I use a global.cmake and write it into environment variables for default enabling. You can do the same if you wish.

### Build Steps
After configuring the toolchain, simply configure with CMake and then build.

## Usage

Using MLC is very simple. Just type in the command line:
```bash
mlc <source_file.mc>
```

MLC will automatically parse dependency files, plan the order automatically, and generate a BuildOutput folder in the same directory as source_file.mc. Inside, there will be several dependent .ll files and an output.exe file. output.exe is the compiled executable.

Additionally, there will usually be accompanying .hash files. These store file hash values for verification during the next incremental build, avoiding redundant compilation.

Referenced file directories will have an additional .cache folder. Inside the .cache folder, there will be a same-named .json file that stores exported symbol table information for other files to query during compilation.
