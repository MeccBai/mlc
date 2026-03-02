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
    DisableOutputBuffering();

    std::vector<Type::StructMember> oneMembers = {
        {"x", std::make_shared<Type::CompileType>(Type::BaseTypes[0])},
        {"y", std::make_shared<Type::CompileType>(Type::BaseTypes[0])},
    };

    auto one = Type::StructDefinition(
        "One", oneMembers
    );

    std::vector<Type::StructMember> twoMembers = {
        {"x", std::make_shared<Type::CompileType>(one)},
        {"y", std::make_shared<Type::CompileType>(Type::BaseTypes[0])},
    };

    auto two = Type::StructDefinition(
        "Two", twoMembers
    );

    using generator = mlc::ir::gen::IRGenerator;
    const auto onePtr = std::make_shared<Type::StructDefinition>(one);
    const auto twoPtr = std::make_shared<Type::StructDefinition>(two);
    const auto compileType = std::make_shared<Type::CompileType>(*twoPtr);
    std::println("{}", generator::Struct(onePtr));
    std::println("{}", generator::Struct(twoPtr));

    auto memberAccess = std::make_shared<ast::MemberAccess>(ast::MemberAccess(twoPtr, 0));

    auto variable = std::make_shared<ast::VariableStatement>("a", compileType, nullptr);
    auto exprVariable = std::make_shared<ast::Expression>(variable);
    auto accessExpr = std::make_shared<ast::Expression>(memberAccess);

    auto exprVec = std::vector{exprVariable, accessExpr, accessExpr};
    //CompositeExpression(std::vector<std::shared_ptr<Expression> > _components,
    //                                     std::vector<BaseOperator> _operators,
    //                                     const bool _isOperatorFirst = false)

    auto i64Type = std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i64"));
    auto arg1 = ast::VariableStatement("one", i64Type);
    auto func1 = ast::FunctionDeclaration("func", i64Type, std::vector{
                                                  ast::MakeVariable(arg1)
                                              });

    auto oneType = std::make_shared<Type::CompileType>(*onePtr);
    auto twoType = std::make_shared<Type::CompileType>(*twoPtr);
    auto arg2 = ast::VariableStatement("two", twoType);

    auto func2 = ast::FunctionDeclaration("func2", oneType, std::vector{
                                              ast::MakeVariable(arg1),
                                              ast::MakeVariable(arg2)
                                          });

    auto arg3 = ast::VariableStatement("one", oneType);

    auto func3 = ast::FunctionDeclaration("func3", twoType, std::vector{
                                              ast::MakeVariable(arg3)
                                          });

    auto composite = ast::CompositeExpression(exprVec, std::vector{ast::BaseOperator::Dot, ast::BaseOperator::Dot},
                                              false);

    auto expr = std::make_shared<ast::Expression>(std::make_shared<ast::CompositeExpression>(composite));

    auto const10 = std::make_shared<ast::Expression>(ast::ConstValue("10"));
    auto i32Type = std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i32"));
    auto variable10 = ast::MakeVariable(ast::Variable("on10",i32Type, const10));

    auto variable10Expr = std::make_shared<ast::Expression>(variable10);
    auto variable12 = ast::MakeVariable(ast::Variable("on12",i32Type, variable10Expr));

    std::println("{}", generator::ExpressionExpand(expr).code);

    std::println("{}",generator::LocalVariable(variable10));
    std::println("{}",generator::LocalVariable(variable12));

    std::println("{}", generator::FunctionUnit(std::make_shared<ast::FunctionDeclaration>(func1)).functionDecl);
    std::println("{}", generator::FunctionUnit(std::make_shared<ast::FunctionDeclaration>(func2)).functionDecl);
    std::println("{}", generator::FunctionUnit(std::make_shared<ast::FunctionDeclaration>(func3)).functionDecl);

    mlc::ir::gen::IRGenerator::FuncArg a {
        true,false,8,"%struct.c","%1"
    };

    auto result = mlc::ir::gen::IRGenerator::FunctionArg(a,0);

    std::println("{}", result.code);
    std::println("{}", result.resultVar);

    return 0;
}
