//
// Created by Administrator on 2026/2/20.
//

module Token;
import aux;
import std;
mlc::ast::Expression::~Expression() = default;


namespace ast = mlc::ast;
std::string baseOperator(ast::BaseOperator _op) {
    for (const auto &[token, op] : ast::BaseOperators) {
        if (op == _op) return std::string(token);
    }
    return "UnknownOperator";
}

extern const std::vector ast::Type::BaseTypes = {
    BaseType("i32", 4), BaseType("u32", 4), BaseType("f32", 4), BaseType("f64", 8),
    BaseType("i8", 1), BaseType("u8", 1), BaseType("void", 0), BaseType("i16", 2),
    BaseType("u16", 2), BaseType("i64", 8), BaseType("u64", 8),
};

const std::unordered_map<std::string_view, ast::BaseOperator> mlc::ast::BaseOperators = {
    {"+", BaseOperator::Add}, {"-", BaseOperator::Sub}, {"*", BaseOperator::Mul},
    {"/", BaseOperator::Div}, {"%", BaseOperator::Mod}, {"==", BaseOperator::Equal},
    {"!=", BaseOperator::NotEqual}, {">", BaseOperator::Greater}, {"<", BaseOperator::Less},
    {">=", BaseOperator::GreaterEqual}, {"<=", BaseOperator::LessEqual}, {"&&", BaseOperator::And},
    {"||", BaseOperator::Or}, {"!", BaseOperator::Not}, {"&", BaseOperator::BitAnd},
    {"|", BaseOperator::BitOr}, {"^", BaseOperator::BitXor}, {"~", BaseOperator::BitNot},
    {">>", BaseOperator::ShiftRight}, {"<<", BaseOperator::ShiftLeft}, {".", BaseOperator::Dot},
    {"->", BaseOperator::Arrow}, {"[]", BaseOperator::Subscript}, {"@", BaseOperator::AddressOf},
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
    if (Value.front() == '\'') {
        return std::make_shared<Type::CompileType>(Type::BaseTypes[4]);
    }
    return {};
}

// 将复杂的复合表达式逻辑抽离，方便单独挂断点调试
std::shared_ptr<ast::Type::CompileType> handleCompositeType(
    const std::shared_ptr<ast::CompositeExpression> &arg) {
    if (!arg || arg->Components.empty()) return nullptr;

    if (arg->Operators.size() == 1 && arg->Components.size() == 1) {
        auto op = arg->Operators[0];
        auto subType = arg->Components[0].GetType();
        if (!subType) return nullptr;
        if (op == ast::BaseOperator::AddressOf) {
            // 调试点：观察取地址升维过程
            if (auto *ptrData = std::get_if<ast::Type::PointerType>(&(*subType))) {
                auto newLevel = ptrData->PointerLevel;
                // 这里你可以打印或者观察 ptrData->Name
                auto returnPtr = std::make_shared<ast::Type::CompileType>(
                    ast::Type::PointerType(newLevel));
                auto& tempPointer = std::get<ast::Type::PointerType>(*returnPtr);
                tempPointer.Finalize(subType);
                return returnPtr;
            }

            // 基础类型升维
            auto returnPtr= std::make_shared<ast::Type::CompileType>(
                ast::Type::PointerType("", subType, 1));
            auto& tempPointer = std::get<ast::Type::PointerType>(*returnPtr);
            tempPointer.Finalize(subType);
            return returnPtr;
        }
    }

    // 单目运算逻辑
    if (arg->Operators.size() == 1 && arg->Components.size() == 2) {
        auto op = arg->Operators[0];
        auto subType = arg->Components[0].GetType();

        if (!subType) return nullptr;

        if (op == ast::BaseOperator::Add || op == ast::BaseOperator::Sub ||
            op == ast::BaseOperator::Mul || op == ast::BaseOperator::Div) {

            // 2. 地毯式检查所有组件
            for (auto &exp : arg->Components) {
                auto type = exp.GetType();
                if (!type) continue;

                // 3. 核心安检：只要任何一个组件是 PointerType，立刻“枪毙”
                if (std::holds_alternative<ast::Type::PointerType>(*type)) {
                    ErrorPrintln("MLC Semantic Error: Pointer arithmetic is strictly forbidden! \n"
                                 "Cannot use operator '{}' with pointer types.",baseOperator(op)); // 假设你有这个转换函数
                    std::exit(-1);
                }
            }
            }

        if (arg->Operators.size() == 1 && arg->Components.size() == 2) {
            auto baseOp = arg->Operators[0];
            auto leftType = arg->Components[0].GetType();
            if (!leftType) return nullptr;

            // 处理 -> (指针访问成员)
            if (baseOp == ast::BaseOperator::Arrow) {
                // 假设对应 <->> 或 ->
                // 校验：左边必须得是指针
                if (auto *ptrData = std::get_if<ast::Type::PointerType>(&(*leftType))) {
                    // 拿到指针指向的基类型 (比如从 One$ 拿到 One)
                    auto baseType = ptrData->BaseType;
                    auto baseStruct = std::get_if<ast::Type::StructDefinition>(&(*baseType));
                    if (baseStruct) {
                        auto &dataVariant = *(arg->Components[1].Storage);
                        auto memberAccess = *std::get_if<std::shared_ptr<ast::MemberAccess> >(&dataVariant);
                        return baseStruct->Members[memberAccess->Index].Type;
                    } else {
                        ErrorPrintln("Error: '->' operator requires a pointer to a struct type.\n");
                        std::exit(-1);
                    }
                } else {
                    ErrorPrintln("Error: Left side of '->' must be a pointer.\n");
                    std::exit(-1);
                }
            }

            // 处理 . (直接访问成员)
            if (baseOp == ast::BaseOperator::Dot) {
                // 1. 校验：左边必须得是结构体实例 (StructDefinition)
                if (auto *baseStruct = std::get_if<ast::Type::StructDefinition>(&(*leftType))) {
                    // 2. 提取右侧成员访问节点
                    auto &dataVariant = *(arg->Components[1].Storage);
                    auto *mAccessPtr = std::get_if<std::shared_ptr<ast::MemberAccess> >(&dataVariant);

                    if (mAccessPtr && *mAccessPtr) {
                        auto memberAccess = *mAccessPtr;

                        // 3. 安全检查：确保索引在范围内
                        if (memberAccess->Index < baseStruct->Members.size()) {
                            // 成功推导出成员类型
                            return baseStruct->Members[memberAccess->Index].Type;
                        } else {
                            ErrorPrintln("Error: Member index {} out of bounds for struct '{}'.\n",
                                         memberAccess->Index, baseStruct->Name);
                            std::exit(-1);
                        }
                    } else {
                        ErrorPrintln("Error: Right side of '.' must be a member access.\n");
                        std::exit(-1);
                    }
                } else {
                    // 报错：左边不是结构体（可能是基础类型或者指针，那是 Arrow 的活）
                    ErrorPrintln("Error: Left side of '.' must be a struct instance.\n");
                    std::exit(-1);
                }
            }
            if (op == ast::BaseOperator::Dereference) { // 假设对应 <*>
                auto subType = arg->Components[0].GetType();
                if (!subType) return nullptr;

                // 关键逻辑：必须是指针才能解引用
                if (auto *ptrData = std::get_if<ast::Type::PointerType>(&(*subType))) {
                    // 核心：直接返回指针指向的基类型 (BaseType)
                    // 如果原本是 One$ (Level 1)，解引用后就变成了 One
                    // 如果原本是 One$$ (Level 2)，解引用后就变成了 One$
                    return ptrData->BaseType;
                } else {
                    ErrorPrintln("Error: Cannot dereference a non-pointer type.\n");
                    std::exit(-1);
                }
            }
        }
        return subType;
    }

    // 二元运算逻辑
    return arg->Components[0].GetType();
}

std::shared_ptr<ast::Type::CompileType> ast::Expression::GetType() const {
    if (!Storage) return nullptr;

    // 1. 先用 holds_alternative 或者简单的 visit 拿到类型标识
    // 为了极致的调试友好，我们直接在 visit 里根据类型分流到具体的处理函数

    return std::visit([this](auto &&arg) -> std::shared_ptr<Type::CompileType> {
        using T = std::decay_t<decltype(arg)>;

        // --- 调试点：在这里打断点，查看 T 的具体类型 ---

        if constexpr (std::is_same_v<T, ConstValue>) {
            return arg.GetType();
        } else if constexpr (std::is_same_v<T, std::shared_ptr<Variable> >) {
            return arg ? arg->VarType : nullptr;
        } else if constexpr (std::is_same_v<T, std::shared_ptr<FunctionCall> >) {
            return (arg && arg->FunctionDecl) ? arg->FunctionDecl->ReturnType : nullptr;
        } else if constexpr (std::is_same_v<T, std::shared_ptr<CompositeExpression> >) {
            // 调试点：进入复合表达式推导
            return handleCompositeType(arg);
        }

        return nullptr;
    }, *Storage);
}


ast::FunctionDeclaration ast::FunctionScope::ToDeclaration() const {
    return FunctionDeclaration(Name, ReturnType, Parameters, IsVarList);
}
