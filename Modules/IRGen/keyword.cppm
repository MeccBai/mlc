//
// Created by Administrator on 2026/2/26.
//

export module keyword;


import std;

export namespace mlc::ir::kv {

    // --- 内存操作 (Memory Ops) ---
    constexpr std::string_view Alloca = "alloca";
    constexpr std::string_view Load   = "load";
    constexpr std::string_view Store  = "store";
    constexpr std::string_view Gep    = "getelementptr"; // 处理数组/结构体的神
    constexpr std::string_view Align = "align"; // 内存对齐

    // --- 算术运算 (Arithmetic Ops) ---
    constexpr std::string_view Add    = "add";
    constexpr std::string_view Sub    = "sub";
    constexpr std::string_view Mul    = "mul";
    constexpr std::string_view Sdiv   = "sdiv"; // 有符号除法
    constexpr std::string_view Srem   = "srem"; // 取余

    // --- 比较与逻辑 (Logic Ops) ---
    constexpr std::string_view Icmp   = "icmp"; // 整数比较
    constexpr std::string_view Equal     = "eq";   // equal
    constexpr std::string_view NotEqual     = "ne";   // not equal
    constexpr std::string_view SignGreater    = "sgt";  // signed greater than
    constexpr std::string_view SignLess    = "slt";  // signed less than

    // --- 控制流 (Control Flow) ---
    constexpr std::string_view Jump     = "br";
    constexpr std::string_view Return    = "ret";
    constexpr std::string_view Label  = "label";
    constexpr std::string_view Call   = "call";

    // --- 类型与定义 (Type & Define) ---
    constexpr std::string_view Define  = "define";
    constexpr std::string_view Declare = "declare";
    constexpr std::string_view Type    = "type";
    constexpr std::string_view Global  = "global";

    // --- 常用数据类型 ---
    constexpr std::string_view i1     = "i1";
    constexpr std::string_view i8     = "i8";
    constexpr std::string_view i16    = "i16";
    constexpr std::string_view i32    = "i32";
    constexpr std::string_view i64    = "i64";
    constexpr std::string_view f32    = "f32";
    constexpr std::string_view f64    = "f64";
    constexpr std::string_view u8     = "u8";
    constexpr std::string_view u16     = "u16";
    constexpr std::string_view u32     = "u32";
    constexpr std::string_view u64     = "u64";

    constexpr std::string_view ptr    = "ptr";  // 现代 LLVM 统一使用不透明指针
    constexpr std::string_view VoidType = "void";
    constexpr std::string_view Struct = "struct";

    constexpr std::string_view Truncate = "trunc"; // 高位截断 (i64 -> i32)
    constexpr std::string_view SignExt  = "sext";  // 符号扩展 (i32 -> i64)
    constexpr std::string_view ZeroExt  = "zext";  // 零扩展
    constexpr std::string_view To    = "to";    ; // 配合关键字：trunc i64 %1 to i32

    constexpr std::string_view Private = "private";
}

