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

constexpr std::string_view llvmCopy = "@llvm.memcpy.p0.p0.i64";

GenClass::FuncResult GenClass::FunctionUnit(const sPtr<ast::FunctionDeclaration> &_funcDecl) {
    bool isCopyResult = false;
    auto funcName = std::format("@{}", _funcDecl->Name);
    auto llvmRetType = TypeToLLVM(_funcDecl->ReturnType);
    if (const auto size = type::GetSize(_funcDecl->ReturnType); size > 16) {
        isCopyResult = true;
        llvmRetType = "void";
    } else if (size == 8) {
        llvmRetType = "i64";
    }
    size_t paramOffset = 0;
    auto functionDecl = std::format("declare {} {}(", llvmRetType, funcName);
    std::vector<std::string> llvmParamTypes(_funcDecl->Parameters.size());
    for (const auto &[type,param]: std::views::zip(llvmParamTypes, _funcDecl->Parameters)) {
        auto paramType = param->VarType;
        if (const auto paramSize = type::GetSize(paramType); paramSize > 16) {
            type = "noundef ptr";
        } else if (paramSize == 8) {
            type = "i64";
        } else {
            type = TypeToLLVM(paramType);
        }
        functionDecl.append(std::format("{} %{},", type, paramOffset++));
    }
    functionDecl.back() = ')'; // 替换最后一个逗号为右括号
    return {
        isCopyResult,
        llvmRetType,
        funcName,
        "",
        llvmParamTypes,
        functionDecl
    };
}
