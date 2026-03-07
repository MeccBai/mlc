# MLC 
## Description
- MLC(Mecc LLVM C-Frontend) is a C compiler based on the LLVM infrastructure. It is designed to provide a modern and safer C-Language. It can't compile any .c file, but you can use function declarations in .c files to call functions defined in .c files. MLC is still in early development, and it is not yet ready for production use. However, it is a promising project that aims to provide a better C programming experience.

- Non-streaming architecture: The system first undergoes dehydration processing, followed by the use of global slicing to partition the data into several discrete blocks, which are then processed sequentially. Single-Pass Integrated Frontend Pipeline, the Abstract Syntax Tree (AST) is generated directly from raw strings. Subsequently, the LLVM IR is emitted directly from the AST.

- Non-Third Library: MLC is built without relying on any third-party libraries, ensuring a lightweight and efficient compiler.

- Undergraduate Project : MLC is an undergraduate project, and it is still in early development. It is not yet ready for production use. It is born out of the personal interest and passion of the developer, and it aims to explore the field of compiler design and implementation.
## 介绍
- MLC(Mecc LLVM C-Frontend) 是一个以LLVM为基础的C语言编译器。它旨在提供一个现代化和更安全的C语言。它不能编译任何.c文件，但你可以使用.c文件中的函数声明来调用在.c文件中定义的函数。MLC仍处于早期开发阶段，还不适合生产使用。然而，它是一个有前途的项目，旨在提供更好的C编程体验。

- 非流式架构：系统首先进行脱水处理，然后使用全局切片将数据划分为几个离散的块，依次处理。将词法语法语义分析三合一，直接从原始字符串生成抽象语法树（AST）。随后，直接从AST发出LLVM IR。

- 非第三方库：MLC在构建过程中不依赖任何第三方库，确保了编译器的轻量级和高效性。

- 学生项目：MLC是一个本科生项目，目前仍处于早期开发阶段，还不适合生产使用。它因开发者的个人兴趣和热情而生，旨在探索编译器设计和实现的领域。

## Features
- Variable shadowing is strictly prohibited.
- All primitive types must use LLVM-style naming (e.g., i32, f64, u8).
- Variable definitions within switch blocks are not allowed.
- Forward declaration constraints remain for struct-by-value embedding.
- No forward declaration constraints for struct pointer members.
- Function calls do not require forward declarations (Global Scope Resolution).
- Explicit type casting must use functional syntax (e.g., i32(5.5)).
- Arrays are not permitted as function parameters or struct members (use pointers).
- Pointer arithmetic is disabled (Manual offset calculation only).
- Currently, for loops are not supported but will be added soon.
- Type casting is currently not implemented.

## 特性
- 严格禁止变量遮蔽。
- 所有基本类型必须使用LLVM风格的命名（例如i32、f64、u8）。
- 不允许在switch块内定义变量。
- 结构体按值嵌套仍然存在前向声明约束。
- 结构体指针成员没有前向声明约束。
- 函数调用不需要前向声明（全局作用域解析）。
- 显式类型转换必须使用函数式语法（例如i32(5.5)）。
- 数组不允许作为函数参数或结构体成员（使用指针）。
- 禁止指针运算（仅允许下标访问）。
- 目前不支持for循环，但很快会添加
- 类型转换目前没有实现

## Future Features
- Introduce import/export keywords to replace function declarations (except for C-interop).
- Out-of-the-box dependency resolution for .mc files; automatic target generation without external build tools.
- Implement JSON-based compilation caching to support binary distribution, replacing traditional header-based models.
- Currently, for loop is not supported

## 未来特性
- 引入import/export关键字替代函数声明（C-interop除外）。
- 开箱即用的.mc文件依赖解析；无需外部构建工具的自动目标生成。
- 实现基于JSON的编译缓存以支持二进制分发，取代传统的基于头文件的模型。
- 目前不支持for循环

## Environment & Build Configuration
### Prerequisites
- CMake 4.0 or higher
- Clang 20.0 or higher
- Ninja

### 准备
- CMake 4.0或更高版本
- Clang 20.0或更高版本
- Ninja

### Recommended Toolchain 
It is highly recommended to use CLion paired with Clang 21.8. On Windows, LLVM-MinGW is the preferred distribution, with Ninja configured as the underlying builder.

### 推荐工具链
强烈推荐使用CLion配合Clang 21.8。在Windows上，LLVM-MinGW是首选的发行版，Ninja配置为底层构建工具。


### Build Instructions
Module Support: Module support is integrated via global.cmake, which is enabled by default through environment variables to support the construction of the std module.

global.cmake: 
```cmake
# global.cmake
# Enable module support by default
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_WARN_DEV OFF CACHE BOOL "" FORCE)
```
You need to replace "d0edc3af-4c50-42ea-a356-e2862fe7a444" with your uuid of your cmake.
You can get it in cmake github repository:

You can also add them to the CmakeLists.txt file after cloning the repo.

Clone this repo, then execute the following command to initialize the project:

```Shell
cmake -DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_MAKE_PROGRAM=ninja \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-G Ninja \
-B .\build\Debug \
-DCMAKE_TOOLCHAIN_FILE="path/to/your/global.cmake"
```

Then you can build the project using the following command:

```Shell
cmake --build .\build\Debug --target MLC
```

### 构建说明
模块支持：模块支持通过global.cmake集成，默认通过环境变量启用以支持std模块的构建。
global.cmake:
```cmake# global.cmake
# Enable module support by default
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_WARN_DEV OFF CACHE BOOL "" FORCE)
```
你需要将"d0edc3af-4c50-42ea-a356-e2862fe7a444"替换为你的cmake的uuid。你可以在cmake github仓库中找到它：
你也可以在克隆仓库后将它们添加到CmakeLists.txt文件中。

克隆这个仓库，然后执行以下命令来初始化项目：

```Shell
cmake -DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_MAKE_PROGRAM=ninja \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-G Ninja \
-B .\build\Debug \
-DCMAKE_TOOLCHAIN_FILE="path/to/your/global.cmake"
```

然后你可以使用以下命令来构建项目：

```Shell
cmake --build .\build\Debug --target MLC
```

## Roadmap
-[ ] Method Injection: Integrated struct impl logic for object-oriented style in C.
-[ ] Loop Modernization: Reimagining the traditional for loop for safety and clarity.
-[ ] Memory & Ownership Precursors: Enforcing the "Single Mutable Pointer" rule to mitigate data races at the compiler level.

## 路线图
-[ ] 方法注入：集成结构体impl逻辑，实现C语言的面向对象风格。
-[ ] 循环现代化：重新设计传统的for循环，以提高安全性和清晰度。
-[ ] 内存与所有权前置条件：强制执行“单可变指针”规则，在编译器级别缓解数据竞争。

## Example
Example files are located in the "Example" directory. You can build and run them after MLC built;

## 示例
示例文件位于"Example"目录下。你可以在MLC构建完成后构建并运行它们；

## License
Apache License 2.0

## 许可证
Apache License 2.0