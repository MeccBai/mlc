//
// Created by Administrator on 2026/3/4.
//
module Generator;

import std;
import Token;
import keyword;
import Parser;
import aux;

std::string constConditionExpand(const std::string_view _true,const std::string_view _false,ast::ConstValue * _constVal) {
    const auto type = _constVal->Type;
    const auto llvmType = GenClass::TypeToLLVM(type);
    const auto value = _constVal->Value;
    bool isTrue = false;
    if (llvmType.starts_with('i') || llvmType.starts_with('u')) {
        if (value != "0") {
            isTrue = true;
        }
    }
    if (llvmType.starts_with('f')) {
        double num = 0.0;
        if (auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), num); ec == std::errc{}) {
            isTrue = (num != 0.0);
        } else {
            isTrue = false;
        }
    }
    std::string target = isTrue ? std::string(_true) : std::string(_false);
    return std::format("br label %{}\n", target);
}

std::string varConditionExpand(const std::string_view _true,const std::string_view _false,ast::Variable * _var) {
    auto llvmType = GenClass::TypeToLLVM(_var->VarType);
    std::string code;
    std::string valReg = std::format("%{}", GenClass::exprCnt++);
    code += std::format("{} = load {}, ptr %{}\n", valReg, llvmType, _var->Name);
    std::string boolReg = std::format("%{}", GenClass::exprCnt++); // 💡 必须是新的寄存器
    if (llvmType.starts_with('i') || llvmType.starts_with('u')) {
        code += std::format("{} = icmp ne {} {}, 0\n", boolReg, llvmType, valReg);
    } else if (llvmType.starts_with('f')) {
        code += std::format("{} = fcmp une {} {}, 0.0\n", boolReg, llvmType, valReg);
    } else if (llvmType == "ptr") {
        code += std::format("{} = icmp ne ptr {}, null\n", boolReg, valReg);
    }
    code += std::format("br i1 {}, label %{}, label %{}\n", boolReg, _true, _false);
    return code;
}

std::string funcConditionExpand(const std::string_view _true,const std::string_view _false,ast::FunctionCall * _funcCall) {
    auto funcDecl = _funcCall->FunctionDecl;
    auto [isCopyResult, llvmType, resultVar, callCode] = GenClass::FunctionCall(
        std::make_shared<ast::FunctionCall>(*_funcCall));
    std::string code = callCode;
    if (isCopyResult) {
        ErrorPrintln("Condition cannot be a function call that returns a large struct (>=16 bytes) "
            "because it requires memcpy to retrieve the result, "
            "which is not supported in condition expressions.\n");
        std::exit(-1);
    }
    std::string boolReg = std::format("%{}", GenClass::exprCnt++);
    if (llvmType.starts_with('i') || llvmType.starts_with('u')) {
        code += std::format("{} = icmp ne {} {}, 0\n", boolReg, llvmType, resultVar);
    }
    if (llvmType.starts_with('f')) {
        code += std::format("{} = fcmp une {} {}, 0.0\n", boolReg, llvmType, resultVar);
    } else if (llvmType == "ptr") {
        code += std::format("{} = icmp ne ptr {}, null\n", boolReg, resultVar);
    }
    code += std::format("br i1 {}, label %{}, label %{}\n", boolReg, _true, _false);
    return code;
}

std::string compositeConditionExpand(const std::string_view _true, const std::string_view _false, const sPtr<ast::Expression> _condition) {
    auto [llvmType, resultVar, code, isCopyResult] = GenClass::ExpressionExpand(_condition);
    std::string boolReg = std::format("%{}", GenClass::exprCnt++);
    if (isCopyResult) {
        ErrorPrintln("Condition cannot be a composite expression that results in a large struct (>=16 bytes) "
            "because it requires memcpy to retrieve the result, "
            "which is not supported in condition expressions.\n");
        std::exit(-1);
    }
    if (llvmType.starts_with('i') || llvmType.starts_with('u')) {
        code += std::format("{} = icmp ne {} {}, 0\n", boolReg, "i1", resultVar);
    } else if (llvmType.starts_with('f')) {
        code += std::format("{} = fcmp une {} {}, 0.0\n", boolReg, "i1", resultVar);
    } else if (llvmType == "ptr") {
        code += std::format("{} = icmp ne ptr {}, null\n", boolReg, resultVar);
    }
    code += std::format("br i1 {}, label %{}, label %{}\n", boolReg, _true, _false);
    return code;
}

std::string GenClass::ConditionExpression(const expr &_condition, const std::string_view _true,
                                          const std::string_view _false) {
    if (auto *const constVal = _condition->GetConstValue()) {
        return constConditionExpand(_true, _false, constVal);
    }
    if (const auto *const varExprPtr = _condition->GetVariable()) {
        return varConditionExpand(_true, _false, varExprPtr->get());
    }
    if (const auto *const funcExprPtr = _condition->GetFunctionCall()) {
        return funcConditionExpand(_true, _false, funcExprPtr->get());
    }
    if (_condition->GetCompositeExpression()) {
        return compositeConditionExpand(_true, _false,  _condition);
    }
    ErrorPrintln("Error: Unsupported expression type in condition.\n");
    std::exit(-1);
}

std::string handleWhileBlock(auto &getLabel, ast::SubScope *_subScope, const sPtr<ast::FunctionDeclaration> &_decl,
                             const sPtr<ast::Statement> &_stmt) {
    std::string code;
    std::string condLabel = getLabel(); // 条件判定块
    std::string bodyLabel = getLabel(); // 循环体块
    std::string endLabel = getLabel(); // 循环退出块
    code += std::format("br label %{}\n", condLabel);
    code += std::format("{}:\n", condLabel);
    auto condRes = GenClass::ExpressionExpand(_subScope->Condition);
    code += condRes.code;
    code += GenClass::ConditionExpression(_subScope->Condition, bodyLabel, endLabel);
    code += std::format("{}:\n", bodyLabel);
    bool bodyTerminated = false;
    for (const auto &stmt: _subScope->Statements) {
        if (std::get_if<ast::ReturnStatement>(&*stmt)) {
            code += GenClass::ReturnStatementGenerate(stmt, GenClass::FunctionUnit(_decl));
            bodyTerminated = true;
            break;
        }
        if (std::get_if<ast::ContinueStatement>(&*stmt) || std::get_if<ast::BreakStatement>(&*stmt)) {
            code += GenClass::StreamControlGenerate(_stmt, stmt, condLabel, endLabel);
            continue;
        }
        code += GenClass::StatementGenerate(stmt, _decl);
    }
    if (!bodyTerminated) {
        code += std::format("br label %{}\n", condLabel);
    }
    code += std::format("{}:\n", endLabel);
    return code;
}

std::string handleDoWhileBlock(auto & getLabel,ast::SubScope * _subScope,const sPtr<ast::FunctionDeclaration> &_decl,const sPtr<ast::Statement> &_stmt) {
    std::string code;
    std::string bodyLabel = getLabel();
    std::string condLabel = getLabel();
    std::string endLabel = getLabel();
    code += std::format("br label %{}\n", bodyLabel);
    code += std::format("{}:\n", bodyLabel);
    bool bodyTerminated = false;
    for (const auto &stmt: _subScope->Statements) {
        if (std::get_if<ast::ReturnStatement>(&*stmt)) {
            code += GenClass::ReturnStatementGenerate(stmt, GenClass::FunctionUnit(_decl));
            bodyTerminated = true;
            break;
        }
        if (std::get_if<ast::ContinueStatement>(&*stmt) || std::get_if<ast::BreakStatement>(&*stmt)) {
            code += GenClass::StreamControlGenerate(_stmt, stmt, condLabel, endLabel);
            continue;
        }
        code += GenClass::StatementGenerate(stmt, _decl);
    }
    if (!bodyTerminated) {
        code += std::format("br label %{}\n", condLabel);
    }
    code += std::format("{}:\n", condLabel);
    code += GenClass::ConditionExpression(_subScope->Condition, bodyLabel, endLabel);
    code += std::format("{}:\n", endLabel);
    return code;
}

std::string GenClass::SubScopeGenerate(const sPtr<ast::Statement> &_stmt,
                                       const sPtr<ast::FunctionDeclaration> &_decl) {
    std::string code;
    switch (const auto subScope = std::get_if<ast::SubScope>(&*_stmt); subScope->ScopeType) {
        case ast::SubScopeType::WhileBlock: {
            code += handleWhileBlock(getLabel, subScope, _decl, _stmt);
        }
        break;
        case ast::SubScopeType::DoWhileBlock: {
            code += handleDoWhileBlock(getLabel, subScope, _decl, _stmt);
        }
        break;
        case ast::SubScopeType::SwitchBlock: {
            auto caseCode = std::string();
            auto endLabel = getLabel();
            auto labels = std::vector<std::string>();
            auto condRes = ExpressionExpand(subScope->Condition);
            std::string jumpCode = condRes.code;
            for (const auto &stmt: subScope->Statements) {
                auto sub = std::get_if<ast::SubScope>(&*stmt);
                auto currentLabel = getLabel();
                labels.push_back(currentLabel);
                caseCode += std::format("{}:\n{}\n", currentLabel,
                                        caseBlockGenerate(_stmt, sub, endLabel, _decl));
            }
            const size_t total = subScope->Statements.size();
            for (size_t i = 0; i < total - 1; ++i) {
                auto *sub = std::get_if<ast::SubScope>(&*subScope->Statements[i]);
                auto caseCondRes = ConstExpressionExpand(sub->Condition->GetType(), sub->Condition,true);
                std::string cmpReg = std::format("%{}", exprCnt++);
                std::string nextCheckLabel = getLabel();
                jumpCode += std::format("{} = icmp eq {} {}, {}\n",
                                        cmpReg, condRes.llvmType, condRes.resultVar, caseCondRes);
                jumpCode += std::format("br i1 {}, label %{}, label %{}\n",
                                        cmpReg, labels[i], nextCheckLabel);
                jumpCode += std::format("{}:\n", nextCheckLabel);
                if (i == total - 2) {
                    jumpCode += std::format("br label %{}\n", labels.back());
                }
            }
            return std::format("{}\n{}\n{}:\n", jumpCode, caseCode, endLabel);
        }
        case ast::SubScopeType::AnonymousBlock: {
            for (const auto &stmt: subScope->Statements) {
                code += StatementGenerate(stmt, _decl);
            }
        }
        case ast::SubScopeType::ElseBlock: {
            ErrorPrintln("Error, else block should be handled in if block generation");
            std::exit(-1);
        }
        default:
            ErrorPrintln("Compiler Internal Error: Unknown SubScopeType.");
            std::exit(-1);
    }
    return code;
}

std::string GenClass::ifBlockGenerate(const sPtr<ast::FunctionDeclaration> &_decl,
                                      const sPtr<ast::SubScope> &_ifBlock,
                                      const sPtr<ast::SubScope> &_elseBlock) {
    std::string code;
    auto thenLabel = getLabel();
    auto elseLabel = getLabel();
    auto mergeLabel = getLabel();
    auto condRes = ExpressionExpand(_ifBlock->Condition);
    code += condRes.code;
    auto i1Reg = std::format("%{}", exprCnt++);
    std::string falseLabel = _elseBlock ? elseLabel : mergeLabel;
    code += ConditionExpression(_ifBlock->Condition, thenLabel, falseLabel);
    code += std::format("{}:\n", thenLabel);
    bool thenTerminated = false;
    for (size_t i = 0; i < _ifBlock->Statements.size(); ++i) {
        const auto &stmt = _ifBlock->Statements[i];
        if (auto currentSub = std::get_if<ast::SubScope>(&*stmt);
            currentSub && currentSub->ScopeType == ast::SubScopeType::IfBlock) {
            sPtr<ast::SubScope> nextElse = nullptr;
            if (i + 1 < _ifBlock->Statements.size()) {
                if (auto nextSub = std::get_if<ast::SubScope>(&*_ifBlock->Statements[i + 1]);
                    nextSub && nextSub->ScopeType == ast::SubScopeType::ElseBlock) {
                    nextElse = std::make_shared<ast::SubScope>(*nextSub);
                    i++;
                }
            }
            auto ifBlock = std::get_if<ast::SubScope>(&*stmt);
            code += ifBlockGenerate(_decl, ast::Make(*ifBlock), nextElse);
            continue;
        }
        if (std::get_if<ast::ReturnStatement>(&*stmt)) {
            code += ReturnStatementGenerate(stmt, FunctionUnit(_decl));
            thenTerminated = true; // 标记已终止！
            break;
        }
        code += StatementGenerate(stmt, _decl);
    }
    if (!thenTerminated) {
        code += std::format("br label %{}\n", mergeLabel);
    }
    if (_elseBlock) {
        code += std::format("{}:\n", elseLabel);
        bool elseTerminated = false;
        for (const auto &stmt: _elseBlock->Statements) {
            if (std::get_if<ast::ReturnStatement>(&*stmt)) {
                code += ReturnStatementGenerate(stmt, FunctionUnit(_decl));
                elseTerminated = true;
                break;
            }
            code += StatementGenerate(stmt, _decl);
        }
        if (!elseTerminated) {
            code += std::format("br label %{}\n", mergeLabel);
        }
    }
    code += std::format("{}:\n", mergeLabel);
    return code;
}
