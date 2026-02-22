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

void astClass::typeDefParser(std::string_view _typedefContent) {
}



ast::FunctionScope astClass::functionDefParser(std::string_view functionContent) {
    auto statements = mlc::seg::TokenizeFunctionBody(functionContent);
    // 这里可以进一步解析函数头，提取参数和返回类型等信息
    return ast::FunctionScope("", {}, {}, {});
}

ast::FunctionDeclaration mlc::parser::AbstractSyntaxTree::functionDeclParser(std::string_view functionContent) {
    return ast::FunctionDeclaration("", {}, {}, false);
}

astClass::AbstractSyntaxTree(const std::vector<seg::TokenStatement> &tokens) {
    std::vector<std::string_view> groups[6];

    std::ranges::for_each(tokens, [&](const auto &t) {
        groups[static_cast<size_t>(t.type)].emplace_back(t.content);
    });


    auto &functions = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDefinition)];
    auto &structs = groups[static_cast<size_t>(ast::GlobalStateType::StructDefinition)];
    auto &enums = groups[static_cast<size_t>(ast::GlobalStateType::EnumDefinition)];
    auto &typedefs = groups[static_cast<size_t>(ast::GlobalStateType::Typedef)];
    auto &varDecls = groups[static_cast<size_t>(ast::GlobalStateType::VariableDeclaration)];
    auto &funcDecls = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDeclaration)];

    functionSymbolTable.reserve(ast::Type::BaseTypes.size() + functions.size() + funcDecls.size());

    typeSymbolTable.reserve(ast::Type::BaseTypes.size() + structs.size() + enums.size() + typedefs.size());

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

    for (auto &typedefDef: typedefs) {
        typeDefParser(typedefDef);
    }
}
