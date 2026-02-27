//
// Created by Administrator on 2026/2/20.
//

module Token;

import :Type;
import :Statement;
import :Expression;

import aux;
import std;
import Parser;
namespace ast = mlc::ast;
ast::Expression::~Expression() = default;



std::string baseOperator(ast::BaseOperator _op) {
    for (const auto &[token, op]: ast::BaseOperators) {
        if (op == _op) return std::string(token);
    }
    return "UnknownOperator";
}



std::string ast::Type::ArrayType::GetTypeName() const {
    std::string baseName = std::visit([](auto &&arg) -> std::string {
        // 这里调用各类型的 GetTypeName()
        return arg.Name;
    }, *BaseType);
    return std::format("{}[{}]", baseName, Length);
}


std::shared_ptr<ast::Type::CompileType> ast::ConstValue::GetType() const {
    if (Value.empty()) {
        return {};
    }
    if (IsChar) {
        return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i8"));
    }
    if (Value == "nullptr") {
        return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("null"));
    }
    if (std::isdigit(Value[0]) || (Value[0] == '-' && Value.size() > 1)) {
        if (Value.find('.') != std::string::npos) {
            return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("f64"));
        }
        return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i32"));
    }
    if (Value.front() == '"') {
        const auto strType = std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i8"));
        const auto trueType = std::make_shared<Type::PointerType>(strType,1);
        return std::make_shared<Type::CompileType>(*trueType);
    }
    if (Value.front() == '\'') {
        return std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i8"));
    }
    return {};
}

void ast::Type::ValidateType(const std::shared_ptr<CompileType> &targetType,
                            const std::shared_ptr<CompileType> &actualType,
                            const std::string_view contextInfo) {
    if (!targetType || !actualType) {
        ErrorPrintln("Compiler internal error.\n", contextInfo);
        std::exit(-1);
    }
    auto getName = [](const CompileType &type) -> std::string {
        return std::visit([](auto &&t) -> std::string {
            return std::string(t.Name);
        }, type);
    };
    std::string expectedName = getName(*targetType);
    const auto targetPtr = std::get_if<PointerType>(&*targetType);
    if (const auto actualPtr = std::get_if<BaseType>(&*actualType);
        targetPtr && actualPtr && actualPtr->Name == "null") {
        return;
    }

    if (std::string actualName = getName(*actualType); expectedName != actualName) {
        ErrorPrintln("Error: Type mismatch for {}. Expected '{}', got '{}'\n",
                     contextInfo, expectedName, actualName);
        std::exit(-1);
    }
}

// 将复杂的复合表达式逻辑抽离，方便单独挂断点调试
std::shared_ptr<ast::Type::CompileType> handleCompositeType(
    const std::shared_ptr<ast::CompositeExpression> &arg) {
    if (!arg || arg->Components.empty()) return nullptr;
    if (arg->Operators[0] == ast::BaseOperator::Subscript) {
        const auto currentType = arg->Components[0]->GetType();
        if (!currentType) return nullptr;
        return std::visit([]<typename T0>(T0 &&type) -> std::shared_ptr<ast::Type::CompileType> {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, ast::Type::ArrayType>) {
                return type.BaseType;
            } else if constexpr (std::is_same_v<T, ast::Type::PointerType>) {
                return type.BaseType;
            }
            return nullptr;
        }, *currentType);
    }

    if (arg->Operators.size() == 1 && arg->Components.size() == 1) {
        const auto op = arg->Operators[0];
        const auto subType = arg->Components[0]->GetType();
        if (!subType) return nullptr;
        if (op == ast::BaseOperator::AddressOf) {
            if (const auto *ptrData = std::get_if<ast::Type::PointerType>(&(*subType))) {
                const auto newLevel = ptrData->PointerLevel;
                auto returnPtr = std::make_shared<ast::Type::CompileType>(
                    ast::Type::PointerType(newLevel));
                auto &tempPointer = std::get<ast::Type::PointerType>(*returnPtr);
                tempPointer.Finalize(subType);
                return returnPtr;
            }
            auto returnPtr = std::make_shared<ast::Type::CompileType>(
                ast::Type::PointerType(subType, 1));
            auto &tempPointer = std::get<ast::Type::PointerType>(*returnPtr);
            tempPointer.Finalize(subType);
            return returnPtr;
        }
    }

    // 单目运算逻辑
    if (arg->Operators.size() == 1 && arg->Components.size() == 2) {
        const auto op = arg->Operators[0];
        auto subType = arg->Components[0]->GetType();
        if (!subType) return nullptr;
        if (op == ast::BaseOperator::Add || op == ast::BaseOperator::Sub ||
            op == ast::BaseOperator::Mul || op == ast::BaseOperator::Div) {
            // 2. 地毯式检查所有组件
            for (auto &exp: arg->Components) {
                auto type = exp->GetType();
                if (!type) continue;
                // 3. 核心安检：只要任何一个组件是 PointerType，立刻“枪毙”
                if (std::holds_alternative<ast::Type::PointerType>(*type)) {
                    ErrorPrintln("MLC Semantic Error: Pointer arithmetic is strictly forbidden! \n"
                                 "Cannot use operator '{}' with pointer types.", baseOperator(op));
                    std::exit(-1);
                }
            }
        }

        if (arg->Operators.size() == 1 && arg->Components.size() == 2) {
            const auto baseOp = arg->Operators[0];
            const auto leftType = arg->Components[0]->GetType();
            if (!leftType) return nullptr;
            if (baseOp == ast::BaseOperator::Arrow) {
                if (const auto *ptrData = std::get_if<ast::Type::PointerType>(&(*leftType))) {
                    const auto baseType = ptrData->BaseType;
                    if (const auto baseStruct = std::get_if<ast::Type::StructDefinition>(&(*baseType))) {
                        const auto &dataVariant = *(arg->Components[1]->Storage);
                        const auto memberAccess = *std::get_if<std::shared_ptr<ast::MemberAccess> >(&dataVariant);
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
            if (baseOp == ast::BaseOperator::Dot) {
                if (const auto *baseStruct = std::get_if<ast::Type::StructDefinition>(&(*leftType))) {
                    const auto &dataVariant = *(arg->Components[1]->Storage);
                    if (const auto *mAccessPtr = std::get_if<std::shared_ptr<ast::MemberAccess> >(&dataVariant);
                        mAccessPtr && *mAccessPtr) {
                        if (const auto memberAccess = *mAccessPtr; memberAccess->Index < baseStruct->Members.size()) {
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
                    ErrorPrintln("Error: Left side of '.' must be a struct instance.\n");
                    std::exit(-1);
                }
            }
            if (op == ast::BaseOperator::Dereference) {
                const auto argSubType = arg->Components[0]->GetType();
                if (!argSubType) return nullptr;
                if (auto *ptrData = std::get_if<ast::Type::PointerType>(&(*argSubType))) {
                    return ptrData->BaseType;
                } else {
                    ErrorPrintln("Error: Cannot dereference a non-pointer type.\n");
                    std::exit(-1);
                }
            }
        }
        return subType;
    }
    return arg->Components[0]->GetType();
}

std::shared_ptr<ast::Type::CompileType> ast::Expression::GetType() const {
    if (!Storage) return nullptr;

    return std::visit([this]<typename T0>(T0 &&arg) -> std::shared_ptr<Type::CompileType> {
        using T = std::decay_t<T0>;
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


void ast::VariableStatement::InitListValidCheck() const {
    if (!Initializer) {
        return;
    }
    const auto listPtr = std::get_if<std::shared_ptr<InitializerList> >(&*(Initializer->Storage));
    if (!listPtr) {
        return;
    }
    const auto initializerList = *listPtr;

    const auto type = this->VarType;
    const auto structType = std::get_if<Type::StructDefinition>(&(*type));
    const auto arrayType = std::get_if<Type::ArrayType>(&(*type));
    if (!structType && !arrayType) {
        ErrorPrintln("Error: Initializer list can only be used for struct or array types.\n");
        std::exit(-1);
    }

    if (arrayType || structType) {
        // 定义递归 Lambda
        auto recursiveCheck = [&](auto &self,
                                  const std::shared_ptr<Type::CompileType> &target,
                                  const std::shared_ptr<Expression> &init) -> void {
            if (const auto listPtrTemp = std::get_if<std::shared_ptr<InitializerList> >(&*(init->Storage))) {
                const auto &list = *listPtrTemp;

                // 情况 A: 目标是数组
                if (const auto arr = std::get_if<Type::ArrayType>(&(*target))) {
                    if (list->Values.size() > arr->Length) {
                        ErrorPrintln("Error: Too many initializers (expected {}, got {}).\n", arr->Length,
                                     list->Values.size());
                        std::exit(-1);
                    }
                    for (const auto &val: list->Values) {
                        self(self, arr->BaseType, val); // 递归进入下一层
                    }
                }
                // 情况 B: 目标是结构体
                else if (const auto str = std::get_if<Type::StructDefinition>(&(*target))) {
                    if (list->Values.size() > str->Members.size()) {
                        ErrorPrintln("Error: Struct '{}' has only {} members.\n", str->Name, str->Members.size());
                        std::exit(-1);
                    }
                    for (size_t i = 0; i < list->Values.size(); ++i) {
                        self(self, str->Members[i].Type, list->Values[i]); // 递归进入成员
                    }
                } else {
                    ErrorPrintln("Error: Cannot use initializer list for non-composite type.\n");
                    std::exit(-1);
                }
            } else {
                // 2. 触底反弹：这已经是一个具体的表达式了，执行最终校验
                auto typeName = std::visit([](auto &&t) { return t.Name; }, *target);
                auto tip = std::format("element of type '{}'", typeName);
                ValidateType(target, init->GetType(), tip);
            }
        };
        recursiveCheck(recursiveCheck, this->VarType, this->Initializer);
    }
}

ast::FunctionDeclaration ast::FunctionScope::ToDeclaration() const {
    return FunctionDeclaration(Name, ReturnType, Parameters, IsVarList);
}

size_t ast::Type::StructDefinition::Size() const {
    size_t currentOffset = 0;
    size_t maxAlign = 1;
    for (const auto &[name, type]: Members) {
        size_t mSize = std::visit([](auto &&t) { return t.Size(); }, *type);
        size_t mAlign = mSize;
        currentOffset = (currentOffset + mAlign - 1) & ~(mAlign - 1);
        currentOffset += mSize;
        if (mAlign > maxAlign) maxAlign = mAlign;
    }
    return (maxAlign + currentOffset - 1ul) & ~(maxAlign - 1ul);
}


size_t ast::Type::ArrayType::Size() const {
    const size_t baseSize = std::visit([](auto &&t) { return t.Size(); }, *BaseType);
    return baseSize * Length;
}
