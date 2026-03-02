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
    if (isCopyResult) {
        functionDecl+= "ptr %0,";
        paramOffset = 1; // 参数偏移1，因为第一个参数是返回值的指针
    }
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

GenClass::FuncHeader GenClass::FunctionCall(
    const sPtr<ast::FunctionCall> &_funcCall, const std::string_view _varName) {

    auto funcName = std::format("@{}", _funcCall->FunctionDecl->Name);

    auto args = _funcCall->Arguments | std::views::transform([](const auto &arg) {;
        return ExpressionExpand(arg);
    }) | std::ranges::to<std::vector>();


}

GenClass::ExprResult GenClass::FunctionArg(FuncArg &_funcArg, size_t _index) {
    auto argName = "%" + std::to_string(_index); // %0, %1...
    auto currentType = _funcArg.llvmType;
    std::string code;
    const auto resultVar = _funcArg.resultVar;
    std::string resultType = "ptr"; // 最终结果通常是栈上的指针
    if (_funcArg.isCasting) {
        code += std::format("%{} = alloca {}, align 8\n", resultVar, _funcArg.llvmType);
        code += std::format("store i64 {}, ptr %{}, align 8\n", argName, resultVar);
        resultType = "ptr";
    } else if (_funcArg.isMemoryArg) {
        auto localStackVar = std::string(resultVar);
        code += std::format("%{} = alloca {}, align {}\n", localStackVar, currentType, _funcArg.size);
        size_t typeSize = _funcArg.size;
        code += std::format("  call void @llvm.memcpy.p0.i64(ptr align 8 %{}, ptr align 8 {}, i64 {}, i1 false)\n",
                            localStackVar, argName, typeSize);
        resultType = "ptr";
    } else {
        auto stackVar = std::string(resultVar);
        code += std::format("%{} = alloca {}, align {}\n", stackVar, currentType, _funcArg.size);
        code += std::format("  store {} {}, ptr %{}, align {}\n", currentType, argName, stackVar, _funcArg.size);
        resultType = "ptr";
    }
    return {resultType, resultVar, code};
}

void GenClass::FunctionGenerate(const sPtr<ast::FunctionScope> &_func) {
    const auto decl = _func->ToDeclaration();
    std::vector<FuncArg> args;
    auto funcResult = FunctionUnit(decl);
    args.reserve(decl->Parameters.size());
    for (auto &i: decl->Parameters) {
        args.emplace_back(FunctionArgAnalyze(*i));
    }
    std::string code;
    for (auto [index,arg]: std::views::zip(std::views::iota(0ull, args.size()), args)) {
        auto [resultType, resultVar, argCode] = FunctionArg(arg, index);
        code += argCode;
    }
}

GenClass::FuncArg GenClass::FunctionArgAnalyze(const ast::VariableStatement &_param) {
    const auto paramType = _param.VarType;
    auto llvmType = TypeToLLVM(paramType);
    bool isCasting = false;
    bool isMemoryArg = false;
    const size_t size = type::GetSize(paramType);
    if (size > 16) {
        isMemoryArg = true;
        llvmType = "ptr";
    } else if (size == 8) {
        if (ast::Type::GetTypeName(*_param.VarType) != "i64") {
            isCasting = true;
        }
        llvmType = "i64";
    }
    return {isCasting, isMemoryArg, size, llvmType, _param.Name};
}
