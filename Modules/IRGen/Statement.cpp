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
    if (auto variable = std::get_if<ast::VariableStatement>(&*_stmt)) {
        auto initRes = variable->Initializer
                           ? ExpressionExpand(variable->Initializer, variable->VarType)
                           : exprResult{"", "", ""};

        std::string code;
        if (variable->Initializer) {
            if (initRes.llvmType == "list") {
                std::string varAddr = std::format("%{}", variable->Name);
                code += std::vformat(initRes.resultVar, std::make_format_args(varAddr));
                code += initRes.code;
            } else if (initRes.isCopyResult) {
                auto size = type::GetSize(variable->VarType);
                code += initRes.code;
                code += std::format(
                    "call void @llvm.memcpy.p0.p0.i64(ptr %{}, ptr {}, i64 {}, i1 false)\n",
                    variable->Name, initRes.resultVar, size
                );
            } else {
                code += initRes.code;
                code += std::format(
                    "store {} {}, ptr %{}\n",
                    initRes.llvmType, initRes.resultVar, variable->Name
                );
            }
        }
        return code;
    }
    if (auto assign = std::get_if<ast::AssignStatement>(&*_stmt)) {
        auto leftResult = ExpressionExpand(assign->BaseValue);
        auto rightResult = ExpressionExpand(assign->Value);
        std::string code = leftResult.code + rightResult.code;
        if (rightResult.isCopyResult) {
            auto size = type::GetSize(assign->Value->GetType());
            code += std::format(
                "call void @llvm.memcpy.p0.p0.i64(ptr {}, ptr {}, i64 {}, i1 false)\n",
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
        return "";
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
        return "  ret void\n";
    }
    if (!expr) {
        ErrorPrintln("Error: Must return a value from a non-void function.\n");
        std::exit(-1);
    }
    auto exprResult = ExpressionExpand(expr);
    std::string code = exprResult.code;
    if (exprResult.isCopyResult) {
        std::string finalCode = std::vformat(exprResult.code, std::make_format_args("%0"));
        return finalCode + "  ret void\n";
    }
    return exprResult.code + std::format("  ret {} {}\n", type, exprResult.resultVar);
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

