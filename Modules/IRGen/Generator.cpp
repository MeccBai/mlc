module Generator;

import std;
import Token;
import std;
import keyword;

namespace gen = mlc::ir::gen;

std::string gen::IRGenerator::Struct(const std::shared_ptr<ast::Type::StructDefinition> &_structDef) {
    const auto title = std::format("%{}.{} = {} {{",kv::Struct,_structDef->Name,kv::Type);

    std::string body;
    for (const auto &[Name, Type]: _structDef->Members) {
       std::visit ([&]<typename T0>(T0 &&arg) {
           using T = std::decay_t<T0>;
           if constexpr (std::is_same_v<T, ast::Type::BaseType>) {
               body += std::format("{},", arg.Name);
           } else if constexpr (std::is_same_v<T, ast::Type::StructDefinition>) {
               body += std::format("%struct.{},", arg.Name);
           } else if constexpr (std::is_same_v<T, ast::Type::ArrayType>) {
               body += std::format("[{} x {}],", arg.Size(), std::visit([](auto &&t) -> std::string { return t.Name; }, *arg.BaseType));
           } else if constexpr (std::is_same_v<T, ast::Type::PointerType>) {
               body += "ptr,";
           }
       }, *Type);
    }
    body.pop_back(); // 去掉最后一个逗号
    body += "}";

    return title + body;
}

std::string gen::IRGenerator::GlobalVariable(const sPtr<ast::VariableStatement> &_variable) {

}
