//
// Created by Administrator on 2026/3/6.
//
module Token;

import :Type;

import std;
using size_t = std::size_t;
namespace ast = mlc::ast;
namespace type=ast::Type;

std::string baseOperator(const ast::BaseOperator _op) {
    for (const auto &[token, op]: ast::BaseOperators) {
        if (op == _op) return std::string(token);
    }
    return "UnknownOperator";
}

std::shared_ptr<ast::Type::CompileType> handleCompositeType(
    const std::shared_ptr<ast::CompositeExpression> &arg) {
    if (!arg || arg->Components.empty()) return nullptr;
    if (arg->Operators[0] == ast::BaseOperator::Subscript) {
        const auto currentTypeVariant = arg->Components[0]->GetType();
        if (!currentTypeVariant) return nullptr;
        auto *storagePtr = &*currentTypeVariant;
        if (const auto *arrayType = std::get_if<ast::Type::ArrayType>(storagePtr)) {
            // 确保返回的是 shared_ptr 的拷贝
            return arrayType->BaseType;
        }
        if (auto *pointerType = std::get_if<ast::Type::PointerType>(storagePtr)) {
            // 确保返回的是 shared_ptr 的拷贝
            if (pointerType->PointerLevel == 1) {
                return pointerType->BaseType;
            }
            return std::make_shared<ast::Type::CompileType>(
                ast::Type::PointerType(pointerType->BaseType, pointerType->PointerLevel - 1));
        }
        return nullptr;
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
        if (op == ast::BaseOperator::Dereference) {
            if (const auto *ptrData = std::get_if<ast::Type::PointerType>(&(*subType))) {
                return ptrData->BaseType;
            }
            ErrorPrintln("Error: Cannot dereference a non-pointer type.\n");
            std::exit(-1);
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
                if (std::holds_alternative<ast::Type::EnumDefinition>(*type)) {
                    ErrorPrintln("MLC Semantic Error: Cannot use operator '{}' with enum types.\n",
                                 baseOperator(op));
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


type::sPtr<ast::Type::CompileType> ast::Expression::GetType() const {
    if (!Storage) return nullptr;

    // 1. 获取 Variant 指针
    auto *storagePtr = &*Storage;

    // 2. 依次尝试获取每种类型
    if (const auto *constVal = std::get_if<ConstValue>(storagePtr)) {
        return constVal->GetType();
    }

    if (const auto *varPtr = std::get_if<std::shared_ptr<Variable> >(storagePtr)) {
        return (*varPtr) ? (*varPtr)->VarType : nullptr;
    }

    if (const auto *funcCallPtr = std::get_if<std::shared_ptr<FunctionCall> >(storagePtr)) {
        return (*funcCallPtr && (*funcCallPtr)->FunctionDecl) ? (*funcCallPtr)->FunctionDecl->ReturnType : nullptr;
    }

    if (const auto *compExprPtr = std::get_if<std::shared_ptr<CompositeExpression> >(storagePtr)) {
        return handleCompositeType(*compExprPtr);
    }

    if (const auto *enumValPtr = std::get_if<EnumValue>(storagePtr)) {
        return std::make_shared<Type::CompileType>(*(enumValPtr->EnumDef));
    }

    if (auto *memberAccessPtr = std::get_if<std::shared_ptr<MemberAccess> >(storagePtr)) {
        auto &arg = *memberAccessPtr; // 为了方便书写
        if (arg && arg->StructDef) {
            const auto &members = arg->StructDef->Members;
            if (arg->Index < members.size()) {
                return members[arg->Index].Type;
            }
        }
    }

    return nullptr;
}