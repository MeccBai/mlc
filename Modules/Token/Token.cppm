//
// Created by Administrator on 2026/2/20.
//

export module Token;
export import :Type;
export import :Expression;
export import :Statement;
export import :Factory;
import std;

using size_t = std::size_t;

export template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

export template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

export namespace mlc::ast {
    const std::unordered_map<BaseOperator, std::string> BaseIROperators = {
        {BaseOperator::Add, "add"}, {BaseOperator::Sub, "sub"}, {BaseOperator::Mul, "mul"},
        {BaseOperator::Div, "div"}, {BaseOperator::Mod, "mod"},
        {BaseOperator::Equal, "eq"}, {BaseOperator::NotEqual, "ne"}, {BaseOperator::Greater, "gt"},
        {BaseOperator::Less, "lt"}, {BaseOperator::GreaterEqual, "ge"}, {BaseOperator::LessEqual, "le"},
        {BaseOperator::And, "and"}, {BaseOperator::Or, "or"},
        {BaseOperator::BitAnd, "bitand"}, {BaseOperator::BitOr, "bitor"}, {BaseOperator::BitXor, "bitxor"},
        {BaseOperator::ShiftLeft, "shl"}, {BaseOperator::ShiftRight, "shr"},
    };
}

export namespace mlc::ast::Type {
    const std::vector BaseTypes = {
        BaseType("i32", 4), BaseType("u32", 4), BaseType("f32", 4), BaseType("f64", 8),
        BaseType("i8", 1), BaseType("u8", 1), BaseType("void", 0), BaseType("i16", 2),
        BaseType("u16", 2), BaseType("i64", 8), BaseType("u64", 8), BaseType("null", 0)
    };

    const std::unordered_map<std::string, std::shared_ptr<BaseType> > BaseTypeMap = {
        {"i32", std::make_shared<BaseType>(BaseTypes[0])}, {"u32", std::make_shared<BaseType>(BaseTypes[1])},
        {"f32", std::make_shared<BaseType>(BaseTypes[2])},
        {"f64", std::make_shared<BaseType>(BaseTypes[3])}, {"i8", std::make_shared<BaseType>(BaseTypes[4])},
        {"u8", std::make_shared<BaseType>(BaseTypes[5])}, {"void", std::make_shared<BaseType>(BaseTypes[6])},
        {"i16", std::make_shared<BaseType>(BaseTypes[7])}, {"u16", std::make_shared<BaseType>(BaseTypes[8])},
        {"i64", std::make_shared<BaseType>(BaseTypes[9])}, {"u64", std::make_shared<BaseType>(BaseTypes[10])},
        {"null", std::make_shared<BaseType>(BaseTypes[11])}
    };

    std::string GetTypeName(const CompileType &type) {
        return std::visit([](auto &&t) -> std::string {
            return std::string(t.Name);
        }, type);
    }

    bool IsIntegerType(const CompileType &type) {
        return std::visit(
            overloaded{
                [](const BaseType &t) -> bool {
                    return t.Name.starts_with('i') || t.Name.starts_with('u');
                },
                [](const auto &) -> bool {
                    return false;
                }
            }, type
        );
    }
}


namespace ast = mlc::ast;
