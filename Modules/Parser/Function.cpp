//
// Created by Administrator on 2026/2/22.
//

module Parser;
import aux;

using astClass = mlc::parser::AbstractSyntaxTree;

template<typename type>
using sPtr = std::shared_ptr<type>;
namespace ast = mlc::ast;
using size_t = std::size_t;


ast::FunctionScope astClass::functionDefParser(std::string_view functionContent) {
    auto statements = mlc::seg::TokenizeFunctionBody(functionContent);
    // 这里可以进一步解析函数头，提取参数和返回类型等信息
    return ast::FunctionScope("", {}, {}, {});
}

ast::FunctionDeclaration mlc::parser::AbstractSyntaxTree::functionDeclParser(const std::string_view _functionContent) {
    //void func(int a,int b)
    const std::string_view returnType = _functionContent.substr(0, _functionContent.find(' '));


    const auto leftBracket = _functionContent.find('(');
    const auto rightBracket = _functionContent.find(')');
    const std::string_view argsList = _functionContent.substr(leftBracket + 1, rightBracket - leftBracket - 1);
    const auto functionName = _functionContent.substr(returnType.length() + 1, leftBracket - returnType.length() - 1);

    std::weak_ptr<ast::Type::CompileType> returnTypePtr = findType(returnType);

    if (argsList == "...") {
        return ast::FunctionDeclaration(std::string(functionName), returnTypePtr, {}, true);
    }

    if (returnTypePtr.expired()) {
        ErrorPrintln("Error: Unknown return type '{}'\n", returnType);
        std::exit(-1);
    }
    if (argsList.empty()) {
        return ast::FunctionDeclaration(std::string(functionName), returnTypePtr, {});
    }

    std::vector<ast::VariableStatement> args;

    for (const auto argsSplit = Spilit(argsList, ","); const auto &arg: argsSplit) {
        const auto argument = Spilit(arg, " ");
        if (argument.size() != 2) {
            ErrorPrintln("Error: Invalid argument format '{}'\n", arg);
            std::exit(-1);
        }
        const auto argType = argument[0];
        const auto argName = argument[1];
        std::weak_ptr<ast::Type::CompileType> argTypePtr = findType(argType);
        if (argTypePtr.expired()) {
            ErrorPrintln("Error: Unknown type '{}'\n", argType);
            std::exit(-1);
        }
        args.emplace_back(argName, argTypePtr.lock(), nullptr);
    }

    return ast::FunctionDeclaration(std::string(functionName), returnTypePtr, args);
}
