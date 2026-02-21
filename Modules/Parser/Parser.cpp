//
// Created by Administrator on 2026/2/20.
//

module Parser;
import std;
import aux;

using astClass = mlc::parser::AbstractSyntaxTree;

template<typename _type>
using sPtr = std::shared_ptr<_type>;
namespace ast = mlc::ast;
using size_t = std::size_t;
//[if(p==0){a.x=10;}else{a.y=10;}]

using astClass = mlc::parser::AbstractSyntaxTree;

template<typename _type>
using sPtr = std::shared_ptr<_type>;

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
    return ast::Type::EnumDefinition("enum",enums);
}

void astClass::typeDefParser(std::string_view _typedefContent) {
}


std::vector<ast::VariableStatement> astClass::variableParser(
    const std::string_view variableContent) {
    // 这里可以进一步解析变量声明，提取变量类型、名称和初始化表达式等信息
    std::println("{}", variableContent);

    const auto pos = variableContent.find(' ');
    auto type = variableContent.substr(0, pos);
    const auto declaration = variableContent.substr(pos + 1);

    const auto it = std::ranges::find_if(typeSymbolTable, [&](const auto &t) {
        return std::visit([](auto &&arg) -> std::string_view {
            return arg.Name; // 只要所有子类都有 Name，这个就能编译通过
        }, *t) == type;
    });

    if (it == typeSymbolTable.end()) {
        ErrorPrintln("Error: Unknown type '{}'\n", type);
    }

    std::vector<std::string_view> declarations;
    std::stack<char> brackets;
    size_t start = 0;
    for (auto i = 0; i < declaration.length(); i++) {
        if (const char c = declaration[i]; c == '{' || c == '(' || c == '[') {
            brackets.push(c);
        } else if (c == '}' || c == ')' || c == ']') {
            if (brackets.empty()) {
                ErrorPrintln("MLC Syntax Error: Unmatched closing bracket '", c, "' at position ", i);
                std::exit(-1);
            }

            if (char top = brackets.top(); (c == '}' && top == '{') || (c == ')' && top == '(') || (
                                               c == ']' && top == '[')) {
                brackets.pop();
            } else {
                ErrorPrintln("MLC Syntax Error: Mismatched bracket. Found '", c, "' but expected match for '", top,
                             "'");
                std::exit(-1);
            }
        } else if (c == ',' && brackets.empty()) {
            declarations.emplace_back(declaration.substr(start, i - start));
            start = i + 1;
        }
    }
    if (start < declaration.length()) {
        // 别忘了清理一下可能残留的空格（虽然你已经脱水了，但安全第一）
        if (std::string_view lastItem = declaration.substr(start); !lastItem.empty()) {
            declarations.emplace_back(lastItem);
        }
    }

    for (auto d: declarations) {
        std::println("Declaration: [{}]", d);
    }

    return {};
    //ast::VariableStatement("", {}, std::nullopt);
}


ast::FunctionScope astClass::functionDefParser(std::string_view functionContent) {
    auto statements = mlc::seg::TokenizeFunctionBody(functionContent);
    // 这里可以进一步解析函数头，提取参数和返回类型等信息
    return ast::FunctionScope("", {}, {}, {});
}

ast::FunctionDeclaration mlc::parser::AbstractSyntaxTree::functionDeclParser(std::string_view functionContent) {
    return ast::FunctionDeclaration("", {}, {}, false);
}

astClass::AbstractSyntaxTree(const std::vector<mlc::seg::TokenStatement> &tokens) {
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
