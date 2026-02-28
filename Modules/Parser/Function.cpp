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
    const auto bracketEndPos = _functionContent.find("){");
    const auto functionBody = _functionContent.substr(bracketEndPos + 2, _functionContent.length() - bracketEndPos - 3);
    const auto functionHeader = _functionContent.substr(0, bracketEndPos + 1);
    auto statements = seg::TokenizeFunctionBody(functionBody);
    auto functionDecl = functionDeclParser(functionHeader);
    const auto functionDeclPtr = ast::FunctionDeclaration(functionDecl);
    for (const auto args = functionDeclPtr.Parameters;
         const auto &arg: args) {
        context.emplace_back(arg);
    }
    const auto statementsTemp = statements | std::views::transform(
                                    [&](const std::string_view statement) {
                                        return statementParser(context, statement);
                                    }) | std::ranges::to<std::vector<std::vector<std::shared_ptr<
                                    ast::Statement> > > >();

    auto statementsParsed = statementsTemp | std::views::join | std::ranges::to<std::vector<std::shared_ptr<
                                ast::Statement> > >();

    return {functionDecl, statementsParsed};
}

sPtr<ast::VariableStatement> astClass::functionArgParser(const std::string_view _argContent) const {
    if (_argContent.empty()) {
        ErrorPrintln("Error: Empty argument declaration\n");
        std::exit(-1);
    }
    if (_argContent.find(' ') != std::string_view::npos) {
        const auto argPack = split(_argContent," ");
        if (argPack.size() != 2) {
            ErrorPrintln("Error: Invalid argument declaration '{}'\n", _argContent);
            std::exit(-1);
        }
        const auto type = argPack[0];
        const auto name = argPack[1];
        const auto typePtr = findType(type);
        if (name.find('[') != std::string_view::npos || name.find(']')!= std::string_view::npos) {
            ErrorPrintln("Error: Function Argument not allow array type '{}'\n", name);
            std::exit(-1);
        }
        if (typePtr == std::nullopt) {
            ErrorPrintln("Error: Unknown type '{}'\n", type);
            std::exit(-1);
        }
        const auto& currentType = typePtr.value();
        if (name.empty()) {
            ErrorPrintln("Error: Invalid argument declaration '{}'\n", _argContent);
            std::exit(-1);
        }
        return std::make_shared<ast::VariableStatement>(ast::VariableStatement(std::string(name), currentType, nullptr));
    }
    if (_argContent.find('$') != std::string_view::npos) {
        const auto first = _argContent.find_first_of('$');
        const auto last = _argContent.find_last_of('$');
        auto level = last - first + 1;
        auto typeName = _argContent.substr(0, first);
        const auto name = _argContent.substr(last + 1);
        if (const auto typePtr = findType(typeName); typePtr == std::nullopt) {
            ErrorPrintln("Error: Unknown type '{}'\n", typeName);
            std::exit(-1);
        }
        auto typePtr = findType(typeName).value();
        const auto pType = std::make_shared<type::PointerType>(level);
        pType->Finalize(typePtr);
        typePtr = std::make_shared<type::CompileType>(*pType);
        if (name.empty()) {
            ErrorPrintln("Error: Invalid argument declaration '{}'\n", _argContent);
            std::exit(-1);
        }
        return std::make_shared<ast::VariableStatement>(ast::VariableStatement(std::string(name), typePtr, nullptr));
    }
    ErrorPrintln("Error: Invalid argument declaration '{}'\n", _argContent);
    std::exit(-1);
}

ast::FunctionDeclaration astClass::functionDeclParser(
    const std::string_view _functionContent)  {
    //void func(int a,int b)
    const std::string_view returnType = _functionContent.substr(0, _functionContent.find(' '));

    const auto leftBracket = _functionContent.find('(');
    const auto rightBracket = _functionContent.find(')');
    const auto argsList = _functionContent.substr(leftBracket + 1, rightBracket - leftBracket - 1);
    const auto functionName = _functionContent.substr(returnType.length() + 1, leftBracket - returnType.length() - 1);

    const auto returnTypePtr = findType(returnType);
    if (!returnTypePtr) {
        ErrorPrintln("Error: Unknown return type '{}'\n", returnType);
        std::exit(-1);
    }
    if (argsList == "...") {
        return ast::FunctionDeclaration(std::string(functionName), returnTypePtr.value(), {}, true);
    }


    if (argsList.empty()) {
        return ast::FunctionDeclaration(std::string(functionName), returnTypePtr.value(), {});
    }

    std::vector<sPtr<ast::VariableStatement> > args;

    for (const auto argsSplit = argSplit(argsList); const auto &arg: argsSplit) {

        args.emplace_back(functionArgParser(arg));
    }

    return ast::FunctionDeclaration(std::string(functionName), returnTypePtr.value(), args);
}

astClass::functionWarper astClass::functionDeclSpliter(const std::string_view _functionContent) {
    const auto bracketEndPos = _functionContent.find("){");
    const auto functionBody = _functionContent.substr(bracketEndPos + 2, _functionContent.length() - bracketEndPos - 3);
    const auto functionHeader = _functionContent.substr(0, bracketEndPos + 1);
    const auto functionDecl = functionDeclParser(functionHeader);
    return {functionDecl, functionBody};
}
