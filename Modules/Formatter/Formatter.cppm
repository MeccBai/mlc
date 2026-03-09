//
// Created by Administrator on 2026/3/9.
//

export module Formatter;
import Parser;
import Token;
import std;
import Json;

export namespace mlc::ast::fmt {
    using astClass = parser::AbstractSyntaxTree;
    using json = nlohmann::json;
    using sComType = std::shared_ptr<Type::CompileType>;

    using sComType = std::shared_ptr<Type::CompileType>;

    json formatType(const sComType &_type);

    json FormatExportTable(const astClass::ExportTable &_exportTable);

    astClass::ExportTable ParseExportTable(astClass &_ast, const std::filesystem::path &_importPath);

    sComType parseCompileType(astClass &_ast, const json &_json, std::set<sComType> &_types);

    astClass::ExportTable GenerateCache(const std::filesystem::path &_sourcePath);
}

bool CheckCache(const std::filesystem::path &_sourcePath);
