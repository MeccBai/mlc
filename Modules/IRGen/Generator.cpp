module Generator;

import std;
import Token;
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
                body += std::format("[{} x {}],", arg.Size(), type::GetTypeName(*(arg.BaseType.get())));
            } else if constexpr (std::is_same_v<T, ast::Type::PointerType>) {
                body += "ptr,";
            }
        }, *Type);
    }
    body.pop_back(); // 去掉最后一个逗号
    body += "}\n";

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


void fixLLVMTypes(std::string &ir) {
    size_t pos = 0;
    while ((pos = ir.find(" f64 ", pos)) != std::string::npos) {
        ir.replace(pos, 5, " double "); // " f64 " 是 5 位，" double " 是 8 位
        pos += 8; // 跳过新替换的长度
    }
    pos = 0;
    while ((pos = ir.find("f64\n", pos)) != std::string::npos) {
        ir.replace(pos, 4, "double\n");
        pos += 7;
    }
    pos = 0;
    while ((pos = ir.find("f64,", pos)) != std::string::npos) {
        ir.replace(pos, 4, "double,");
        pos += 7;
    }
    pos = 0;
    while ((pos = ir.find("f64]", pos)) != std::string::npos) {
        ir.replace(pos, 4, "double]");
        pos += 7;
    }
    pos = 0;
    while ((pos = ir.find(" f32 ", pos)) != std::string::npos) {
        ir.replace(pos, 5, " float "); // " f64 " 是 5 位，" double " 是 8 位
        pos += 8; // 跳过新替换的长度
    }
    pos = 0;
    while ((pos = ir.find("f32\n", pos)) != std::string::npos) {
        ir.replace(pos, 4, "float\n");
        pos += 6;
    }
    pos = 0;
    while ((pos = ir.find("f32,", pos)) != std::string::npos) {
        ir.replace(pos, 4, "float,");
        pos += 6;
    }
    pos = 0;
    while ((pos = ir.find("f32]", pos)) != std::string::npos) {
        ir.replace(pos, 4, "float]");
        pos += 6;
    }
}

std::string GenClass::GenerateIR(parser::AbstractSyntaxTree &_ast) {
    std::string code = "target triple = \"x86_64-w64-windows-gnu\"\n";
    const auto &[types, variables, decls,funcs] = _ast.ExportAST();

    auto structCode = types
                      | std::views::filter([](const auto &type) {
                          return std::holds_alternative<ast::Type::StructDefinition>(*type);
                      })
                      | std::views::transform([](const auto &type) {
                          auto structDef = std::get<ast::Type::StructDefinition>(*type);
                          return Struct(std::make_shared<ast::Type::StructDefinition>(structDef));
                      });
    auto globalVarCode = variables
                         | std::views::transform([](const auto &var) {
                             return GlobalVariable(var);
                         });
    auto funcDecl = decls
                    | std::views::filter([&](const auto &func) {
                        // 1. 物理隔离：剔除基本类型占位符 (i32, f32, etc.)
                        if (ast::Type::BaseTypeMap.contains(func->Name)) {
                            return false;
                        }
                        const bool hasDefinition = std::ranges::any_of(
                            funcs,
                            [&](const auto &funcScope) {
                                return funcScope->Name == func->Name;
                            });
                        return !hasDefinition;
                    })
                    | std::views::transform([&](const auto &func) {
                        return FunctionDeclarationGenerate(func);
                    });


    auto funcCode = funcs
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

    fixLLVMTypes(code);

    return code;
}
