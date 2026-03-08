

//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;
import :Decl;

std::vector<std::string_view> initializerListSplit(std::string_view content) {
    // 1. 去掉外层的 {} 括号
    if (content.starts_with('{') && content.ends_with('}')) {
        content = content.substr(1, content.size() - 2);
    }

    std::vector<std::string_view> elements;
    size_t start = 0;
    int depth = 0; // {} 深度
    int parenDepth = 0; // () 深度
    bool inString = false;
    for (size_t i = 0; i < content.length(); ++i) {
        const char c = content[i];
        if (c == '\\' && i + 1 < content.length()) {
            i++;
            continue;
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
        variableSymbolTable.insert(std::make_shared<ast::VariableStatement>(*vptr));
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

sPtr<ast::Expression> astClass::initExprParser(std::string_view _initExpr,
                                               const sPtr<type::CompileType> &_currentType,
                                               ContextTable<ast::VariableStatement> &_context,
                                               const std::string_view _realName) {
    auto finalInitExpr = sPtr<ast::Expression>(nullptr);
    if (!_initExpr.empty()) {
        if (_initExpr.starts_with('{') and _initExpr.ends_with('}')) {
            auto parseInitList = [&](auto &self, const std::string_view expr) -> sPtr<ast::Expression> {
                if (expr.starts_with('{') and expr.ends_with('}')) {
                    const auto elementViews = initializerListSplit(expr);
                    std::vector<sPtr<ast::Expression> > elements;
                    for (auto ev: elementViews) {
                        if (ev.starts_with('{') and ev.ends_with('}')) elements.push_back(self(self, ev));
                        else elements.push_back(expressionParser(_context, ev));
                    }
                    return ast::MakeExpression(ast::MakeInitializerList(elements));
                }
                return expressionParser(_context, expr);
            };
            finalInitExpr = parseInitList(parseInitList, _initExpr);
            finalInitExpr = ast::MakeExpression(fillDefaultValue(_currentType, finalInitExpr));
        } else if (_initExpr.starts_with('"') and _initExpr.ends_with('"')) {
            std::vector<sPtr<ast::Expression>> strElements;
            const auto rawContent = _initExpr.substr(1, _initExpr.size() - 2);
            for (size_t i = 0; i < rawContent.size(); ++i) {
                std::string charBuffer;
                if (rawContent[i] == '\\' && i + 1 < rawContent.size()) {
                    charBuffer = std::string(rawContent.substr(i, 2));
                    i++;
                } else {
                    charBuffer = std::string(1, rawContent[i]);
                }
                strElements.push_back(ast::MakeExpression(
                    ast::ConstValue(charBuffer, true)
                ));
            }
            finalInitExpr = ast::MakeExpression(ast::MakeInitializerList(strElements));
            finalInitExpr = ast::MakeExpression(fillDefaultValue(_currentType, finalInitExpr));
        }else {
            finalInitExpr = expressionParser(_context, _initExpr);
            type::ValidateType(_currentType, finalInitExpr->GetType(), _realName);
        }
    } else {
        finalInitExpr = ast::MakeExpression(fillDefaultValue(_currentType, nullptr));
    }
    return finalInitExpr;
}



std::pair<std::string_view, sPtr<type::CompileType>> resolveTypeModifier(const sPtr<type::CompileType> &_baseType, std::string_view decl) {
    auto currentType = _baseType;
    size_t pLevel = 0;
    while (pLevel < decl.size() && decl[pLevel] == '$') pLevel++;
    if (pLevel > 0) {
        const auto pType = std::make_shared<type::PointerType>(pLevel);
        pType->Finalize(currentType);
        currentType = ast::Make<type::CompileType>(*pType);
        decl.remove_prefix(pLevel);
    }
    const auto bracketPos = decl.find('[');
    std::string_view realName = decl.substr(0, bracketPos);
    if (bracketPos != std::string_view::npos) {
        std::string_view suffix = decl.substr(bracketPos);
        while (!suffix.empty() && suffix[0] == '[') {
            const size_t end = suffix.find(']');
            const size_t dim = std::stoull(std::string(suffix.substr(1, end - 1)));
            currentType = ast::Make<type::CompileType>(ast::Type::ArrayType(currentType, dim));
            suffix = suffix.substr(end + 1);
        }
    }
    return {realName, currentType};
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
        auto [realName, currentType] = resolveTypeModifier(typePtr.value(), name);
        if (initExpression.empty()) {
            auto varStmt = std::make_shared<ast::VariableStatement>(realName, currentType, nullptr);
            _context.insert(varStmt);
            result.emplace_back(std::make_shared<ast::Statement>(*varStmt));
            continue;
        }
        auto finalInitExpr = initExprParser(initExpression, currentType, _context, realName);
        auto varStmt = std::make_shared<ast::VariableStatement>(realName, currentType, finalInitExpr);
        _context.insert(varStmt);
        result.emplace_back(std::make_shared<ast::Statement>(*varStmt));
    }
    return result;
}
