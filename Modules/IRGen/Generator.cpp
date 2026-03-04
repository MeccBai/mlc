module Generator;

import std;
import Token;
import std;
import keyword;
import Parser;
import aux;

namespace kv = mlc::ir::kv;


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

std::string GenClass::GenerateIR(parser::AbstractSyntaxTree &_ast) {
    std::string code;
    auto structCode = _ast.typeSymbolTable
                      | std::views::filter([](const auto &type) {
                          return std::holds_alternative<ast::Type::StructDefinition>(*type);
                      })
                      | std::views::transform([](const auto &type) {
                          auto structDef = std::get<ast::Type::StructDefinition>(*type);
                          return Struct(std::make_shared<ast::Type::StructDefinition>(structDef));
                      });
    auto globalVarCode = _ast.variableSymbolTable
                         | std::views::transform([](const auto &var) {
                             return GlobalVariable(var);
                         });
    auto funcDecl = _ast.functionSymbolTable
                    | std::views::filter([&](const auto &func) {
                        // 1. 物理隔离：剔除基本类型占位符 (i32, f32, etc.)
                        if (ast::Type::BaseTypeMap.contains(func->Name)) {
                            return false;
                        }
                        const bool hasDefinition = std::ranges::any_of(
                            _ast.functionScopeTable,
                            [&](const auto &funcScope) {
                                return funcScope->Name == func->Name;
                            });
                        return !hasDefinition;
                    })
                    | std::views::transform([&](const auto &func) {
                        return FunctionDeclarationGenerate(func);
                    });


    auto funcCode = _ast.functionScopeTable
                    | std::views::transform([](const auto &func) {
                        return FunctionGenerate(func);
                    });
    auto join_to_string = [](auto &&view) {
        std::string res;
        for (const auto &s: view) res += s;
        return res;
    };
    code += join_to_string(structCode);
    code += join_to_string(globalVarCode);
    code += join_to_string(funcDecl);
    code += join_to_string(funcCode);
    return code;
}
