//
// Created by Administrator on 2026/3/3.
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
using gen = mlc::ir::gen::IRGenerator;

int main() {
    DisableOutputBuffering();
    auto members = std::vector<Type::StructMember>{
        {"x", std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64"))},
        {"y", std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64"))},
        {"z", std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64"))},
        {"m", std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64"))},
    };

    auto structOne = Type::StructDefinition(
        "One", members
    );

    auto memebers2 = std::vector<Type::StructMember>{
        {"a", std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64"))},
        {"b", std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64"))},
        {"c", std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64"))},
    };

    auto structTwo = Type::StructDefinition(
        "Two", memebers2
    );

    auto decl = std::make_shared<ast::FunctionDeclaration>(
        "func",
        std::make_shared<Type::CompileType>(structOne),
        std::vector{
            ast::MakeVariable(
                ast::VariableStatement("one", std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64")),
                                       nullptr)),
            ast::MakeVariable(
                ast::VariableStatement("two", std::make_shared<Type::CompileType>(structTwo),
                                       nullptr))

        }
    );
    auto func = ast::FunctionScope(
        *decl, {}, false
    );



    auto funcPtr = std::make_shared<ast::FunctionScope>(func);
    auto funcStr = gen::FunctionGenerate(funcPtr);

    std::println("{}", funcStr);

    std::println("{}",gen::FunctionDeclarationGenerate(funcPtr->ToDeclaration()));

    auto decl2 = std::make_shared<ast::FunctionDeclaration>(
        "func2",
        std::make_shared<Type::CompileType>(structOne),
        std::vector<Type::sPtr<ast::VariableStatement> >{}
    );
    auto argsExpr = std::vector<std::shared_ptr<ast::Expression> >{};
    auto functionCall = std::make_shared<ast::FunctionCall>(ast::FunctionCall(
        decl2, argsExpr
    ));

    auto result = gen::FunctionCall(
        functionCall
    );

    std::println("{}", result.callCode);
    auto callCode = result.callCode;
    auto varName = std::string("var");
    std::println("{}",result.GetCallCode(varName));
}
