# MLC

## 简介
这是一个由我手写的一个LLVM C方言前端，虽然语法很类似C，但是它并不能编译任何的.c文件。但是可以通过声明去调用.c文件里的函数和结构体，但不提供全局变量的调用方式，在一些安全性上做出了改进

### 一体化构建引擎
 - 内生依赖扫描：内置拓扑感知算法（RequireScaner），彻底抛弃 CMake/Makefile 等外部工具。
 - 双层哈希校验：基于文件内容指纹与 JSON 格式符号表缓存，实现高效增量构建，避免重复编译。
 - 全机标准库缓存：标准库（Standard Library）一次编译，全机永久复用。

### 三合一处理
 - 三合一流水线：词法、语法、语义分析高度集成，直接坍缩生成 AST（抽象语法树）
 - 文本流 IR 映射：跳过臃肿的 LLVM API，通过高密度的字符串模板直接生成精准的 LLVM IR 文本。


### 现代化语言特性
 - 强类型系统：采用 i8/i16/u32/u64/f32 等明确类型，杜绝 C 语言的模糊定义。
 - 显式转换：禁止隐式类型转换，必须使用函数风格，如 i32 a = i32(2.8)。
 - 现代循环：计划引入更现代化的 for 循环语法（暂未实现）。
 - 无预处理器：采用 export/import 模块系统，import std.io; 即导入 std/io.mc 中所有导出的符号。
 - 强制返回值：所有函数必须显式 return，switch 语句必须包含 default 分支。
 - 枚举类：采用类似 C++ 的 enum class 语法，增强类型安全。
 - 解除了前向声明限制，函数可以在调用前定义，结构体指针互相引用也无需前向声明。但单纯的包含需要先定义子类型再定义父类型

### 技术栈
 - 现代C++，完全使用modules，除taskflow和json库外无任何头文件
 - 纯净实现，无任何第三方依赖（除taskflow和json库外）
 - 直接生成LLVM IR文本，无需链接LLVM库，极大简化构建流程


### 语言语法
参照Example目录下的示例文件.


## 构建方法

### 推荐工具链
- Clang 21
- CMake 4.2.3
- Ninja
- VCPKG

Windows上必须使用LLVM-MinGW，clang-cl无模块支持
### 预先准备
Windows上，需要将VCPKG的默认default triplet写为x64-mingw
</br>
或者在CMAKE选项里手动指定
`-DVCPKG_DEFAULT_TRIPLET=x64-mingw-static`
或者 </br>
`-DVCPKG_DEFAULT_TRIPLET=x64-mingw-dynamic`


需要手动开启标准库模块构建
```CMAKE
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_WARN_DEV OFF CACHE BOOL "" FORCE)
```

你需要将`d0edc3af-4c50-42ea-a356-e2862fe7a444`替换为你的CMake版本的token，可以在CMake的github仓库里找到对应的token


如果使用VCPKG，那还需要引入VCPKG,注意TRIPLET在windows上必须为x64-mingw的

你需要手动的修改我的cmakelists.txt文件，将你有关vcpkg或者其他库的路径以及标准库模块构建信息写入。
我使用一个global.cmake然后写入了环境变量来实现默认启用，如果你想这么做也可以

### 构建步骤
配置好工具链以后，直接使用CMake配置然后构建即可


## 使用方法

MLC的使用方法非常简单，直接在命令行里输入
```bash
mlc <source_file.mc>
```

MLC会自动解析依赖文件，然后自动规划顺序，在source_file.mc所在目录下生成一个BuildOutput文件夹，里面会有若干个依赖的.ll以及一个output.exe文件，output.exe就是编译好的可执行文件了

同时一般还会有配套的.hash文件，这些用来存储文件的哈希值，以便下次增量构建时进行校验，避免重复编译

然后被引用的文件的目录下会多出一个.cache文件夹，.cache文件里会有一个同名.json，这里储存了导出的符号表信息，供其他文件在编译时查询使用