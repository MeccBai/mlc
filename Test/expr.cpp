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
    auto c1 = ast::MakeExpression(ast::ConstValue("0"));
    auto c2 = ast::MakeExpression(ast::ConstValue("1"));
    auto c3 = ast::MakeExpression(ast::ConstValue("2"));

    auto comp = ast::MakeExpression(ast::MakeCompExpr(
        std::vector{c1, c2, c3},
        std::vector{ast::BaseOperator::Add, ast::BaseOperator::Mul}
    ));

    auto arrayType = Type::MakeCompileType(Type::ArrayType(Type::MakeCompileType(*Type::BaseTypeMap.at("i32")), 10));
    auto initList = ast::MakeExpression(ast::MakeInitializerList(std::vector{
        ast::MakeExpression(ast::ConstValue("0")),
        ast::MakeExpression(ast::ConstValue("1")),
        ast::MakeExpression(ast::ConstValue("2"))
    }));


    auto a1 = ast::MakeVariable(ast::Variable("a", arrayType, initList));

    const auto i32Type = std::make_shared<Type::CompileType>(*Type::BaseTypeMap.at("i32"));

    std::println("{}", gen::ConstExpressionExpand(i32Type, comp));

    std::println("{}", gen::ExpressionExpand(comp).code);

    auto iResult = gen::ExpressionExpand(initList);
    std::println("{}\n{}", iResult.resultVar,iResult.code);

}
