//
// Created by Administrator on 2026/2/20.
//

module Parser;
import std;
import aux;

using astClass = mlc::parser::AbstractSyntaxTree;

template<typename type>
using sPtr = std::shared_ptr<type>;
namespace ast = mlc::ast;
using size_t = std::size_t;
//[if(p==0){a.x=10;}else{a.y=10;}]


template<typename type>
using sPtr = std::shared_ptr<type>;


std::vector<std::string_view> Spilit(std::string_view str, std::string_view delimiter) {
    std::vector<std::string_view> result;
    size_t start = 0;
    while (true) {
        size_t pos = str.find(delimiter, start);
        if (pos == std::string_view::npos) {
            result.push_back(str.substr(start));
            break;
        }
        result.push_back(str.substr(start, pos - start));
        start = pos + delimiter.length();
    }
    return result;
}

ast::SubScope astClass::subScopeParser(const std::string_view subScopeContent) {
    auto statements = mlc::seg::TokenizeFunctionBody(subScopeContent);
    // 这里可以进一步解析条件表达式等信息
    return ast::SubScope({}, ast::SubScopeType::AnonymousBlock, {});
}

namespace type = ast::Type;

std::vector<type::StructDefinition> astClass::structDefParser(
    const std::vector<std::string_view> &_structContents) {
    const type::StructMember member{
        typeSymbolTable[0], "One"
    };
    std::vector members{member};
    return {type::StructDefinition("MyStruct", members)};
}

ast::Type::EnumDefinition astClass::enumDefParser(std::string_view _enumContent) {
    auto enums = std::vector<std::string>{"Value1", "Value2", "Value3"};
    return ast::Type::EnumDefinition("enum", enums);
}


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

astClass::AbstractSyntaxTree(const std::vector<seg::TokenStatement> &tokens) {
    std::vector<std::string_view> groups[6];

    std::ranges::for_each(tokens, [&](const auto &t) {
        groups[static_cast<size_t>(t.type)].emplace_back(t.content);
    });

    auto &functions = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDefinition)];
    auto &structs = groups[static_cast<size_t>(ast::GlobalStateType::StructDefinition)];
    auto &enums = groups[static_cast<size_t>(ast::GlobalStateType::EnumDefinition)];
    auto &varDecls = groups[static_cast<size_t>(ast::GlobalStateType::VariableDeclaration)];
    auto &funcDecls = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDeclaration)];

    functionSymbolTable.reserve(ast::Type::BaseTypes.size() + functions.size() + funcDecls.size());
    for (auto type: ast::Type::BaseTypes) {
        auto typePtr = std::make_shared<ast::Type::CompileType>(type);

        typeSymbolTable.emplace_back(typePtr);
        functionSymbolTable.emplace_back(
            std::make_shared<ast::FunctionDeclaration>(
                ast::FunctionDeclaration(
                    type.Name,
                    typePtr,
                    {},
                    true
                )
            )
        );
    }


    for (auto &func: functions) {
        auto decl = functionDefParser(func);
        functionSymbolTable.emplace_back(std::make_shared<ast::FunctionDeclaration>(decl.ToDeclaration()));
    }

    for (auto &decl: funcDecls) {
        auto declParsed = functionDeclParser(decl);
        functionSymbolTable.emplace_back(std::make_shared<ast::FunctionDeclaration>(declParsed));
    }

    for (auto structDefs = structDefParser(structs); auto &structDef: structDefs) {
        auto structPtr = std::make_shared<ast::Type::CompileType>(structDef);
        typeSymbolTable.emplace_back(structPtr);
    }

    for (auto &varDecl: varDecls) {
        for (const auto varParsed = variableParser(varDecl); auto &v: varParsed) {
            variableSymbolTable.emplace_back(std::make_shared<ast::VariableStatement>(v));
        }
    }

    for (auto &enumDef: enums) {
        auto enumParsed = enumDefParser(enumDef);
        auto enumPtr = std::make_shared<ast::Type::CompileType>(enumParsed);
        typeSymbolTable.emplace_back(enumPtr);
    }
}

auto astClass::findType(const std::string_view _typeName) const -> std::weak_ptr<ast::Type::CompileType> {
    for (const auto &typePtr: typeSymbolTable) {
        if (std::visit([](auto &&arg) -> std::string_view {
            return arg.Name; // 只要所有子类都有 Name，这个就能编译通过
        }, *typePtr) == _typeName) {
            return typePtr;
        }
    }
    return {};
}
