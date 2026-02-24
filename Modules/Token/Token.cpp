//
// Created by Administrator on 2026/2/20.
//

module Token;

import std;
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

std::shared_ptr<ast::Type::CompileType> ast::ConstValue::GetType() const {
    // 假设你的字符串值存放在 value 成员中
    if (Value.empty()) {
        return {};
    }
    if (std::isdigit(Value[0]) || (Value[0] == '-' && Value.size() > 1)) {
        if (Value.find('.') != std::string::npos) {
            return std::make_shared<Type::CompileType>(Type::BaseTypes[2]);
        }
        return std::make_shared<Type::CompileType>(Type::BaseTypes[0]);
    }
    if (Value.front() == '"') {
        auto strType = std::make_shared<Type::CompileType>(Type::BaseTypes[4]);
        const auto trueType = std::make_shared<Type::PointerType>("", 1);
        trueType->Finalize(strType);
        return strType; // 假设 "void" 代表字符串类型，或者你可以定义一个专门的字符串类型
    }
    return {};
}

std::shared_ptr<ast::Type::CompileType> ast::Expression::GetType() const {
    if (!Storage) {
        return nullptr; // 理论上不应该走到这里，除非 Expression 是空的
    }

    // 使用 std::visit 访问 variant 中的数据
    return std::visit([](auto&& arg) -> std::shared_ptr<Type::CompileType> {
        using T = std::decay_t<decltype(arg)>;

        // 1. 如果是常量 (ConstValue)
        if constexpr (std::is_same_v<T, ConstValue>) {
            // 调用我们刚才实现的 ConstValue::GetType()
            return arg.GetType();
        }

        // 2. 如果是变量引用 (Variable / VariableStatement)
        else if constexpr (std::is_same_v<T, std::shared_ptr<Variable>>) {
            // 变量节点自带它在声明时被赋予的类型
            return arg ? arg->VarType : nullptr;
        }

        // 3. 如果是函数调用 (FunctionCall)
        else if constexpr (std::is_same_v<T, std::shared_ptr<FunctionCall>>) {
            // 函数调用的类型就是该函数的返回值类型 (ReturnType)
            return (arg && arg->FunctionDecl) ? arg->FunctionDecl->ReturnType : nullptr;
        }

        // 4. 如果是复合表达式 (CompositeExpression)
        else if constexpr (std::is_same_v<T, std::shared_ptr<CompositeExpression>>) {
            // 复合表达式（如 a + b）的类型由其操作数和操作符决定
            // 这里我们调用它内部实现的 GetResultType()
            return arg ? arg->GetResultType() : nullptr;
        }

        return nullptr;
    }, *Storage);
}

ast::FunctionDeclaration ast::FunctionScope::ToDeclaration() const {
    return FunctionDeclaration(
        Name,
        ReturnType,
        Parameters,
        IsVarList
    );
}
