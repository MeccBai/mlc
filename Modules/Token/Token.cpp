//
// Created by Administrator on 2026/2/20.
//

module Token;

mlc::ast::Expression::~Expression() = default;

namespace ast = mlc::ast;

extern const std::vector ast::Type::BaseTypes = {
    BaseType("i32", 4),
    BaseType("u32", 4),
    BaseType("f32", 4),
    BaseType("f64", 8),
    BaseType("i8", 1),
    BaseType("u8", 1),
    BaseType("void", 0),
    BaseType("i16", 2),
    BaseType("u16", 2),
    BaseType("i64", 8),
    BaseType("u64", 8),
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
