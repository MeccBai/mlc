//
// Created by Administrator on 2026/3/4.
//
module Generator;

import std;
import Token;
import std;
import keyword;
import Parser;
import aux;

GenClass::exprResult GenClass::conditionalExpression(const sPtr<ast::Expression> &_condition) {

}


std::string GenClass::SubScopeGenerate(const sPtr<ast::Statement> &_stmt,
                                       const sPtr<ast::FunctionDeclaration> &_decl) {
    std::string code;
    const auto subScope = std::get_if<ast::SubScope>(&*_stmt);

    switch (subScope->ScopeType) {
        case ast::SubScopeType::WhileBlock: {
            std::string condLabel = getLabel(); // 条件判定块
            std::string bodyLabel = getLabel(); // 循环体块
            std::string endLabel = getLabel(); // 循环退出块
            code += std::format("  br label %{}\n", condLabel);
            code += std::format("{}:\n", condLabel);
            auto condRes = ExpressionExpand(subScope->Condition);
            code += condRes.code;
            std::string i1Reg = std::format("%{}", exprCnt++);
            code += std::format("{} = icmp ne {} {}, 0\n", i1Reg, condRes.llvmType, condRes.resultVar);
            code += std::format("br i1 {}, label %{}, label %{}\n", i1Reg, bodyLabel, endLabel);
            code += std::format("{}:\n", bodyLabel);
            bool bodyTerminated = false;
            for (const auto &stmt: subScope->Statements) {
                if (std::get_if<ast::ReturnStatement>(&*stmt)) {
                    code += ReturnStatementGenerate(stmt, FunctionUnit(_decl));
                    bodyTerminated = true;
                    break;
                }
                if (std::get_if<ast::ContinueStatement>(&*stmt) || std::get_if<ast::BreakStatement>(&*stmt)) {
                    code += StreamControlGenerate(_stmt, stmt, condLabel, endLabel);
                    continue;
                }
                code += StatementGenerate(stmt, _decl);
            }
            if (!bodyTerminated) {
                code += std::format("br label %{}\n", condLabel);
            }
            code += std::format("{}:\n", endLabel);
        }
        break;
        case ast::SubScopeType::DoWhileBlock: {
            std::string bodyLabel = getLabel();
            std::string condLabel = getLabel();
            std::string endLabel = getLabel();
            code += std::format("br label %{}\n", bodyLabel);
            code += std::format("{}:\n", bodyLabel);
            bool bodyTerminated = false;
            for (const auto &stmt: subScope->Statements) {
                if (std::get_if<ast::ReturnStatement>(&*stmt)) {
                    code += ReturnStatementGenerate(stmt, FunctionUnit(_decl));
                    bodyTerminated = true;
                    break;
                }
                if (std::get_if<ast::ContinueStatement>(&*stmt) || std::get_if<ast::BreakStatement>(&*stmt)) {
                    code += StreamControlGenerate(_stmt, stmt, condLabel, endLabel);
                    continue;
                }
                code += StatementGenerate(stmt, _decl);
            }
            if (!bodyTerminated) {
                code += std::format("br label %{}\n", condLabel);
            }
            code += std::format("{}:\n", condLabel);
            auto condRes = ExpressionExpand(subScope->Condition);
            code += condRes.code;
            std::string i1Reg = std::format("%{}", exprCnt++);
            code += std::format("{} = icmp ne {} {}, 0\n", i1Reg, condRes.llvmType, condRes.resultVar);
            code += std::format("br i1 {}, label %{}, label %{}\n", i1Reg, bodyLabel, endLabel);
            code += std::format("{}:\n", endLabel);
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
            size_t total = subScope->Statements.size();
            for (size_t i = 0; i < total - 1; ++i) {
                auto sub = std::get_if<ast::SubScope>(&*subScope->Statements[i]);
                auto caseCondRes = ConstExpressionExpand(sub->Condition->GetType(), sub->Condition);
                std::string cmpReg = std::format("%{}", exprCnt++);
                std::string nextCheckLabel = getLabel();
                jumpCode += std::format("  {} = icmp eq {} {}, {}\n",
                                        cmpReg, condRes.llvmType, condRes.resultVar, caseCondRes);
                jumpCode += std::format("  br i1 {}, label %{}, label %{}\n",
                                        cmpReg, labels[i], nextCheckLabel);
                jumpCode += std::format("{}:\n", nextCheckLabel);
                if (i == total - 2) {
                    jumpCode += std::format("  br label %{}\n", labels.back());
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
    code += std::format("{} = icmp ne {} {}, 0\n", i1Reg, condRes.llvmType, condRes.resultVar);
    std::string falseLabel = _elseBlock ? elseLabel : mergeLabel;
    code += std::format("br i1 {}, label %{}, label %{}\n", i1Reg, thenLabel, falseLabel);
    code += std::format("{}:\n", thenLabel);
    bool thenTerminated = false; // 提出来！🌟
    for (size_t i = 0; i < _ifBlock->Statements.size(); ++i) {
        const auto &stmt = _ifBlock->Statements[i];
        if (auto currentSub = std::get_if<ast::SubScope>(&*stmt); currentSub && currentSub->ScopeType == ast::SubScopeType::IfBlock) {
            sPtr<ast::SubScope> nextElse = nullptr;
            if (i + 1 < _ifBlock->Statements.size()) {
                if (auto nextSub = std::get_if<ast::SubScope>(&*_ifBlock->Statements[i+1]);
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
        code += std::format("  br label %{}\n", mergeLabel);
    }
    if (_elseBlock) {
        code += std::format("{}:\n", elseLabel);
        bool elseTerminated = false;
        for (const auto &stmt : _elseBlock->Statements) {
            if (std::get_if<ast::ReturnStatement>(&*stmt)) {
                code += ReturnStatementGenerate(stmt, FunctionUnit(_decl));
                elseTerminated = true;
                break;
            }
            code += StatementGenerate(stmt, _decl);
        }
        if (!elseTerminated) {
            code += std::format("  br label %{}\n", mergeLabel);
        }
    }
    code += std::format("{}:\n", mergeLabel);
    return code;
}