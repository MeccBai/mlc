//
// Created by Administrator on 2026/3/6.
//
module Parser;
import Token;
import aux;
import :Decl;

astClass::StatementTable<ast::Statement> parseReturnStatement(
    const exprParser &_parse,
    astClass::ContextTable<ast::VariableStatement> &_context,
    const std::string_view _content, const sPtr<ast::FunctionDeclaration> &_currentFunc) {
    auto returnBody = _content.substr(6);
    if (!returnBody.empty() && returnBody.back() == ';') {
        returnBody.remove_suffix(1);
    }
    if (returnBody.empty()) {
        return {std::make_shared<ast::Statement>(ast::ReturnStatement(nullptr))};
    }

    auto returnExpr = _parse(_context, returnBody);
    type::ValidateType(_currentFunc->ReturnType, returnExpr->GetType(),
                       std::format("return statement in function '{}'", _currentFunc->Name));
    return {
        std::make_shared<ast::Statement>(
            ast::ReturnStatement(_parse(_context, returnBody)))
    };
}

// 解析赋值语句
astClass::StatementTable<ast::Statement> parseAssignmentStatement(
    const exprParser &_parse,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view _content) {
    const auto pos = _content.find('=');
    const auto left = _content.substr(0, pos);
    const auto right = _content.substr(pos + 1, _content.length() - pos - 2);
    const auto leftExpr = _parse(_context, left);
    const auto rightExpr = _parse(_context, right);
    type::ValidateType(leftExpr->GetType(), rightExpr->GetType(), left);
    if (!isLeftExpression(leftExpr)) {
        ErrorPrintln("{} is not a valid left-hand expression in assignment\n", left);
        std::exit(-1);
    }
    return {std::make_shared<ast::Statement>(ast::AssignStatement(leftExpr, rightExpr))};
}

// 解析函数调用语句
astClass::StatementTable<ast::Statement> astClass::parseFunctionCallStatement(
    ContextTable<ast::VariableStatement> &_context,
    std::string_view _content) {
    const auto functionName = _content.substr(0, _content.find('('));
    const auto argsStr = _content.substr(_content.find('(') + 1,
                                         _content.length() - functionName.length() - 3);

    std::vector<sPtr<ast::Expression> > args;
    if (!argsStr.empty()) {
        args = argSplit(argsStr)
        | std::views::transform([&_context, this](const std::string_view arg) {
            return expressionParser(_context, arg);
        })
        | std::ranges::to<std::vector<sPtr<ast::Expression> > >();
    }


    sPtr<ast::FunctionDeclaration> decl = nullptr;

    for (const auto &function: functionSymbolTable) {
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

        for (auto [exp, param]: std::views::zip(args, function->Parameters)) {
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
