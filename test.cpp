//
// Created by Administrator on 2026/2/21.
//

import Parser;
import std;
import Prepare;
import Compiler;
import aux;
import Token;
import Generator;

namespace ast = mlc::ast;
namespace Type = ast::Type;

int main() {
    DisableOutputBuffering() ;

    std::vector<Type::StructMember> members = {
        {"x", std::make_shared<Type::CompileType>(Type::BaseTypes[0])},
        {"y", std::make_shared<Type::CompileType>(Type::BaseTypes[0])},
    };

    auto one = Type::StructDefinition(
        "One", members
    );

    mlc::ir::gen::IRGenerator generator;
    auto ptr = std::make_shared<Type::StructDefinition>(one);
    std::print("{}",generator.Struct(ptr));

     return 0;
}
