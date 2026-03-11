//
// Created by Administrator on 2026/3/6.
//

export module Parser:Decl;

import Parser;
import std;
import Token;

std::vector<std::string_view> split(std::string_view str, std::string_view delimiter);

std::vector<std::string_view> argSplit(std::string_view str);

using astClass = mlc::parser::AbstractSyntaxTree;
template<typename type>
using sPtr = std::shared_ptr<type>;
namespace ast = mlc::ast;
using size_t = std::size_t;
namespace type = ast::Type;

export std::set operators = {
    '+', '-', '*', '/', '%',
    '=', '!', '<', '>',
    '&', '|', '^', '~', '.', '@', '$'
};

export bool isOpChar(const char c) {
    return operators.contains(c);
}


using exprParser = std::function<sPtr<ast::Expression>(astClass::ContextTable<ast::VariableStatement> &,
                                                       std::string_view)>;

astClass::StatementTable<ast::Statement> parseReturnStatement(
    const exprParser &_parse,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view _content, const sPtr<ast::FunctionDeclaration> &_currentFunc);

astClass::StatementTable<ast::Statement> parseAssignmentStatement(
    const exprParser &_parse,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view _content);

astClass::StatementTable<ast::Statement> parseFunctionCallStatement(
    const exprParser &_parse,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view _content);

sPtr<ast::Expression> parseFunctionCallExpr(
    const exprParser &_parse,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view str);

sPtr<ast::Expression> parseEnumExpr(const exprParser &_parse, std::string_view str);


bool isLeftExpression(const std::shared_ptr<ast::Expression> &_expression) {
    if (const auto *const vPtr = _expression->GetVariable(); vPtr != nullptr) {
        return true;
    }
    if (const auto *const fPtr = _expression->GetFunctionCall();
        fPtr != nullptr) {
        return false;
    }
    if (const auto *const cPtr = _expression->GetConstValue(); cPtr != nullptr) {
        return false;
    }
    if (const auto *const compPtr = _expression->GetCompositeExpression();
        compPtr != nullptr) {
        if (const auto &operators = (*compPtr)->Operators; !operators.empty()) {
            // 只有当第一个操作符是访问类操作符（. 或 ->）时，才可能是左值
            if ((*compPtr)->OperatorFirst) {
                if (operators[0] == ast::BaseOperator::Dereference) return true;
            }
            if (operators[0] == ast::BaseOperator::Dot) return true;
            if (operators[0] == ast::BaseOperator::Arrow) return true;
            if (operators[0] == ast::BaseOperator::Subscript) {
                return type::IsType<type::ArrayType>((*compPtr)->Components[0]->GetType());
            }
        }


    }
    return false;
}
