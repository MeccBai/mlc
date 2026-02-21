//
// Created by Administrator on 2026/2/20.
//

module Token;

mlc::ast::Expression::~Expression() = default;

namespace  ast = mlc::ast;

extern const std::vector ast::Type::BaseTypes = {
    BaseType("int", 4),
    BaseType("unsigned int", 4),
    BaseType("float", 4),
    BaseType("double", 8),
    BaseType("char", 1),
    BaseType("unsigned char", 1),
    BaseType("void", 0),
    BaseType("short", 2),
    BaseType("unsigned short", 2),
    BaseType("long", 8),
    BaseType("unsigned long", 8),
};

ast::FunctionDeclaration ast::FunctionScope::ToDeclaration() const  {

    return FunctionDeclaration(
        Name,
        ReturnType,
        Parameters,
        IsVarList
    );
}