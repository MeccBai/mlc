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
astClass::StatementTable<ast::Statement> astClass::statementParser(ContextTable<ast::VariableStatement> &_context,
                                                                   const std::string_view _statementContent) {
    if (_statementContent.starts_with("if(") ||
        _statementContent.starts_with("while(") ||
        _statementContent.starts_with("switch(") ||
        _statementContent.starts_with("do{") ||
        _statementContent.starts_with("else{") ||
        _statementContent.starts_with("{")) {
        const auto sub = subScopeParser(_context, _statementContent);
        return std::vector{std::static_pointer_cast<ast::Statement>(sub)};
    }

    if (_statementContent.starts_with("return")) {
        // 检查 return 之后是否紧跟合法边界
        // 比如：return;  return (x);  return x;
        auto trim = [](std::string_view str) {
            while (!str.empty() && std::isspace(str.front())) str.remove_prefix(1);
            while (!str.empty() && std::isspace(str.back())) str.remove_suffix(1);
            return str;
        };

        bool isStandalone = (_statementContent.length() == 6 ||
                             std::isspace(_statementContent[6]) ||
                             _statementContent[6] == '(' ||
                             _statementContent[6] == ';');

        if (isStandalone) {
            auto returnBody = _statementContent.substr(6);

            // 移除首尾空格和末尾分号（如果有的话）
            // 你应该有一个统一的 Trim 函数
            auto trimmedBody = trim(returnBody);
            if (!trimmedBody.empty() && trimmedBody.back() == ';') {
                trimmedBody.remove_suffix(1);
                trimmedBody = trim(trimmedBody);
            }

            if (trimmedBody.empty()) {
                return std::vector{std::make_shared<ast::Statement>(ast::ReturnStatement(nullptr))};
            }

            return std::vector{
                std::make_shared<ast::Statement>(
                    ast::ReturnStatement(
                        expressionParser(_context, trimmedBody)
                    ))
            };
        };
    }
    if (_statementContent.starts_with("break;")) {
        return std::vector{std::make_shared<ast::Statement>(ast::BreakStatement())};
    }
    if (_statementContent.starts_with("continue;")) {
        return std::vector{std::make_shared<ast::Statement>(ast::ContinueStatement())};
    }
    if (_statementContent.find(' ') != std::string_view::npos) {
        return variableParser(_context, _statementContent);
    }
    if (auto pos = _statementContent.find('$'); pos != std::string_view::npos && !_statementContent.starts_with('$')) {
        if (auto pos2 = _statementContent.find('='); pos2 > pos) {
            return variableParser(_context, _statementContent) ;
        }
    }
    if (const auto pos = _statementContent.find('='); pos != std::string_view::npos) {
        const auto left = _statementContent.substr(0, pos);
        auto right = _statementContent.substr(pos + 1, _statementContent.length() - pos - 2);
        auto leftExpr = expressionParser(_context, left);
        auto rightExpr = expressionParser(_context, right);
        auto leftType = leftExpr->GetType();
        auto rightType = rightExpr->GetType();
        type::ValidateType(leftType, rightType, left);
        if (!isLeftExpression(leftExpr)) {
            ErrorPrintln("{} is not a valid left-hand expression in assignment\n", left);
            std::exit(-1);
        }
        return std::vector{std::make_shared<ast::Statement>(ast::AssignStatement(leftExpr, rightExpr))};
    }
    if (_statementContent.find('(') != std::string_view::npos) {
        if (const auto pos = _statementContent.find("if("); pos == 0) {
            return std::vector{
                std::static_pointer_cast<ast::Statement>(subScopeParser(_context, _statementContent))
            };
        }
        if (const auto pos = _statementContent.find("while("); pos == 0) {

            auto sub = subScopeParser(_context, _statementContent);
            return std::vector{std::make_shared<ast::Statement>(*sub)};
        }
        if (const auto pos = _statementContent.find("switch("); pos == 0) {
            return std::vector{
                std::static_pointer_cast<ast::Statement>(subScopeParser(_context, _statementContent))
            };
        }
        if (const auto pos = _statementContent.find("do{"); pos == 0) {
            return std::vector{
                std::static_pointer_cast<ast::Statement>(subScopeParser(_context, _statementContent))
            };
        }
        const auto functionName = _statementContent.substr(0, _statementContent.find('('));
        const auto argsStr = _statementContent.substr(_statementContent.find('(') + 1,
                                                      _statementContent.length() -
                                                      functionName.length() - 1);
        std::vector<sPtr<ast::Expression> > args = argSplit(argsStr) | std::views::transform(
                                                       [this, &_context](std::string_view arg) {
                                                           return expressionParser(_context, arg);
                                                       }) | std::ranges::to<std::vector<sPtr<ast::Expression> > >();
        auto decl = std::shared_ptr<ast::FunctionDeclaration>(nullptr);

        for (const auto& function : functionSymbolTable) {
            if (function->Name != functionName) continue;

            const size_t numParams = function->Parameters.size();
            const size_t numArgs = args.size();

            // 1. 基础数量校验
            if (function->IsVarList) {
                // 可变参数函数：传入参数必须 >= 固定参数数量
                if (numArgs < numParams) {
                    ErrorPrintln("Error: Variadic function '{}' expects at least {} arguments but {} were provided\n",
                                 functionName, numParams, numArgs);
                    std::exit(-1);
                }
            } else {
                // 普通函数：数量必须严格相等
                if (numArgs != numParams) {
                    ErrorPrintln("Error: Function '{}' expects {} arguments but {} were provided\n",
                                 functionName, numParams, numArgs);
                    std::exit(-1);
                }
            }
            for (auto [exp, param] : std::views::zip(args, function->Parameters)) {
                auto exprType = exp->GetType();
                auto paramType = param->VarType;
                if (!exprType || !paramType) {
                    throw std::runtime_error("Type inference failed for argument: " + param->Name);
                }
                auto tip = std::format(R"(argument '{}' in function '{}')", param->Name, functionName);
                ValidateType(paramType, exprType, tip);
            }
            decl = function;
            break;
        }
        if (!decl) {
            ErrorPrintln("Error: Undefined function '{}'\n", functionName);
            std::exit(-1);
        }
        return std::vector{std::make_shared<ast::Statement>(
                ast::Statement(ast::FunctionCallStatement(decl, args)))};
    }
    ErrorPrintln("Invalid statement '{}'\n", _statementContent);
    std::exit(-1);
}
