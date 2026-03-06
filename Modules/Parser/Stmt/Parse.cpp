//
// Created by Administrator on 2026/3/6.
//
module Parser;
import Token;
import aux;
import :Decl;

astClass::StatementTable<ast::Statement> parseReturnStatement(
    astClass &self,
    astClass::ContextTable<ast::VariableStatement> &_context,
    const std::string_view _content) {

    auto returnBody = _content.substr(6);
    if (!returnBody.empty() && returnBody.back() == ';') {
        returnBody.remove_suffix(1);
    }
    if (returnBody.empty()) {
        return {std::make_shared<ast::Statement>(ast::ReturnStatement(nullptr))};
    }
    return {std::make_shared<ast::Statement>(
        ast::ReturnStatement(self.expressionParser(_context, returnBody)))};
}

// 解析赋值语句
astClass::StatementTable<ast::Statement> parseAssignmentStatement(
    astClass &self,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view _content) {

    const auto pos = _content.find('=');
    const auto left = _content.substr(0, pos);
    const auto right = _content.substr(pos + 1, _content.length() - pos - 2);
    const auto leftExpr = self.expressionParser(_context, left);
    const auto rightExpr = self.expressionParser(_context, right);
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