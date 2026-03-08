//
// Created by Administrator on 2026/2/20.
//

module Parser;
import std;
import aux;
import :Decl;

//[if(p==0){a.x=10;}else{a.y=10;}]

ast::Type::EnumDefinition astClass::enumDefParser(std::string_view _enumContent) {
    const auto pos = _enumContent.find(' ');
    const auto enumName = _enumContent.substr(pos, _enumContent.find('{') - pos);
    const auto memberStr = _enumContent.substr(_enumContent.find('{') + 1,
                                               _enumContent.rfind('}') - _enumContent.find('{') - 1);
    auto memberDefs = split(memberStr, ",");

    auto options = memberDefs | std::views::transform([](const std::string_view member) {
        return std::string(member);
    }) | std::ranges::to<std::vector<std::string> >();

    auto name = enumName;
    while (!name.empty() && std::isspace(name.front())) name.remove_prefix(1);
    while (!name.empty() && std::isspace(name.back())) name.remove_suffix(1);

    return ast::Type::EnumDefinition(name, options);
}


astClass::AbstractSyntaxTree(const std::vector<seg::TokenStatement> &tokens) {
    std::vector<std::string_view> groups[7];

    std::ranges::for_each(tokens, [&](const auto &t) {
        groups[static_cast<size_t>(t.type)].emplace_back(t.content);
    });

    auto &functions = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDefinition)];
    auto &structs = groups[static_cast<size_t>(ast::GlobalStateType::StructDefinition)];
    auto &enums = groups[static_cast<size_t>(ast::GlobalStateType::EnumDefinition)];
    auto &varDecls = groups[static_cast<size_t>(ast::GlobalStateType::VariableDeclaration)];
    auto &funcDecls = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDeclaration)];
    auto &imports = groups[static_cast<size_t>(ast::GlobalStateType::ImportFile)];

    auto paths = getImportPaths(imports);

    for (auto type: ast::Type::BaseTypes) {
        auto typePtr = ast::Make<ast::Type::CompileType>(type);
        typeSymbolTable.insert(typePtr);
        functionSymbolTable.insert(
            ast::Make<ast::FunctionDeclaration>(
                ast::FunctionDeclaration(
                    type.Name, typePtr, {},
                    true, true, true
                )
            )
        );
    }

    std::ranges::for_each(paths, [&](const std::filesystem::path &path) {
        extractExportSymbols(path);
    });

    for (auto &enumDef: enums) {
        auto enumParsed = enumDefParser(enumDef);
        auto enumPtr = ast::Make<ast::Type::CompileType>(enumParsed);
        typeSymbolTable.insert(enumPtr);
    }
    for (auto structDefs = structDefParser(structs); auto &structDef: structDefs) {
        auto structPtr = ast::Make<ast::Type::CompileType>(structDef);
        typeSymbolTable.insert(structPtr);
    }

    for (auto &decl: funcDecls) {
        auto declParsed = functionDeclParser(decl);
        functionSymbolTable.insert(ast::Make<ast::FunctionDeclaration>(declParsed));
    }

    for (auto &varDecl: varDecls) {
        auto dummyContext = ContextTable<ast::VariableStatement>{};
        for (const auto varParsed = variableParser(dummyContext, varDecl); const auto &v: varParsed) {
            auto *vPtr = std::get_if<ast::VariableStatement>(v.operator->());
            auto shadowPtr = std::shared_ptr<ast::VariableStatement>(v, vPtr);
            variableSymbolTable.insert(shadowPtr);
        }
    }

    for (auto &func: functions) {
        auto [decl, body] = functionDeclSpliter(func);
        functionSymbolTable.insert(ast::Make<ast::FunctionDeclaration>(decl));
    }

    for (auto &func: functions) {
        auto def = functionDefParser(func);
        functionScopeTable.insert(ast::Make<ast::FunctionScope>(def));
    }
}


auto astClass::findType(const std::string_view _typeName) const -> sOptional<ast::Type::CompileType> {
    for (const auto &typePtr: typeSymbolTable) {
        if (std::visit([](auto &&arg) -> std::string_view {
            return arg.Name;
        }, *typePtr) == _typeName) {
            return typePtr;
        }
    }
    return std::nullopt;
}
