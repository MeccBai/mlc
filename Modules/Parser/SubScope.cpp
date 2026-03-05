//
// Created by Administrator on 2026/2/23.
//

module Parser;
import std;
import aux;


std::vector<std::string_view> splitCaseBlocks(std::string_view str) {
    std::vector<std::string_view> segments;
    constexpr std::string_view p_case = "case ";
    constexpr std::string_view p_default = "default";

    const size_t first0 = str.find(p_case);
    const size_t first1 = str.find(p_default);
    size_t start = std::min(first0, first1);

    while (start != std::string_view::npos) {
        const bool isDefault = str.substr(start).starts_with(p_default);
        const size_t keywordLen = isDefault ? p_default.length() : p_case.length();
        const size_t pushStart = isDefault ? start : (start + keywordLen);
        const size_t nextP0 = str.find(p_case, start + keywordLen);
        const size_t nextP1 = str.find(p_default, start + keywordLen);
        const size_t next = std::min(nextP0, nextP1);
        if (next == std::string_view::npos) {
            segments.push_back(str.substr(pushStart));
            break;
        }
        segments.push_back(str.substr(pushStart, next - pushStart));
        start = next;
    }
    return segments;
}

size_t caseConditionParser(const std::string_view caseBlockStr) {
    if (caseBlockStr.empty()) return std::string_view::npos;

    for (size_t i = 0; i < caseBlockStr.length(); ++i) {
        if (caseBlockStr[i] == ':') {
            const bool prevIsColon = (i > 0 && caseBlockStr[i - 1] == ':');
            const bool nextIsColon = (i + 1 < caseBlockStr.length() && caseBlockStr[i + 1] == ':');
            if (!prevIsColon && !nextIsColon) {
                return i;
            }
        }
    }
    return std::string_view::npos;
}

sPtr<astClass::caseBlock> astClass::caseBlockParser(
    ContextTable<ast::VariableStatement> &_context, std::string_view statementContent) {
    const auto conditionStr = statementContent.substr(0, caseConditionParser(statementContent));
    const auto statementsStr = statementContent.substr(caseConditionParser(statementContent) + 1);
    std::shared_ptr<ast::Expression> condition;
    if (conditionStr == "default") {
        condition = std::shared_ptr<ast::Expression>({});
    } else {
        condition = expressionParser(_context, conditionStr);
    }
    const auto statementsTemp = seg::TokenizeFunctionBody(statementsStr) | std::views::transform(
                                    [&](const std::string_view statement) {
                                        return statementParser(_context, statement);
                                    }) | std::ranges::to<std::vector<std::vector<sPtr<ast::Statement> > > >();
    const auto statements = statementsTemp | std::views::join | std::ranges::to<std::vector<std::shared_ptr<
                                ast::Statement> > >();
    std::function<void(const std::shared_ptr<ast::Statement> &)> checkNoVarDef
            = [&](const std::shared_ptr<ast::Statement> &stmt) {
        if (const auto varStmt = std::get_if<ast::VariableStatement>(stmt.operator->())) {
            ErrorPrintln("Define variable : {} in case/default block is not allowed", varStmt->Name);
            std::exit(-1);
        }
        if (const auto subScope = std::get_if<ast::SubScope>(stmt.operator->())) {
            for (const auto &s: subScope->Statements) {
                checkNoVarDef(s);
            }
        }
    };

    for (const auto &stmt: statements) {
        checkNoVarDef(stmt);
    }

    return std::make_shared<caseBlock>(caseBlock{condition, statements});
}

sPtr<ast::Statement> astClass::subScopeParser(ContextTable<ast::VariableStatement> &_context,
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
        auto toCaseBlock = [](const std::shared_ptr<caseBlock> &block) {
            auto &[caseCondition,statements] = block.operator*();
            return std::make_shared<ast::Statement>(
                ast::SubScope(statements, ast::SubScopeType::CaseBlock, caseCondition));
        };
        const auto caseBlockParsed =
                caseBlocks | std::views::transform(toBlock) | std::views::transform(toCaseBlock)
                | std::ranges::to<std::vector<std::shared_ptr<ast::Statement> > >();
        if (caseBlockParsed.empty()) {
            ErrorPrintln("Error: Switch statement must contain at least one case or default block\n");
            std::exit(-1);
        }
        const auto& endCaseBlock = caseBlockParsed.back();
        if (const auto caseBlockPtr = std::get_if<ast::SubScope>(&*endCaseBlock)) {
            if (caseBlockPtr->ScopeType != ast::SubScopeType::DefaultBlock) {
                ErrorPrintln("Error: The last block in a switch statement must be a default block\n");
                std::exit(-1);
            }
        }
        return std::make_shared<ast::Statement>(
            ast::SubScope(caseBlockParsed, ast::SubScopeType::SwitchBlock, condition));
    }

    const auto bodyToStatements = [this, &newContext](const std::string_view body) {
        auto temp = seg::TokenizeFunctionBody(body) | std::views::transform(
                        [&newContext, this](const std::string_view statement) {
                            return statementParser(newContext, statement);
                        }) | std::ranges::to<std::vector<std::vector<sPtr<ast::Statement> > > >();
        return temp | std::views::join | std::ranges::to<std::vector<sPtr<ast::Statement> > >();
    };

    if (const auto start = _subScopeContent.starts_with("do{"); start) {
        const auto pos = _subScopeContent.rfind("}while(");
        const auto body = _subScopeContent.substr(3, pos - 3);
        const auto endParen = _subScopeContent.find_last_of(')');
        const auto conditionStr = _subScopeContent.substr(pos + 7, endParen - (pos + 7));
        const auto condition = expressionParser(newContext, conditionStr);
        const auto statements = bodyToStatements(body);
        return std::make_shared<ast::Statement>(ast::SubScope(statements, ast::SubScopeType::DoWhileBlock, condition));
    }

    if (_subScopeContent.starts_with("{")) {
        const auto body = _subScopeContent.substr(1, _subScopeContent.length() - 1);
        const auto statements = bodyToStatements(body);
        return std::make_shared<ast::Statement>(ast::SubScope(statements));
    }

    if (_subScopeContent.find("else{") != std::string_view::npos) {
        const auto body = _subScopeContent.substr(5, _subScopeContent.length() - 6);
        const auto statements = bodyToStatements(body);
        return std::make_shared<ast::Statement>(ast::SubScope(statements, ast::SubScopeType::ElseBlock));
    }

    auto offest = 0;
    if (_subScopeContent.starts_with("if(")) {
        offest = 3;
    } else if (_subScopeContent.starts_with("while(")) {
        offest = 6;
    }


    const auto pos = _subScopeContent.find("){");
    auto condition = expressionParser(newContext, _subScopeContent.substr(offest, pos - offest));
    auto body = _subScopeContent.substr(pos + 2, _subScopeContent.length() - pos - 3);
    const auto statements = bodyToStatements(body);
    if (offest == 3) {
        return std::make_shared<ast::Statement>(ast::SubScope(statements, ast::SubScopeType::IfBlock, condition));
    }
    return std::make_shared<ast::Statement>(ast::SubScope(statements, ast::SubScopeType::WhileBlock, condition));
}
