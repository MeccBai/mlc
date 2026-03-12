//
// Created by Administrator on 2026/3/3.
//
module Generator;

import std;
import Token;
import keyword;
import Parser;
import aux;

std::string functionCallGenerate(ast::FunctionCall *_funcCall) {
    auto funcDecl = _funcCall->FunctionDecl;
    std::string finalCode;
    std::vector<std::string> argStrings;

    for (const auto &argExpr: _funcCall->Arguments) {
        auto [argType, argVar, argCode, _] = GenClass::ExpressionExpand(argExpr);
        finalCode += argCode; // 🌟 关键：把参数计算的 IR 捡回来！

        std::string finalType = argType;
        if (std::get_if<type::ArrayType>(&*argExpr->GetType())) finalType = "ptr";
        argStrings.emplace_back(std::format("{} {}", finalType, argVar));
    }
    std::string argsList = std::views::all(argStrings)
                           | std::views::join_with(std::string(", "))
                           | std::ranges::to<std::string>();

    // 3. 构建调用签名 (针对可变参数的关键步骤！🌟)
    std::string retType = GenClass::TypeToLLVM(funcDecl->ReturnType);
    std::string callSignature;

    if (funcDecl->IsVarList) {
        std::vector<std::string> fixedTypes;
        fixedTypes.reserve(_funcCall->Arguments.size());
        for (const auto &p: _funcCall->Arguments) {
            const auto type = p->GetType();
            if (std::get_if<type::ArrayType>(&*type)) {
                fixedTypes.emplace_back("ptr");
                continue;
            }
            fixedTypes.push_back(GenClass::TypeToLLVM(type));
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

    if (retType != "void") {
        auto valReg = std::format("%{}", GenClass::exprCnt++);
        finalCode += std::format("{} = call {} @{}({})\n",
                                 valReg, callSignature, funcDecl->Name, argsList);

        auto resultAddr = std::format("%{}", GenClass::exprCnt++);
        finalCode += std::format("{} = alloca {}\n", resultAddr, retType);
        finalCode += std::format("store {} {}, ptr {}\n", retType, valReg, resultAddr);
    } else {
        finalCode += std::format("call void @{}({})\n", funcDecl->Name, argsList);
    }
    return finalCode;
}


std::string GenClass::StatementGenerate(const sPtr<ast::Statement> &_stmt,
                                        const sPtr<ast::FunctionDeclaration> &_decl) {
    auto variableStmt = [&](ast::VariableStatement &_variable) {
        return LocalVariable(ast::Make<ast::Variable>(_variable));
    };
    auto assignStmt = [](ast::AssignStatement &_assign) {
        auto leftResult = LeftExpressionExpand(_assign.BaseValue);
        const auto rightResult = ExpressionExpand(_assign.Value);
        std::string code = leftResult.code + rightResult.code;
        if (rightResult.isCopyResult) {
            auto size = type::GetSize(_assign.Value->GetType());
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
    };

    auto returnStmt = [&](ast::ReturnStatement &) {
        return ReturnStatementGenerate(_stmt, FunctionUnit(_decl));
    };

    auto subScopeStmt = [&](ast::SubScope &) {
        return SubScopeGenerate(_stmt, _decl);
    };

    auto functionCallStmt = [](ast::FunctionCall &_funcCall) {
        return functionCallGenerate(&_funcCall);
    };

    auto continueStmt = [](ast::ContinueStatement &) -> std::string {
        ErrorPrintln(
            "Loop control statements (continue/break) are not supported in general scope\n");
        std::exit(-1);
    };

    auto breakStmt = [](ast::BreakStatement &)-> std::string {
        ErrorPrintln(
            "Loop control statements (continue/break) are not supported in general scope\n");
        std::exit(-1);
    };

    return std::visit(
        overloaded{
            variableStmt, assignStmt, returnStmt, subScopeStmt,
            breakStmt, continueStmt, functionCallStmt
        },
        *_stmt);
}


std::string GenClass::ReturnStatementGenerate(const sPtr<ast::Statement> &_stmt,
                                              const funcResult &_func) {
    auto type = _func.llvmType;
    auto *retStmt = std::get_if<ast::ReturnStatement>(&*_stmt);
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
    if (const auto *const parent = std::get_if<ast::SubScope>(&*_parentScope)) {
        if (parent->ScopeType == ast::SubScopeType::WhileBlock || parent->ScopeType ==
            ast::SubScopeType::DoWhileBlock) {
            return std::visit(
                overloaded{
                    [&](ast::ContinueStatement &) {
                        return std::format("br label %{}\n", _startLabel);
                    },
                    [&](ast::BreakStatement &) {
                        return std::format("br label %{}\n", _endLabel);
                    },
                    [&](auto &) -> std::string {
                        ErrorPrintln(
                            "Loop control statements (continue/break) are not supported in non-loop scope\n");
                        std::exit(-1);
                    }
                }, *_self
            );
        }
    }
    ErrorPrintln("Loop control statements (continue/break) are not supported in general scope\n");
    std::exit(-1);
}

std::string GenClass::caseBlockGenerate(const sPtr<ast::Statement> &_parentScope,
                                        const ast::SubScope *_caseBlock, std::string_view _endLabel,
                                        const sPtr<ast::FunctionDeclaration> &_decl) {
    auto stmtGenerate = [&](const sPtr<ast::Statement> &_stmt) {
        return std::visit(
            overloaded{
                [&](ast::ContinueStatement &) -> std::string {
                    ErrorPrintln(
                        "Loop control statements (continue) are not supported in case/default block\n");
                    std::exit(-1);
                },
                [&](ast::BreakStatement &) {
                    return std::format("br label %{}\n", _endLabel);
                },
                [&](auto &) {
                    return StatementGenerate(_stmt, _decl);
                }
            },
            *_stmt);
    };

    auto code = _caseBlock->Statements | std::views::transform(stmtGenerate) | std::views::join | std::ranges::to<
                    std::string>();
    return code;
}
