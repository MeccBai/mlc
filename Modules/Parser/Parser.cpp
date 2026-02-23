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

std::vector<std::string_view> argSplit(std::string_view str) {
    std::vector<std::string_view> results;
    int depth = 0;
    size_t start = 0;

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '(' || str[i] == '{' || str[i] == '[') depth++;
        else if (str[i] == ')' || str[i] == '}' || str[i] == ']') depth--;
        else if (str[i] == ',' && depth == 0) {
            results.push_back(str.substr(start, i - start));
            start = i + 1;
        }
    }
    results.push_back(str.substr(start)); // 别忘了最后一块
    return results;
}

std::vector<std::string_view> split(std::string_view str, std::string_view delimiter) {
    std::vector<std::string_view> result;
    size_t start = 0;
    while (true) {
        const size_t pos = str.find(delimiter, start);
        if (pos == std::string_view::npos) {
            result.push_back(str.substr(start));
            break;
        }
        result.push_back(str.substr(start, pos - start));
        start = pos + delimiter.length();
    }
    return result;
}

ast::Statement astClass::statementParser(ContextTable<ast::VariableStatement>& _context,const std::string_view _statementContent) {
    if (_statementContent.find(' ') != std::string_view::npos) {
        return variableParser(_context,_statementContent)[0];
    }
    if (const auto pos = _statementContent.find('='); pos != std::string_view::npos) {
        const auto left = _statementContent.substr(0, pos);
        const auto right = _statementContent.substr(pos + 1);
        return ast::AssignStatement(expressionParser(left), expressionParser(right));
    }
    if (_statementContent.find('(') != std::string_view::npos) {
        if (const auto pos = _statementContent.find("if("); pos == 0) {
            return subScopeParser(_context,_statementContent);
        }
        if (const auto pos = _statementContent.find("while("); pos == 0) {
            return subScopeParser(_context,_statementContent);
        }
        if (const auto pos = _statementContent.find("switch("); pos == 0) {
            return subScopeParser(_context,_statementContent);
        }
        const auto functionName = _statementContent.substr(0, _statementContent.find('('));
        const auto argsStr = _statementContent.substr(_statementContent.find('(') + 1,
                                                      _statementContent.length() -
                                                      functionName.length() - 1);
        std::vector<ast::Expression> args = argSplit(argsStr) | std::views::transform([this, &_context](std::string_view arg) {
            ;
            return expressionParser(_context,arg);
        }) | std::ranges::to<std::vector<ast::Expression> >();
        return ast::FunctionCallStatement(functionName, args);
    }
    ErrorPrintln("Invalid statement '{}'\n", _statementContent);
    std::exit(-1);
}




ast::Type::EnumDefinition astClass::enumDefParser(std::string_view _enumContent) {
    const auto pos = _enumContent.find(' ');
    const auto enumName = _enumContent.substr(pos, _enumContent.find('{') - pos);
    const auto memberStr = _enumContent.substr(_enumContent.find('{') + 1,
                                               _enumContent.rfind('}') - _enumContent.find('{') - 1);
    auto memberDefs = split(memberStr, ",");

    auto options = memberDefs | std::views::transform([](std::string_view member) {
        return std::string(member);
    }) | std::ranges::to<std::vector<std::string> >();

    return ast::Type::EnumDefinition(enumName, options);
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
        auto dummyContext = ContextTable<ast::VariableStatement>{};
        for (const auto varParsed = variableParser(dummyContext,varDecl); auto &v: varParsed) {
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
