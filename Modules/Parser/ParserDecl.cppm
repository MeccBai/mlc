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
    return std::visit(overloaded{
    [&](const type::sPtr<ast::Variable>&) {
        return true;
    },
    [&](const type::sPtr<ast::CompositeExpression>& comp) {
        if (comp->Operators.empty()) return false;

        const auto firstOp = comp->Operators[0];
        if (comp->OperatorFirst && firstOp == ast::BaseOperator::Dereference) {
            return true;
        }
        if (firstOp == ast::BaseOperator::Dot || firstOp == ast::BaseOperator::Arrow) {
            return true;
        }
        if (firstOp == ast::BaseOperator::Subscript) {
            return type::IsArrayOrPointer(comp->Components[0]->GetType());
        }
        return false;
    },
    [&](const auto&) {
        return false;
    }
}, *_expression->Storage);
}
