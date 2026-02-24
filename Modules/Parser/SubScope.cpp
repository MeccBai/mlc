//
// Created by Administrator on 2026/2/23.
//

module Parser;
import std;
import aux;

namespace ast = mlc::ast;
using size_t = std::size_t;
using astClass = mlc::parser::AbstractSyntaxTree;

astClass::caseBlock astClass::caseBlockParser(
    ContextTable<ast::VariableStatement> &_context, std::string_view statementContent) {
    const auto conditionStr = statementContent.substr(0, statementContent.find(':'));
    const auto statementsStr = statementContent.substr(statementContent.find(':') + 1);
    const auto condition = expressionParser(_context, conditionStr);
    const auto statements = seg::TokenizeFunctionBody(statementsStr) | std::views::transform(
                                [&](const std::string_view statement) {
                                    return statementParser(_context, statement);
                                }) | std::ranges::to<std::vector<ast::Statement> >();
    return {condition, statements};
}

ast::SubScope astClass::subScopeParser(ContextTable<ast::VariableStatement> &_context,
                                       const std::string_view _subScopeContent) {
    auto newContext = _context; // 创建新的上下文，初始内容为父作用域的内容

    if (_subScopeContent.find("switch(") == 0) {
        const auto pos = _subScopeContent.find("){");
        const auto condition = expressionParser(newContext, _subScopeContent.substr(7, pos - 7));
        const auto caseBlocksStr = _subScopeContent.substr(pos + 2, _subScopeContent.length() - pos - 3);
        const auto caseBlocks = split(caseBlocksStr, "case ");
        auto toBlock = [this, &newContext](const std::string_view block) {
            return caseBlockParser(newContext, block);
        };
        auto toCaseBlock = [](const caseBlock &block) {
            auto [condition,statements] = block;
            return ast::SubScope(statements, ast::SubScopeType::CaseBlock, condition);
        };
        const auto caseBlockParsed =
                caseBlocks | std::views::transform(toBlock) | std::views::transform(toCaseBlock) | std::ranges::to<
                    std::vector<ast::Statement> >();
        return ast::SubScope(caseBlockParsed, ast::SubScopeType::SwitchBlock, condition);
    }

    const auto bodyToStatements = [this, &newContext](const std::string_view body) {
        return seg::TokenizeFunctionBody(body) | std::views::transform(
                   [&newContext, this](const std::string_view statement) {
                       return statementParser(newContext, statement);
                   }) | std::ranges::to<std::vector<ast::Statement> >();
    };

    if (_subScopeContent.find("if(") != std::string_view::npos) {
        const auto pos = _subScopeContent.find("){");
        auto condition = expressionParser(newContext, _subScopeContent.substr(3, pos - 3));
        auto body = _subScopeContent.substr(pos + 2, _subScopeContent.length() - pos - 3);
        auto statements = bodyToStatements(body);
        return ast::SubScope(statements, ast::SubScopeType::IfBlock, condition);
    } else if (_subScopeContent.find("while(") != std::string_view::npos) {
        const auto pos = _subScopeContent.find("){");
        auto condition = expressionParser(newContext, _subScopeContent.substr(6, pos - 6));
        auto body = _subScopeContent.substr(pos + 2, _subScopeContent.length() - pos - 3);
        const auto statements = bodyToStatements(body);
        return ast::SubScope(statements, ast::SubScopeType::WhileBlock, condition);
    } else if (const auto start = _subScopeContent.find("do{"); start != std::string_view::npos) {
        const auto pos = _subScopeContent.rfind("}while(");
        const auto body = _subScopeContent.substr(start + 3, pos - (start + 3));
        const auto endParen = _subScopeContent.find_last_of(')');
        const auto conditionStr = _subScopeContent.substr(pos + 7, endParen - (pos + 7));
        const auto condition = expressionParser(newContext, conditionStr);
        const auto statements = bodyToStatements(body);

        return ast::SubScope(statements, ast::SubScopeType::DoWhileBlock, condition);
    } else if (_subScopeContent.find("else{")) {
        const auto body = _subScopeContent.substr(5, _subScopeContent.length() - 6);
        const auto statements = bodyToStatements(body);
        return ast::SubScope(statements, ast::SubScopeType::ElseBlock, {});
    }

    return ast::SubScope({}, {}, {});
}
