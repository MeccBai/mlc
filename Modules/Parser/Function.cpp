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


ast::FunctionScope astClass::functionDefParser(const std::string_view _functionContent) {
    ContextTable<ast::VariableStatement> context;
    auto pos = _functionContent.find(' ');
    auto bracketPos = _functionContent.find('(');
    auto bracketEndPos = _functionContent.find("){");
    auto functionBody = _functionContent.substr(bracketEndPos + 2, _functionContent.length() - bracketEndPos - 3);
    auto functionHeader = _functionContent.substr(0, bracketEndPos + 1);
    auto statements = seg::TokenizeFunctionBody(functionBody);
    auto functionDecl = functionDeclParser(functionHeader);
    auto functionDeclPtr = ast::FunctionDeclaration(functionDecl);
    for (auto args = functionDeclPtr.Parameters;
         const auto &arg: args) {
        context.emplace_back(std::make_shared<ast::VariableStatement>(arg));
    }
    const auto statementsParsed = statements | std::views::transform(
                                      [&](const std::string_view statement) {
                                          return statementParser(context, statement);
                                      }) | std::ranges::to<std::vector<ast::Statement> >();
    return {functionDecl, statementsParsed};
}

ast::FunctionDeclaration mlc::parser::AbstractSyntaxTree::functionDeclParser(
    const std::string_view _functionContent) const {
    //void func(int a,int b)
    const std::string_view returnType = _functionContent.substr(0, _functionContent.find(' '));

    const auto leftBracket = _functionContent.find('(');
    const auto rightBracket = _functionContent.find(')');
    const auto argsList = _functionContent.substr(leftBracket + 1, rightBracket - leftBracket - 1);
    const auto functionName = _functionContent.substr(returnType.length() + 1, leftBracket - returnType.length() - 1);

    const std::weak_ptr<ast::Type::CompileType> returnTypePtr = findType(returnType);

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

    for (const auto argsSplit = split(argsList, ","); const auto &arg: argsSplit) {
        const auto argument = split(arg, " ");
        if (argument.size() != 2) {
            ErrorPrintln("Error: Invalid argument format '{}'\n", arg);
            std::exit(-1);
        }
        const auto argType = argument[0];
        if (argType.find('[') != std::string_view::npos || argType.find(']') != std::string_view::npos) {
            ErrorPrintln("Error: Array type is not allowed in function parameters '{}'\n", argType);
            std::exit(-1);
        }
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

astClass::functionWarper astClass::functionDeclSpliter(const std::string_view _functionContent) const {
    const auto bracketEndPos = _functionContent.find("){");
    const auto functionBody = _functionContent.substr(bracketEndPos + 2, _functionContent.length() - bracketEndPos - 3);
    const auto functionHeader = _functionContent.substr(0, bracketEndPos + 1);
    const auto functionDecl = functionDeclParser(functionHeader);
    return {functionDecl, functionBody};
}
