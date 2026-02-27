module Generator;

import std;
import Token;
import std;
import keyword;
import Parser;
import aux;

namespace gen = mlc::ir::gen;

using GenClass = gen::IRGenerator;
namespace ast = mlc::ast;
using size_t = std::size_t;
template<typename type>
using sPtr = std::shared_ptr<type>;
namespace type = ast::Type;

std::string GenClass::Struct(const std::shared_ptr<ast::Type::StructDefinition> &_structDef) {
    const auto title = std::format("%{}.{} = {} {{", kv::Struct, _structDef->Name, kv::Type);

    std::string body;
    for (const auto &[Name, Type]: _structDef->Members) {
        std::visit([&]<typename T0>(T0 &&arg) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, ast::Type::BaseType>) {
                body += std::format("{},", arg.Name);
            } else if constexpr (std::is_same_v<T, ast::Type::StructDefinition>) {
                body += std::format("%struct.{},", arg.Name);
            } else if constexpr (std::is_same_v<T, ast::Type::ArrayType>) {
                body += std::format("[{} x {}],", arg.Size(),
                                    std::visit([](auto &&t) -> std::string { return t.Name; }, *arg.BaseType));
            } else if constexpr (std::is_same_v<T, ast::Type::PointerType>) {
                body += "ptr,";
            }
        }, *Type);
    }
    body.pop_back(); // 去掉最后一个逗号
    body += "}";

    return title + body;
}

std::string GenClass::GlobalVariable(const sPtr<ast::VariableStatement> &_variable) {
    if (const auto initExpr = _variable->Initializer; initExpr != nullptr && ConstExpressionCheck(initExpr)) {
        auto value = ConstExpressionExpand(_variable->VarType, initExpr);
        return std::format("@{} = global {}", _variable->Name, value);
    }
    ErrorPrintln("Error: Global variable '{}' must be initialized with a constant expression.\n", _variable->Name);
    std::exit(-1);
}

std::string GenClass::LocalVariable(const std::string_view _functionName,
                                    const sPtr<ast::VariableStatement> &_variable) {
    auto registerName = std::format("%{}.{}", _functionName, _variable->Name);
}

std::string determineCastOperator(const type::BaseType *_sourceType, const type::BaseType *_targetType) {
    const auto sourceSize = _sourceType->Size();
    const auto targetSize = _targetType->Size();
    auto isTypeSigned = [](const type::BaseType *type) -> bool {
        return type->Name.starts_with('i') || type->Name.starts_with('f');
    };
    auto isTypeFloat = [](const type::BaseType *type) -> bool {
        return type->Name.starts_with('f');
    };
    if (_sourceType->Name == _targetType->Name) return "bitcast";

    const bool sourceIsFloat = isTypeFloat(_sourceType);
    const bool targetIsFloat = isTypeFloat(_targetType);
    if (sourceIsFloat) {
        if (targetIsFloat) {
            return (sourceSize < targetSize) ? "fpext" : "fptrunc";
        }
        return "fptosi";
    }
    if (targetIsFloat) {
        return isTypeSigned(_sourceType) ? "sitofp" : "uitofp";
    }
    if (sourceSize < targetSize) {
        return isTypeSigned(_sourceType) ? "sext" : "zext";
    }
    if (sourceSize > targetSize) {
        return "trunc";
    }
    return "bitcast";
}

GenClass::ExprResult GenClass::ExpressionExpand(const sPtr<ast::Expression> &_expression) {

}

std::string GenClass::ConstExpressionExpand(const sPtr<ast::Type::CompileType> &_type,
                                            const sPtr<ast::Expression> &_expression) {
    if (!_expression || !_expression->Storage) return "";
    const auto &data = *_expression->Storage;
    if (const auto *constVal = std::get_if<ast::ConstValue>(&data)) {
        std::string typeStr = GetTypeName(*_expression->GetType());
        return std::format("{} {}", typeStr, constVal->Value);
    }
    if (const auto *initListPtr = std::get_if<sPtr<ast::InitializerList>>(&data)) {
        const auto& initList = *initListPtr;
        auto elementStrings = initList->Values
                              | std::views::transform([](const sPtr<ast::Expression> &expr) {
                                  return ConstExpressionExpand(expr->GetType(), expr);
                              });
        std::string joinedElements = elementStrings
                                     | std::views::join_with(std::string(", "))
                                     | std::ranges::to<std::string>();
        if (std::get_if<type::StructDefinition>(&*_type)) {
            return std::format("{{ {} }}", joinedElements);
        }
        if (const auto* arrayType = std::get_if<type::ArrayType>(&*_type)) {
            return std::format("[{} x {}] {{ {} }}", arrayType->Length, GetTypeName(*arrayType->BaseType), joinedElements);
        }
    }

    if (const auto *compExprPtr = std::get_if<sPtr<ast::CompositeExpression>>(&data)) {
        const auto& compExpr = *compExprPtr;
        if (compExpr->Components.empty()) return "";

        std::string result = ConstExpressionExpand(compExpr->Components[0]->GetType(), compExpr->Components[0]);
        for (size_t i = 0; i < compExpr->Operators.size(); ++i) {
            std::string opSymbol = ast::BaseIROperators.at(compExpr->Operators[i]);
            std::string nextComponent = ConstExpressionExpand(compExpr->Components[i + 1]->GetType(), compExpr->Components[i + 1]);
            result = std::format("{} ({} {}, {})", opSymbol, GetTypeName(*_expression->GetType()), result, nextComponent);
        }
        return result;
    }

    if (const auto *funcCallPtr = std::get_if<sPtr<ast::FunctionCall>>(&data)) {
        const auto& funcCall = *funcCallPtr;
        auto functionName = funcCall->FunctionDecl ? funcCall->FunctionDecl->Name : "unknown_func";
        auto sourceType = std::get_if<type::BaseType>(&*funcCall->Arguments[0]->GetType());
        auto convertFuncType = &*std::ranges::find_if(type::BaseTypes, [&](const type::BaseType &baseType) { return baseType.Name == functionName; });
        std::string castOp = determineCastOperator(sourceType, convertFuncType);
        return std::format("{} ({} {})", castOp, GetTypeName(*convertFuncType), ConstExpressionExpand(funcCall->Arguments[0]->GetType(), funcCall->Arguments[0]));
    }

    ErrorPrintln("Error: Expression is not a constant expression and cannot be evaluated at compile time.\n");
    std::exit(-1);
}