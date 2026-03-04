//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;

std::vector<std::string_view> initializerListSplit(std::string_view content) {
    // 1. 去掉外层的 {} 括号
    if (content.starts_with('{') && content.ends_with('}')) {
        content = content.substr(1, content.size() - 2);
    }

    std::vector<std::string_view> elements;
    size_t start = 0;
    int depth = 0;      // {} 深度
    int parenDepth = 0; // () 深度
    bool inString = false;
    for (size_t i = 0; i < content.length(); ++i) {
        const char c = content[i];
        if (c == '\\' && i + 1 < content.length()) {
            i++; continue;
        }
        if (c == '"') {
            inString = !inString;
        } else if (!inString) {
            if (c == '{') depth++;
            else if (c == '}') depth--;
            else if (c == '(') parenDepth++;
            else if (c == ')') parenDepth--;
        }

        if (c == ',' && depth == 0 && parenDepth == 0 && !inString) {
            auto element = content.substr(start, i - start);
            while (!element.empty() && std::isspace(element.front())) element.remove_prefix(1);
            while (!element.empty() && std::isspace(element.back())) element.remove_suffix(1);
            if (!element.empty()) elements.push_back(element);
            start = i + 1;
        }
    }
    if (start < content.length()) {
        auto element = content.substr(start);
        if (!element.empty() && element.back() == ';') element.remove_suffix(1);
        while (!element.empty() && std::isspace(element.front())) element.remove_prefix(1);
        while (!element.empty() && std::isspace(element.back())) element.remove_suffix(1);
        if (!element.empty()) elements.push_back(element);
    }

    return elements;
}
std::string_view getVariableName(const std::string_view _declaration) {
    if (_declaration.empty()) return "";
    const size_t start = _declaration.find_first_not_of('$');
    if (start == std::string_view::npos) return "";
    const size_t end = _declaration.find_first_of("[=", start);
    if (end == std::string_view::npos) {
        return _declaration.substr(start);
    }
    return _declaration.substr(start, end - start);
}

astClass::StatementTable<ast::Statement> astClass::globalVariableParser(

    const std::string_view _variableContent) {
    // 这里可以进一步解析变量声明，提取变量类型、名称和初始化表达式等信息
    auto globalContext = ContextTable<ast::VariableStatement>{};
    const auto result = variableParser(globalContext, _variableContent);
    for (const auto &var: result) {
        auto *vptr = std::get_if<ast::VariableStatement>(var.get());
        if (vptr == nullptr) {
            ErrorPrintln("Error: Expected a variable statement in global variable parser\n");
            std::exit(-1);
        }
        if (vptr->Initializer != nullptr) {
        }
        variableSymbolTable.emplace_back(std::make_shared<ast::VariableStatement>(*vptr));
    }
    return result;
}

astClass::StatementTable<ast::Statement> astClass::localVariableParser(
    ContextTable<ast::VariableStatement> &_context, std::string_view _variableContent) {
    return variableParser(_context, _variableContent);
}


struct VariablePack {
    std::string_view Type;
    std::string_view Name;
    std::string_view InitExpression;
};

std::vector<VariablePack> variablePacked(const std::string_view _variable) {
    std::vector<VariablePack> packs;
    auto pos = _variable.find(' ');
    if (pos == std::string_view::npos) {
        pos = _variable.find('$');
    }
    if (pos == std::string_view::npos) {
        ErrorPrintln("Error: Invalid variable declaration '{}'\n", _variable);
        std::exit(-1);
    }
    const std::string_view type = _variable.substr(0, pos);
    const std::string_view variableList = _variable.substr(pos);
    for (const auto variables = initializerListSplit(variableList); auto variable: variables) {
        std::string_view name;
        std::string_view initExpr;
        if (const auto equalPos = variable.find('='); equalPos != std::string_view::npos) {
            name = variable.substr(0, equalPos);
            initExpr = variable.substr(equalPos + 1);
        } else {
            name = variable;
            initExpr = "";
        }
        if (name.empty()) {
            ErrorPrintln("Error: Invalid variable declaration '{}'\n", _variable);
            std::exit(-1);
        }
        packs.push_back({type, name, initExpr});
    }
    return packs;
}


astClass::StatementTable<ast::Statement> astClass::variableParser(ContextTable<ast::VariableStatement> &_context,
                                                                  const std::string_view _variableContent) {
    auto packs = variablePacked(_variableContent);
    StatementTable<ast::Statement> result;
    for (const auto &[type, name, initExpression]: packs) {
        auto typePtr = findType(type);
        if (typePtr == std::nullopt) {
            ErrorPrintln("Error: Unknown type '{}'\n", type);
            std::exit(-1);
        }
        auto currentType = typePtr.value();
        std::string_view decl = name;
        size_t pLevel = 0;
        while (pLevel < decl.size() and decl[pLevel] == '$') pLevel++;
        if (pLevel > 0) {
            auto pType = std::make_shared<type::PointerType>(pLevel);
            pType->Finalize(currentType);
            currentType = std::make_shared<type::CompileType>(*pType);
            decl.remove_prefix(pLevel);
        }
        const auto bracketPos = decl.find('[');
        std::string_view realName = decl.substr(0, bracketPos);
        if (bracketPos != std::string_view::npos) {
            std::string_view suffix = decl.substr(bracketPos);
            while (!suffix.empty() && suffix[0] == '[') {
                const size_t end = suffix.find(']');
                const size_t dim = std::stoull(std::string(suffix.substr(1, end - 1)));
                currentType = std::make_shared<type::CompileType>(ast::Type::ArrayType(currentType, dim));
                suffix = suffix.substr(end + 1);
            }
        }

        sPtr<ast::Expression> finalInitExpr = nullptr;
        if (!initExpression.empty()) {
            if (initExpression.starts_with('{') and initExpression.ends_with('}')) {
                auto parseInitList = [&](auto &self, const std::string_view expr) -> sPtr<ast::Expression> {
                    if (expr.starts_with('{') and expr.ends_with('}')) {
                        const auto elementViews = initializerListSplit(expr);
                        std::vector<sPtr<ast::Expression> > elements;
                        for (auto ev: elementViews) {
                            if (ev.starts_with('{') and ev.ends_with('}')) elements.push_back(self(self, ev));
                            else elements.push_back(expressionParser(_context, ev));
                        }
                        return std::make_shared<ast::Expression>(
                            ast::Expression::Data(std::make_shared<ast::InitializerList>(elements)));
                    }
                    return expressionParser(_context, expr);
                };
                finalInitExpr = parseInitList(parseInitList, initExpression);
                finalInitExpr = std::make_shared<ast::Expression>(fillDefaultValue(currentType, finalInitExpr));
            } else if (initExpression.starts_with('"') and initExpression.ends_with('"')) {
                auto str = initExpression.substr(1, initExpression.size() - 2) | std::views::transform(
                               [](const char c) {
                                   return std::make_shared<ast::Expression>(
                                       ast::ConstValue(std::string_view(&c, 1), true));
                               }) | std::ranges::to<std::vector<sPtr<ast::Expression> > >();
                finalInitExpr = std::make_shared<ast::Expression>(
                    ast::Expression::Data(std::make_shared<ast::InitializerList>(str)));
                finalInitExpr = std::make_shared<ast::Expression>(fillDefaultValue(currentType, finalInitExpr));
            } else {
                finalInitExpr = expressionParser(_context, initExpression);
                type::ValidateType(currentType, finalInitExpr->GetType(), realName);
            }
        } else {
            finalInitExpr = std::make_shared<ast::Expression>(fillDefaultValue(currentType, nullptr));
        }
        auto varStmt = std::make_shared<ast::VariableStatement>(realName, currentType, finalInitExpr);
        _context.emplace_back(varStmt);
        result.push_back(std::make_shared<ast::Statement>(*varStmt));
    }
    return result;
}

bool ast::ConstExpressionCheck(const std::shared_ptr<Expression> &_expr) {
    const auto data = &(*_expr->Storage);
    if (std::get_if<ConstValue>(data) != nullptr) {
        return true;
    }
    if (const auto funcCall = std::get_if<sPtr<FunctionCall> >(data); funcCall != nullptr) {
        return std::ranges::all_of((*funcCall)->Arguments, ConstExpressionCheck);
    }
    if (const auto compExpr = std::get_if<sPtr<CompositeExpression> >(data); compExpr != nullptr) {
        return std::ranges::all_of((*compExpr)->Components, ConstExpressionCheck);
    }
    if (const auto varPtr = std::get_if<sPtr<Variable> >(data); varPtr != nullptr) {
        return (*varPtr)->Initializer != nullptr && ConstExpressionCheck((*varPtr)->Initializer);
    }
    return false;
}
