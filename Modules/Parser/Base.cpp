//
// Created by Administrator on 2026/2/27.
//
module Parser;
import Token;
import aux;

bool isLeftExpression(const std::shared_ptr<ast::Expression> &_expression) {
    if (const auto vPtr = _expression->GetVariable(); vPtr != nullptr) {
        return true;
    }
    if (const auto fPtr = _expression->GetFunctionCall();
        fPtr != nullptr) {
        return false;
    }
    if (const auto cPtr = _expression->GetConstValue(); cPtr != nullptr) {
        return false;
    }
    if (const auto compPtr = _expression->GetCompositeExpression();
        compPtr != nullptr) {
        if (auto &operators = (*compPtr)->Operators; !operators.empty() && !(*compPtr)->isOperatorFirst) {
            // 只有当第一个操作符是访问类操作符（. 或 ->）时，才可能是左值
            if (operators[0] == ast::BaseOperator::Dot) return true;
            if (operators[0] == ast::BaseOperator::Arrow) return true;
            if (operators[0] == ast::BaseOperator::Dereference) return true;
            if (operators[0] == ast::BaseOperator::Subscript) {
                auto resultType = _expression->GetType();
                return !std::holds_alternative<ast::Type::ArrayType>(*resultType);
            }
        }
    }
    return false;
}

std::vector<std::string_view> argSplit(const std::string_view str) {
    std::vector<std::string_view> results;
    int depth = 0;
    size_t start = 0;

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '(' || str[i] == '{' || str[i] == '[') depth++;
        else if (str[i] == ')' || str[i] == '}' || str[i] == ']') depth--;
        else if (str[i] == ',' && depth == 0) {
            results.push_back(str.substr(start, i - start));
            start = i + 1;
        }
    }
    results.push_back(str.substr(start)); // 别忘了最后一块
    if (*results.rbegin()->rbegin() == ';') {
        *results.rbegin() = results.back().substr(0, results.back().size() - 2);
    }
    if (*results.rbegin()->rbegin() == ')') {
        *results.rbegin() = results.back().substr(0, results.back().size() - 1);
    }
    return results;
}

std::vector<std::string_view> split(std::string_view str, std::string_view delimiter) {
    std::vector<std::string_view> result;
    size_t start = 0;
    while (true) {
        const size_t pos = str.find(delimiter, start);
        if (pos == std::string_view::npos) {
            result.push_back(str.substr(start));
            break;
        }
        result.push_back(str.substr(start, pos - start));
        start = pos + delimiter.length();
    }
    return result;
}

// ========== Statement 辅助解析函数 ==========

auto trim = [](std::string_view str) {
    while (!str.empty() && std::isspace(str.front())) str.remove_prefix(1);
    while (!str.empty() && std::isspace(str.back())) str.remove_suffix(1);
    return str;
};

// 解析 return 语句
astClass::StatementTable<ast::Statement> parseReturnStatement(
    astClass &self,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view _content) {

    auto returnBody = _content.substr(6);
    auto trimmedBody = trim(returnBody);
    if (!trimmedBody.empty() && trimmedBody.back() == ';') {
        trimmedBody.remove_suffix(1);
        trimmedBody = trim(trimmedBody);
    }
    if (trimmedBody.empty()) {
        return {std::make_shared<ast::Statement>(ast::ReturnStatement(nullptr))};
    }
    return {std::make_shared<ast::Statement>(
        ast::ReturnStatement(self.expressionParser(_context, trimmedBody)))};
}

// 解析赋值语句
astClass::StatementTable<ast::Statement> parseAssignmentStatement(
    astClass &self,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view _content) {

    const auto pos = _content.find('=');
    const auto left = _content.substr(0, pos);
    auto right = _content.substr(pos + 1, _content.length() - pos - 2);
    auto leftExpr = self.expressionParser(_context, left);
    auto rightExpr = self.expressionParser(_context, right);
    type::ValidateType(leftExpr->GetType(), rightExpr->GetType(), left);
    if (!isLeftExpression(leftExpr)) {
        ErrorPrintln("{} is not a valid left-hand expression in assignment\n", left);
        std::exit(-1);
    }
    return {std::make_shared<ast::Statement>(ast::AssignStatement(leftExpr, rightExpr))};
}

// 解析函数调用语句
astClass::StatementTable<ast::Statement> parseFunctionCallStatement(
    astClass &self,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view _content) {

    const auto functionName = _content.substr(0, _content.find('('));
    const auto argsStr = _content.substr(_content.find('(') + 1,
                                         _content.length() - functionName.length() - 1);

    std::vector<sPtr<ast::Expression>> args = argSplit(argsStr)
        | std::views::transform([&self, &_context](std::string_view arg) {
            return self.expressionParser(_context, arg);
        })
        | std::ranges::to<std::vector<sPtr<ast::Expression>>>();

    sPtr<ast::FunctionDeclaration> decl = nullptr;

    for (const auto& function : self.functionSymbolTable) {
        if (function->Name != functionName) continue;

        const size_t numParams = function->Parameters.size();
        const size_t numArgs = args.size();

        if (function->IsVarList) {
            if (numArgs < numParams) {
                ErrorPrintln("Error: Variadic function '{}' expects at least {} arguments but {} were provided\n",
                             functionName, numParams, numArgs);
                std::exit(-1);
            }
        } else {
            if (numArgs != numParams) {
                ErrorPrintln("Error: Function '{}' expects {} arguments but {} were provided\n",
                             functionName, numParams, numArgs);
                std::exit(-1);
            }
        }

        for (auto [exp, param] : std::views::zip(args, function->Parameters)) {
            auto tip = std::format(R"(argument '{}' in function '{}')", param->Name, functionName);
            type::ValidateType(param->VarType, exp->GetType(), tip);
        }
        decl = function;
        break;
    }

    if (!decl) {
        ErrorPrintln("Error: Undefined function '{}'\n", functionName);
        std::exit(-1);
    }
    return {std::make_shared<ast::Statement>(ast::FunctionCallStatement(decl, args))};
}

// ========== 主 statementParser ==========

astClass::StatementTable<ast::Statement> astClass::statementParser(ContextTable<ast::VariableStatement> &_context,
                                                                   const std::string_view _statementContent) {
    // 控制流语句
    if (_statementContent.starts_with("if(") ||
        _statementContent.starts_with("while(") ||
        _statementContent.starts_with("switch(") ||
        _statementContent.starts_with("do{") ||
        _statementContent.starts_with("else{") ||
        _statementContent.starts_with("{")) {
        return {std::static_pointer_cast<ast::Statement>(subScopeParser(_context, _statementContent))};
    }

    // return 语句
    if (_statementContent.starts_with("return")) {
        bool isStandalone = (_statementContent.length() == 6 ||
                             std::isspace(_statementContent[6]) ||
                             _statementContent[6] == '(' ||
                             _statementContent[6] == ';');
        if (isStandalone) {
            return parseReturnStatement(*this, _context, _statementContent);
        }
    }

    // break/continue
    if (_statementContent.starts_with("break;")) {
        return {std::make_shared<ast::Statement>(ast::BreakStatement())};
    }
    if (_statementContent.starts_with("continue;")) {
        return {std::make_shared<ast::Statement>(ast::ContinueStatement())};
    }

    // 变量声明 (含空格或指针类型)
    if (_statementContent.find(' ') != std::string_view::npos) {
        return variableParser(_context, _statementContent);
    }
    if (auto pos = _statementContent.find('$'); pos != std::string_view::npos && !_statementContent.starts_with('$')) {
        if (auto pos2 = _statementContent.find('='); pos2 > pos) {
            return variableParser(_context, _statementContent);
        }
    }

    // 赋值语句
    if (const auto pos = _statementContent.find('='); pos != std::string_view::npos) {
        return parseAssignmentStatement(*this, _context, _statementContent);
    }

    // 函数调用语句
    if (_statementContent.find('(') != std::string_view::npos) {
        return parseFunctionCallStatement(*this, _context, _statementContent);
    }

    ErrorPrintln("Invalid statement '{}'\n", _statementContent);
    std::exit(-1);
}
