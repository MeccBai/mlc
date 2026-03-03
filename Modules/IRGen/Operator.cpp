//
// Created by Administrator on 2026/3/1.
//

module Generator;

import std;
import Token;
import std;
import keyword;
import Parser;
import aux;

namespace gen = mlc::ir::gen;

using GenClass = gen::IRGenerator;
namespace ast = mlc::ast;
using size_t = std::size_t;
template<typename type>
using sPtr = std::shared_ptr<type>;
namespace type = ast::Type;

std::string GenClass::OperatorToIR(const type::BaseType *_type, const ast::BaseOperator _value) {
    const auto llvmType = TypeToLLVM(std::make_shared<type::CompileType>(*_type));
    return OperatorToIR(llvmType, _value);
}

std::string GenClass::OperatorToIR(const std::string_view _llvmType, const ast::BaseOperator _value) {
    switch (_value) {
        case ast::BaseOperator::Add:
            if (_llvmType.starts_with('f')) {
                return "fadd"; // 浮点数加法
            }
            return "add";
        case ast::BaseOperator::Sub:
            if (_llvmType.starts_with('f')) {
                return "fsub"; // 浮点数减法
            }
            return "sub";
        case ast::BaseOperator::Mul:
            if (_llvmType.starts_with('f')) {
                return "fmul"; // 浮点数乘法
            }
            if (_llvmType.starts_with('i')) {
                return "mul"; // 整数乘法
            }
            return "mul";
        case ast::BaseOperator::Div:
            if (_llvmType.starts_with('i')) {
                return "sdiv"; // 假设有符号
            }
            if (_llvmType.starts_with('f')) {
                return "fdiv"; // 浮点数除法
            }
            return "udiv"; // 无符号
        case ast::BaseOperator::Mod:
            if (_llvmType.starts_with('i')) {
                return "srem"; // 假设有符号
            }
            if (_llvmType.starts_with('f')) {
                return "frem"; // 浮点数模运算
            }
            return "urem"; // 无符号
        // --- 关系运算 (整数比较) ---
        // ⚠️ LLVM 的 icmp 需要具体的条件代码 (eq, ne, sgt, etc.)
        case ast::BaseOperator::Equal:
            if (_llvmType.starts_with('f')) {
                return "fcmp oeq"; // 浮点数相等
            }
            return "icmp eq";
        case ast::BaseOperator::NotEqual:
            if (_llvmType.starts_with('f')) {
                return "fcmp one"; // 浮点数不相等
            }
            return "icmp ne";
        case ast::BaseOperator::Greater:
            if (_llvmType.starts_with('f')) {
                return "fcmp ogt"; // 浮点数大于
            }
            if (_llvmType.starts_with('i')) {
                return "icmp sgt"; // 有符号大于
            }
            return "icmp ugt"; // 无符号大于
        case ast::BaseOperator::Less:
            if (_llvmType.starts_with('f')) {
                return "fcmp olt"; // 浮点数小于
            }
            if (_llvmType.starts_with('i')) {
                return "icmp slt"; // 有符号小于
            }
            return "icmp ult"; // 无符号小于
        case ast::BaseOperator::GreaterEqual:
            if (_llvmType.starts_with('f')) {
                return "fcmp oge"; // 浮点数大于等于
            }
            if (_llvmType.starts_with('i')) {
                return "icmp sge"; // 有符号大于等于
            }
            return "icmp uge"; // 无符号大于等于
        case ast::BaseOperator::LessEqual:
            if (_llvmType.starts_with('f')) {
                return "fcmp ole"; // 浮点数小于等于
            }
            if (_llvmType.starts_with('i')) {
                return "icmp sle"; // 有符号小于等于
            }
            return "icmp ule"; // 无符号小于等于

        // --- 逻辑运算 (通常使用位运算指令) ---
        case ast::BaseOperator::And: return "and";
        case ast::BaseOperator::Or: return "or";
        case ast::BaseOperator::Not: return "xor";
        // Not 通常使用 xor 1 实现

        // --- 位运算 ---
        case ast::BaseOperator::BitAnd: return "and";
        case ast::BaseOperator::BitOr: return "or";
        case ast::BaseOperator::BitXor: return "xor";
        case ast::BaseOperator::ShiftLeft: return "shl";
        // ⚠️ 右移需要区分算术右移和逻辑右移
        case ast::BaseOperator::ShiftRight: return "ashr"; // 算术右移

        // --- 指针操作 ---
        case ast::BaseOperator::AddressOf: return "ptrtoint"; // 视具体需求而定
        case ast::BaseOperator::Dereference: return "load"; // 需要额外处理类型

        default: return "";
    }
}

std::string GenClass::TypeToLLVM(const sPtr<type::CompileType> &_type) {
    if (const auto baseType = std::get_if<type::BaseType>(&*_type)) {
        if (baseType->Name.starts_with('i')) {
            return std::format("i{}", baseType->Size() * 8);
        }
        if (baseType->Name.starts_with('f')) {
            return std::format("f{}", baseType->Size() * 8);
        }
    }
    if (const auto structDef = std::get_if<type::StructDefinition>(&*_type)) {
        return std::format("%struct.{}", structDef->Name);
    }
    if (const auto arrayType = std::get_if<type::ArrayType>(&*_type)) {
        return std::format("[{} x {}]", arrayType->Length, TypeToLLVM(arrayType->BaseType));
    }
    if (std::get_if<type::PointerType>(&*_type)) {
        return "ptr";
    }
    if (std::get_if<type::EnumDefinition>(&*_type)) {
        return "i32";
    }
    ErrorPrintln("Error: Unsupported type for LLVM IR generation.");
    std::exit(-1);
}
