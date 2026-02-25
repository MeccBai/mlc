//
// Created by Administrator on 2026/2/23.
//

module Parser;
import std;
import aux;

namespace ast = mlc::ast;
using size_t = std::size_t;
using astClass = mlc::parser::AbstractSyntaxTree;
std::vector<std::string_view> splitCaseBlocks(std::string_view str) {
    std::vector<std::string_view> segments;
    const std::string_view patterns[] = {"case ", "default"};

    size_t start = 0;
    while (start < str.length()) {
        // 1. 寻找当前块的起点（跳过前面的空格或杂质，定位到 case 或 default）
        size_t p0 = str.find(patterns[0], start);
        size_t p1 = str.find(patterns[1], start);
        size_t currentBlockStart = std::min(p0, p1);

        if (currentBlockStart == std::string_view::npos) break;

        // 2. 从当前起点之后，寻找下一个 case 或 default 作为终点
        size_t nextP0 = str.find(patterns[0], currentBlockStart + 7); // "default" 长度 7
        size_t nextP1 = str.find(patterns[1], currentBlockStart + 7);
        size_t nextBlockStart = std::min(nextP0, nextP1);

        // 3. 切割并存入结果
        if (nextBlockStart == std::string_view::npos) {
            segments.push_back(str.substr(currentBlockStart+5)); // 最后一个块
            break;
        } else {
            segments.push_back(str.substr(currentBlockStart+5, nextBlockStart - currentBlockStart-5));
            start = nextBlockStart; // 挪动指针到下一个块开头
        }
    }
    return segments;
}
astClass::caseBlock astClass::caseBlockParser(
    ContextTable<ast::VariableStatement> &_context, std::string_view statementContent) {
    const auto conditionStr = statementContent.substr(0, statementContent.find(':'));
    const auto statementsStr = statementContent.substr(statementContent.find(':') + 1);
    ast::Expression condition;
    if (conditionStr  == "lt") {
        condition = ast::Expression({});
    }
    else {
        condition = expressionParser(_context, conditionStr);
    }
    const auto statementsTemp = seg::TokenizeFunctionBody(statementsStr) | std::views::transform(
                                [&](const std::string_view statement) {
                                    return statementParser(_context, statement);
                                }) | std::ranges::to<std::vector<std::vector<ast::Statement>> >();
    const auto statements = statementsTemp | std::views::join | std::ranges::to<std::vector<ast::Statement> >();

    return {condition, statements};
}

ast::SubScope astClass::subScopeParser(ContextTable<ast::VariableStatement> &_context,
                                       const std::string_view _subScopeContent) {
    auto newContext = _context; // 创建新的上下文，初始内容为父作用域的内容

    if (_subScopeContent.find("switch(") == 0) {
        const auto pos = _subScopeContent.find("){");
        const auto condition = expressionParser(newContext, _subScopeContent.substr(7, pos - 7));
        const auto caseBlocksStr = _subScopeContent.substr(pos + 2, _subScopeContent.length() - pos - 3);
        const auto caseBlocks = splitCaseBlocks(caseBlocksStr);
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
        auto temp = seg::TokenizeFunctionBody(body) | std::views::transform(
                   [&newContext, this](const std::string_view statement) {
                       return statementParser(newContext, statement);
                   }) | std::ranges::to<std::vector<std::vector<ast::Statement>> >();
        return temp | std::views::join | std::ranges::to<std::vector<ast::Statement> >();
    };

    if (_subScopeContent.starts_with("if(")) {
        const auto pos = _subScopeContent.find("){");
        auto condition = expressionParser(newContext, _subScopeContent.substr(3, pos - 3));
        auto body = _subScopeContent.substr(pos + 2, _subScopeContent.length() - pos - 3);
        auto statements = bodyToStatements(body);
        return ast::SubScope(statements, ast::SubScopeType::IfBlock, condition);
    } else if (_subScopeContent.starts_with("while(")) {
        const auto pos = _subScopeContent.find("){");
        auto condition = expressionParser(newContext, _subScopeContent.substr(6, pos - 6));
        auto body = _subScopeContent.substr(pos + 2, _subScopeContent.length() - pos - 3);
        const auto statements = bodyToStatements(body);
        return ast::SubScope(statements, ast::SubScopeType::WhileBlock, condition);
    } else if (const auto start = _subScopeContent.starts_with("do{"); start) {
        const auto pos = _subScopeContent.rfind("}while(");
        const auto body = _subScopeContent.substr(3, pos - 3);
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
