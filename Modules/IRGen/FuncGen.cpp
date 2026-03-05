//
// Created by Administrator on 2026/3/1.
//
module Generator;

import std;
import Token;
import keyword;
import Parser;
import aux;

GenClass::funcResult GenClass::FunctionUnit(const sPtr<ast::FunctionDeclaration> &_funcDecl) {
    bool isCopyResult = false;
    auto funcName = std::format("@{}", _funcDecl->Name);
    auto llvmRetType = TypeToLLVM(_funcDecl->ReturnType);
    if (const auto size = type::GetSize(_funcDecl->ReturnType); size >= 16) {
        isCopyResult = true;
        llvmRetType = "void";
    } else if (size == 8) {
        llvmRetType = "i64";
    }
    size_t paramOffset = 0;
    auto functionDecl = std::format("{} {}(", llvmRetType, funcName);
    if (isCopyResult) {
        functionDecl += "ptr %0,";
        paramOffset = 1; // 参数偏移1，因为第一个参数是返回值的指针
    }
    std::vector<std::string> llvmParamTypes(_funcDecl->Parameters.size());
    for (const auto &[type,param]: std::views::zip(llvmParamTypes, _funcDecl->Parameters)) {
        auto paramType = param->VarType;
        if (const auto paramSize = type::GetSize(paramType); paramSize > 16) {
            type = "ptr noundef";
        } else if (paramSize == 8) {
            type = "i64";
        } else {
            type = TypeToLLVM(paramType);
        }
        functionDecl.append(std::format("{} %{},", type, paramOffset++));
    }
    if (_funcDecl->IsVarList) {
        functionDecl += "...";
    }
    if (functionDecl.ends_with(',')) {
        functionDecl.pop_back(); // 移除最后一个逗号
    }
    functionDecl += ")";
    return {
        isCopyResult,
        llvmRetType,
        funcName,
        "",
        llvmParamTypes,
        functionDecl
    };
}

std::string GenClass::FunctionDeclarationGenerate(const sPtr<ast::FunctionDeclaration> &_funcDecl) {
    auto funcResult = FunctionUnit(_funcDecl);
    return std::format("declare {}\n", funcResult.functionDecl);
}

GenClass::funcCall GenClass::FunctionCall(
    const sPtr<ast::FunctionCall> &_funcCall) { // 不再传入 _varName
    auto args = _funcCall->Arguments | std::views::transform([](const auto &arg) {
        return ExpressionExpand(arg);
    }) | std::ranges::to<std::vector>();
    const auto retType = _funcCall->FunctionDecl->ReturnType;
    const bool isCopyResult = type::GetSize(retType) > 16;
    const std::string llvmRetType = isCopyResult ? "void" : TypeToLLVM(retType);
    std::string preCode;
    for (const auto &arg: args) preCode += arg.code;
    auto resultVar = std::format("%{}",exprCnt++);
    std::string callLine;
    if (isCopyResult && llvmRetType != "void") {
        callLine = "%{} = "; // 留给上级填入变量名
    } else {
        callLine = std::format("{} = ",resultVar);
    }
    callLine += std::format("call {} @{}(", llvmRetType, _funcCall->FunctionDecl->Name);
    std::vector<std::string> params;
    if (isCopyResult) {
        params.emplace_back("ptr %{}"); // sret 模式下的占位符
    }
    for (const auto &[argType, argVar, _,isCopyResult]: args) {
        params.push_back(std::format("{} {}", argType, argVar));
    }
    callLine += std::ranges::views::all(params)
                | std::views::join_with(std::string(", "))
                | std::ranges::to<std::string>();
    callLine += ")\n";

    return {isCopyResult, llvmRetType,resultVar,preCode + callLine};
}

GenClass::exprResult GenClass::FunctionArg(funcArg &_funcArg, size_t _index) {
    auto argName = "%" + std::to_string(_index); // %0, %1...
    auto currentType = _funcArg.llvmType;
    auto originalType = _funcArg.originalType;
    std::string code;
    const auto resultVar = _funcArg.resultVar;
    std::string resultType = "ptr"; // 最终结果通常是栈上的指针
    if (_funcArg.isCasting) {
        code += std::format("%{} = alloca {}, align 8\n", resultVar, _funcArg.llvmType);
        code += std::format("store i64 {}, ptr %{}, align 8\n", argName, resultVar);
        resultType = "ptr";
    } else if (_funcArg.isMemoryArg) {
        auto localStackVar = std::string(resultVar);
        code += std::format("%{} = alloca {}, align {}\n", localStackVar, originalType, _funcArg.size);
        size_t typeSize = _funcArg.size;
        code += std::format("call void @{}(ptr align 8 %{}, ptr align 8 {}, i64 {}, i1 false)\n",llvmCopy,
                            localStackVar, argName, typeSize);
        resultType = "ptr";
    } else {
        auto stackVar = std::string(resultVar);
        code += std::format("%{} = alloca {}, align {}\n", stackVar, currentType, _funcArg.size);
        code += std::format("store {} {}, ptr %{}, align {}\n", currentType, argName, stackVar, _funcArg.size);
        resultType = "ptr";
    }
    exprCnt++;
    return {resultType, resultVar, code};
}

std::string GenClass::FunctionGenerate(const sPtr<ast::FunctionScope> &_func) {
    const auto decl = _func->ToDeclaration();
    std::vector<funcArg> args;
    auto funcResult = FunctionUnit(decl);
    const size_t bit = funcResult.isCopyResult ? 1 : 0;
    if (bit) {
        args.emplace_back(funcArg{false, true, type::GetSize(decl->ReturnType), "ptr", "0"});
    }
    args.reserve(decl->Parameters.size());
    for (auto &i: decl->Parameters) {
        args.emplace_back(FunctionArgAnalyze(*i));
    }
    std::string code;
    for (auto span = std::views::zip(
             std::views::iota(0ul, args.size()),
             args);
         auto [index,arg]: span | std::views::drop(bit)) {
        auto [type, resultVar, argCode,_] = FunctionArg(arg, index);
        code += argCode;
    }
    for (auto i = 0ul; i< _func->Statements.size();i+=1) {
        const auto &stmt = _func->Statements[i];
        if (auto currentSub = std::get_if<ast::SubScope>(&*stmt); currentSub && currentSub->ScopeType == ast::SubScopeType::IfBlock) {
            sPtr<ast::SubScope> nextElse = nullptr;
            if (i + 1 < _func->Statements.size()) {
                if (auto nextSub = std::get_if<ast::SubScope>(&*_func->Statements[i + 1]);
                    nextSub && nextSub->ScopeType == ast::SubScopeType::ElseBlock) {
                    nextElse = std::make_shared<ast::SubScope>(*nextSub);
                    i+=1;
                }
            }
            auto ifBlock = std::get_if<ast::SubScope>(&*stmt);
            code += ifBlockGenerate(decl, ast::Make(*ifBlock), nextElse);
            continue;
        }
        if (std::get_if<ast::ReturnStatement>(&*stmt)) {
            code += ReturnStatementGenerate(stmt, FunctionUnit(decl));
            break;
        }
        code += StatementGenerate(stmt, decl);
    }
    return std::format("define {} {{ \n{}}}\n",funcResult.functionDecl,code);
}

GenClass::funcArg GenClass::FunctionArgAnalyze(const ast::VariableStatement &_param) {
    const auto paramType = _param.VarType;
    auto llvmType = TypeToLLVM(paramType);
    if (std::get_if<type::ArrayType>(&*paramType)) {
        llvmType = "ptr";
    }
    const auto originalType = llvmType;
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
    return {isCasting, isMemoryArg, size, llvmType, originalType,_param.Name};
}


