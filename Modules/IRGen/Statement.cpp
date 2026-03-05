//
// Created by Administrator on 2026/3/3.
//
module Generator;

import std;
import Token;
import std;
import keyword;
import Parser;
import aux;


std::string GenClass::StatementGenerate(const sPtr<ast::Statement> &_stmt,
                                        const sPtr<ast::FunctionDeclaration> &_decl) {
    if (auto _variable = std::get_if<ast::VariableStatement>(&*_stmt)) {
        return LocalVariable(ast::MakeVariable(*_variable));
    }
    if (auto assign = std::get_if<ast::AssignStatement>(&*_stmt)) {
        auto leftResult = ExpressionExpand(assign->BaseValue);
        auto rightResult = ExpressionExpand(assign->Value);
        std::string code = leftResult.code + rightResult.code;
        if (rightResult.isCopyResult) {
            auto size = type::GetSize(assign->Value->GetType());
            code += std::format(
                "call void @{}(ptr {}, ptr {}, i64 {}, i1 false)\n", llvmCopy,
                leftResult.resultVar, rightResult.resultVar, size
            );
        } else {
            code += std::format(
                "store {} {}, ptr {}\n",
                rightResult.llvmType, rightResult.resultVar, leftResult.resultVar
            );
        }
        return code;
    }
    if (const auto ret = std::get_if<ast::ReturnStatement>(&*_stmt)) {
        return ReturnStatementGenerate(_stmt, FunctionUnit(_decl));
    }
    if (auto subScope = std::get_if<ast::SubScope>(&*_stmt)) {
        return SubScopeGenerate(_stmt, _decl);
    }
    if (auto funcCall = std::get_if<ast::FunctionCall>(&*_stmt)) {
        auto funcDecl = funcCall->FunctionDecl;
        std::vector<std::string> argStrings;
        for (const auto &argExpr: funcCall->Arguments) {
            const auto type = argExpr->GetType();
            auto [argType, argVar, argCode, _] = ExpressionExpand(argExpr);
            if (std::get_if<type::ArrayType>(&*type)) {
                argType = "ptr";
            }
            argStrings.emplace_back(std::format("{} {}", argType, argVar));
        }
        std::string argsList = std::views::all(argStrings)
                               | std::views::join_with(std::string(", "))
                               | std::ranges::to<std::string>();

        // 3. 构建调用签名 (针对可变参数的关键步骤！🌟)
        std::string retType = TypeToLLVM(funcDecl->ReturnType);
        std::string callSignature;

        if (funcDecl->IsVarList) {
            std::vector<std::string> fixedTypes;
            fixedTypes.reserve(funcCall->Arguments.size());
            for (const auto &p: funcCall->Arguments) {
                const auto type = p->GetType();
                if (std::get_if<type::ArrayType>(&*type)) {
                    fixedTypes.emplace_back("ptr");
                    continue;
                }
                fixedTypes.push_back(TypeToLLVM(type));
            }
            std::string fixedStr = std::views::all(fixedTypes)
                                   | std::views::join_with(std::string(", "))
                                   | std::ranges::to<std::string>();
            if (fixedStr.empty()) {
                callSignature = std::format("{} (...)", retType);
            } else {
                callSignature = std::format("{} ({}, ...)", retType, fixedStr);
            }
        } else {
            callSignature = retType;
        }

        // 4. 生成指令序列
        std::string finalCode;
        auto llvmType = retType;
        auto size = type::GetSize(funcDecl->ReturnType);
        std::string resultAddr = std::format("%{}", exprCnt++);
        finalCode += std::format("{} = alloca {}, align {}\n", resultAddr, llvmType, size);
        std::string valReg = std::format("%{}", exprCnt++);
        finalCode += std::format("{} = call {} @{}({})\n",
                                 valReg, callSignature, funcDecl->Name, argsList);
        finalCode += std::format("store {} {}, ptr {}, align {}\n", llvmType, valReg, resultAddr, size);

        return finalCode;
    }
    if (std::holds_alternative<ast::ContinueStatement>(*_stmt) || std::holds_alternative<ast::BreakStatement>(*_stmt)) {
        ErrorPrintln("Loop control statements (continue/break) are not supported in general scope\n");
        std::exit(-1);
    }
    return "\n";
}


std::string GenClass::ReturnStatementGenerate(const sPtr<ast::Statement> &_stmt,
                                              const funcResult &_func) {
    auto type = _func.llvmType;
    auto retStmt = std::get_if<ast::ReturnStatement>(&*_stmt);
    auto expr = retStmt->ReturnValue;
    if (type == "void") {
        if (expr) {
            ErrorPrintln("Error: Cannot return a value from a void function.\n");
            std::exit(-1);
        }
        return "ret void\n";
    }
    if (!expr) {
        ErrorPrintln("Error: Must return a value from a non-void function.\n");
        std::exit(-1);
    }
    auto exprResult = ExpressionExpand(expr);
    std::string code = exprResult.code;
    if (exprResult.isCopyResult) {
        std::string finalCode = std::vformat(exprResult.code, std::make_format_args("%0"));
        return finalCode + "ret void\n";
    }
    return exprResult.code + std::format("ret {} {}\n", type, exprResult.resultVar);
}

std::string GenClass::StreamControlGenerate(const sPtr<ast::Statement> &_parentScope, const sPtr<ast::Statement> &_self,
                                            const std::string_view _startLabel, const std::string_view _endLabel) {
    if (const auto parent = std::get_if<ast::SubScope>(&*_parentScope)) {
        if (parent->ScopeType == ast::SubScopeType::WhileBlock || parent->ScopeType ==
            ast::SubScopeType::DoWhileBlock) {
            if (std::get_if<ast::ContinueStatement>(&*_self)) {
                return std::format("br label %{}\n", _startLabel);
            }
            if (std::get_if<ast::BreakStatement>(&*_self)) {
                return std::format("br label %{}\n", _endLabel);
            }
        }
    }
    ErrorPrintln("Loop control statements (continue/break) are not supported in general scope\n");
    std::exit(-1);
}

std::string GenClass::caseBlockGenerate(const sPtr<ast::Statement> &_parentScope,
                                        const ast::SubScope *_caseBlock, std::string_view _endLabel,
                                        const sPtr<ast::FunctionDeclaration> &_decl) {
    auto code = std::string();
    for (const auto &stmt: _caseBlock->Statements) {
        if (std::get_if<ast::ContinueStatement>(&*stmt)) {
            ErrorPrintln("Loop control statements (continue) are not supported in case/default block\n");
            std::exit(-1);
        }
        if (std::get_if<ast::BreakStatement>(&*stmt)) {
            code += std::format("br label %{}\n", _endLabel);
            continue;
        }
        code += StatementGenerate(stmt, _decl);
    }
    return code;
}
