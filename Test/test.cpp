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

    using generator = mlc::ir::gen::IRGenerator;
    const auto ptr = std::make_shared<Type::StructDefinition>(one);
    std::println("{}",generator::Struct(ptr));

    auto memberAccess = std::make_shared<ast::MemberAccess>(ast::MemberAccess(ptr, 0));

    auto expr = std::make_shared<ast::Expression>(memberAccess);

    std::println("{}",generator::ExpressionExpand(expr).code);

     return 0;
}
