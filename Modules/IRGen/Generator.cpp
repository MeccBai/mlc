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

std::string GenClass::LocalVariable(const sPtr<ast::VariableStatement> &_variable) {

    auto registerName = std::format("%{}",_variable->Name);
    auto alignSize = std::visit([](auto &&t) -> size_t {
        return t.Size();
    }, *_variable->VarType);

    auto alloca = std::format("{} = alloca {}, align {}\n", registerName, TypeToLLVM(_variable->VarType),alignSize);
    const auto [llvmType, resultVar, code] = ExpressionExpand(_variable->Initializer);
    auto store = std::format("store {} {}, {}* {}, align {}\n", llvmType, resultVar, TypeToLLVM(_variable->VarType), registerName,alignSize);
    return std::format("{}{}{}", alloca, code, store);
}

std::string gen::determineCastOperator(const type::BaseType *_sourceType, const type::BaseType *_targetType) {
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
