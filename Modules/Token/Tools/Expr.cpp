//
// Created by Administrator on 2026/3/6.
//
module Token;

import :Type;

import std;
using size_t = std::size_t;
namespace ast = mlc::ast;
namespace type = ast::Type;

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
        if (const auto *arrayType = type::GetType<ast::Type::ArrayType>(currentTypeVariant)) {
            // 确保返回的是 shared_ptr 的拷贝
            return arrayType->BaseType;
        }
        if (const auto *pointerType = type::GetType<ast::Type::PointerType>(currentTypeVariant)) {
            return pointerType->Dereference();
        }
        return nullptr;
    }

    if (arg->Operators.size() == 1 && arg->Components.size() == 1) {
        const auto op = arg->Operators[0];
        const auto subType = arg->Components[0]->GetType();
        if (!subType) return nullptr;
        if (op == ast::BaseOperator::AddressOf) {
            if (const auto *ptrData = std::get_if<ast::Type::PointerType>(&(*subType))) {
                return ast::Make<ast::Type::CompileType>(
                    ast::Type::PointerType(subType, ptrData->PointerLevel + 1));
            }
            const auto returnPtr = ast::Make<ast::Type::CompileType>(
                ast::Type::PointerType(subType, 1));
            auto &tempPointer = std::get<ast::Type::PointerType>(*returnPtr);
            tempPointer.Finalize(subType);
            return returnPtr;
        }
        if (op == ast::BaseOperator::Dereference) {
            if (const auto *ptrData = std::get_if<ast::Type::PointerType>(&(*subType))) {
                return ptrData->Dereference();
            }
            ErrorPrintln("Error: Cannot dereference a non-pointer type.\n");
            std::exit(-1);
        }
    }

    // 单目运算逻辑
    if (arg->Operators.size() == 1 && arg->Components.size() == 2) {
        const auto op = arg->Operators[0];
        const auto subType = arg->Components[0]->GetType();
        if (!subType) return nullptr;
        if (op == ast::BaseOperator::Add || op == ast::BaseOperator::Sub ||
            op == ast::BaseOperator::Mul || op == ast::BaseOperator::Div) {
            // 2. 地毯式检查所有组件
            for (const auto &exp: arg->Components) {
                const auto type = exp->GetType();
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
                    if (const auto *const baseStruct = std::get_if<ast::Type::StructDefinition>(&(*baseType))) {
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
                if (const auto *ptrData = std::get_if<ast::Type::PointerType>(&(*argSubType))) {
                    return ptrData->Dereference();
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
    return std::visit(
        overloaded{
            [&](const ConstValue &constVal) ->Type::sPtr<Type::CompileType> {
                return constVal.GetType();
            },
            [&](const EnumValue &_enum) ->Type::sPtr<Type::CompileType> {
                return _enum.GetType();
            },
            [&](const type::sPtr<Variable> &_var)->Type::sPtr<Type::CompileType> {
                return _var->VarType;
            },
            [&](const type::sPtr<FunctionCall> &_funcCall) {
                return _funcCall->FunctionDecl ? _funcCall->FunctionDecl->ReturnType : nullptr;
            },
            [&](const type::sPtr<CompositeExpression> &_comp) {
                return handleCompositeType(_comp);
            },
            [&](const type::sPtr<MemberAccess> &_memberAccess) {
                if (const auto &arg = _memberAccess;
                    arg && arg->StructDef) {
                    if (const auto &members = arg->StructDef->Members;
                        arg->Index < members.size()) {
                        return members[arg->Index].Type;
                    }
                }
                ErrorPrintln("Error: Invalid member access.\n");
                std::exit(-1);
            },
            [](const auto &)->Type::sPtr<Type::CompileType>  {
                ErrorPrintln(" Compile Internal Error.\n");
                std::exit(-1);
            }
        }, *Storage
    );
}


ast::Expression::~Expression() = default;
