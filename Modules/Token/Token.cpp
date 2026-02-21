//
// Created by Administrator on 2026/2/20.
//

module Token;

mlc::ast::Expression::~Expression() = default;

namespace ast = mlc::ast;

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

const std::unordered_map<std::string_view, ast::BaseOperator> mlc::ast::BaseOperators = {
    {"+", BaseOperator::Add},
    {"-", BaseOperator::Sub},
    {"*", BaseOperator::Mul},
    {"/", BaseOperator::Div},
    {"%", BaseOperator::Mod},
    {"==", BaseOperator::Equal},
    {"!=", BaseOperator::NotEqual},
    {">", BaseOperator::Greater},
    {"<", BaseOperator::Less},
    {">=", BaseOperator::GreaterEqual},
    {"<=", BaseOperator::LessEqual},
    {"&&", BaseOperator::And},
    {"||", BaseOperator::Or},
    {"!", BaseOperator::Not},
    {"&", BaseOperator::BitAnd},
    {"|", BaseOperator::BitOr},
    {"^", BaseOperator::BitXor},
    {"~", BaseOperator::BitNot},
    {">>", BaseOperator::ShiftRight},
    {"<<", BaseOperator::ShiftLeft},
    {".", BaseOperator::Dot},
    {"->", BaseOperator::Arrow},
    {"[]", BaseOperator::Subscript},
    {"@", BaseOperator::AddressOf},
    {"$", BaseOperator::Dereference},
};

ast::FunctionDeclaration ast::FunctionScope::ToDeclaration() const {
    return FunctionDeclaration(
        Name,
        ReturnType,
        Parameters,
        IsVarList
    );
}
