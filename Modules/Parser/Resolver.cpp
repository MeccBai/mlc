//
// Created by Administrator on 2026/3/8.
//

module Parser;
import Prepare;
import std;
import aux;
import Formatter;
import :Decl;


std::vector<std::filesystem::path> importedFiles;
std::unordered_set<std::filesystem::path> importedFileNames;

astClass::ExportTable astClass::ExtractExportSymbols() {
#if 0
    std::ifstream file(_importPath);
    std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    if (source.empty()) {
        ErrorPrintln("Error: failed to read '{}'\n", _importPath.string());
        std::exit(-1);
    }

    auto content = mlc::prepare::Prepare(source);
    auto tokens = seg::TopTokenize(content);

    std::vector<std::pair<std::string, bool> > groups[7];
    std::ranges::for_each(tokens, [&](const auto &t) {
        groups[static_cast<size_t>(t.type)].emplace_back(t.content, t.exported);
    });
    auto functions = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDefinition)];
    auto structs = groups[static_cast<size_t>(ast::GlobalStateType::StructDefinition)];
    auto enums = groups[static_cast<size_t>(ast::GlobalStateType::EnumDefinition)];
    auto funcDecls = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDeclaration)];
    const auto imports = groups[static_cast<size_t>(ast::GlobalStateType::ImportFile)];

    using tokenResult = std::pair<std::string, bool>;
    auto tokenToContent = [](const tokenResult &t) { return t.first; };
    auto tokenToExported = [](const tokenResult &t) { return t.second; };

    for (const auto& importFile : imports) {
        auto result = tokenToContent(importFile);
    }

    std::ranges::for_each(
        getImportPaths(imports | std::views::transform(tokenToContent) | std::ranges::to<std::vector>(),
                       _importPath.parent_path()),
        [this](const std::filesystem::path &_path) {
            ExtractExportSymbols(_path);
        });

    for (auto &enumDef: enums) {
        auto enumParsed = enumDefParser(tokenToContent(enumDef));
        auto enumPtr = ast::Make<ast::Type::CompileType>(enumParsed);
        typeSymbolTable.insert(enumPtr);
    }

    for (auto structDefs = structDefParser(
             structs | std::views::transform(tokenToContent) | std::ranges::to<std::vector>()); auto [structDef ,
             isExported]: std::views::zip(
             structDefs, structs | std::views::transform(tokenToExported) | std::ranges::to<std::vector>())) {
        if (isExported) {
            auto structPtr = ast::Make<ast::Type::CompileType>(structDef);
            typeSymbolTable.insert(structPtr);
        }
    }

    for (auto &[body, exported]: funcDecls) {
        if (exported) {
            auto declParsed = functionDeclParser(body, exported);
            functionSymbolTable.insert(ast::Make<ast::FunctionDeclaration>(declParsed));
        }
    }

    for (auto &[funcBody, exported]: functions) {
        if (exported) {
            auto [decl, _] = functionDeclSpliter(funcBody, exported);
            functionSymbolTable.insert(ast::Make<ast::FunctionDeclaration>(decl));
        }
    }
    return {typeSymbolTable, functionSymbolTable};
#endif



    auto typeTable = typeSymbolTable | std::views::filter([](const sPtr<type::CompileType>& _type) {
        if (auto *const type = std::get_if<type::StructDefinition >(&*_type)) {
            return type->isExported;
        }
        if (auto *const type = std::get_if<type::EnumDefinition >(&*_type)) {
            return type->isExported;
        }
        return false;
    }) | std::ranges::to<std::set<sPtr<type::CompileType> > >();
    auto funcTable = functionSymbolTable | std::views::filter([](const sPtr<ast::FunctionDeclaration>& _func) {
        return _func->IsExported && !_func->IsTypeConvert;
    }) | std::ranges::to<std::set<sPtr<ast::FunctionDeclaration> > >();


    return {typeTable, funcTable};
}

std::vector<std::filesystem::path> astClass::getImportPaths(
    const std::vector<std::string> &_tokens, const std::filesystem::path &_currentPath) {
    const std::filesystem::path sysLibPath = "F:\\Develop\\A.Projects\\C++\\mlc\\lib\\";

    return _tokens | std::views::transform([&](const std::string &t) -> std::filesystem::path {
               auto pathStr = std::string(t.substr(7, t.size() - 8));
               std::ranges::replace(pathStr, '.', '\\');
               pathStr += ".mc";
               const auto filePath = std::filesystem::path(pathStr);
               if (auto path1 = sysLibPath / filePath; std::filesystem::exists(path1)) return path1;
               if (auto path2 = _currentPath / filePath; std::filesystem::exists(path2))
                   return
                           path2;
               ErrorPrintln("Error:failed to find '{}'\n", filePath.generic_string());
               std::exit(-1);
           })
           | std::ranges::to<std::vector<std::filesystem::path> >();
}

std::vector<std::filesystem::path> astClass::GetImportPaths(const std::filesystem::path &_importPath) {
    std::ifstream file(_importPath);
    const std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    if (source.empty()) {
        ErrorPrintln("Error: failed to read '{}'\n", _importPath.string());
        std::exit(-1);
    }
    auto content = mlc::prepare::Prepare(source);
    auto tokens = seg::TopTokenize(content);
    auto imports = tokens | std::views::filter([](const auto &t) {
        return t.type == ast::GlobalStateType::ImportFile;
    }) | std::views::transform([](const auto &t) { return t.content; }) | std::ranges::to<std::vector
        <std::string> >();

    return getImportPaths(imports, _importPath.parent_path());
}
