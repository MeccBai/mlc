//
// Created by Administrator on 2026/2/27.
//
module Parser;
import Token;
import aux;
import :Decl;

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
        const bool isStandalone = (_statementContent.length() == 6 ||
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
    if (const auto pos = _statementContent.find('$'); pos != std::string_view::npos && !_statementContent.starts_with('$')) {
        if (const auto pos2 = _statementContent.find('='); pos2 > pos) {
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
