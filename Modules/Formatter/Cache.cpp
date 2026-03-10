//
// Created by Administrator on 2026/3/9.
//
module Formatter;
import Parser;
import Token;
import std;
import Compiler;
import Json;
import aux;
import :Pool;
import Prepare;

namespace ast = mlc::ast;
namespace type = ast::Type;
namespace afmt = ast::fmt;

using json = afmt::json;
using sComType = afmt::sComType;
using sFunc = std::shared_ptr<ast::FunctionDeclaration>;
using astClass = mlc::parser::AbstractSyntaxTree;
using size_t = std::size_t;

//JSON cache:
//hash:
//types:[{kind:struct/enum/pointer,name:string, ...}, ...]
//functions:[{name:string, returnType:..., parameters:[{name:string, type:

constexpr std::uint64_t MHash(const std::string_view str) {
    std::uint64_t hash = 0xcbf29ce484222325ULL;
    for (const char c: str) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

bool checkCache(const std::filesystem::path& _sourcePath) {
    auto parentPath = _sourcePath.parent_path() / ".cache";
    auto jsonCachePath = parentPath /_sourcePath.filename().replace_extension(".json");
    if (!std::filesystem::exists(jsonCachePath)) return false;
    std::ifstream sourceFile(_sourcePath);
    std::string sourceContent((std::istreambuf_iterator(sourceFile)), std::istreambuf_iterator<char>());
    auto currentHash = MHash(sourceContent);
    std::ifstream cacheFile(jsonCachePath);
    if (const auto cacheJson = json::parse(cacheFile); !cacheJson.contains("hash") || cacheJson["hash"].get<std::uint64_t>() != currentHash) {
        return false;
    }
    auto imports = astClass::GetImportPaths(_sourcePath);
    auto result = std::ranges::all_of(imports, [](const auto& path) {
        return checkCache(path);
    });
    return result;
}

astClass::ExportTable afmt::GenerateCache(const std::filesystem::path &_sourcePath) {
    const auto& path = _sourcePath;
    auto parentPath = path.parent_path() / ".cache";
    if (!std::filesystem::exists(parentPath)) {
        std::filesystem::create_directory(parentPath);
    }
    auto jsonCachePath = parentPath / path.filename().replace_extension(".json");
    std::ifstream sourceFile(_sourcePath);
    std::string sourceContent((std::istreambuf_iterator(sourceFile)), std::istreambuf_iterator<char>());
    auto hash = MHash(sourceContent);

    std::ifstream cacheFile(jsonCachePath);
    std::string cacheContent((std::istreambuf_iterator(cacheFile)), std::istreambuf_iterator<char>());

    auto imports = astClass::GetImportPaths(_sourcePath);

    const bool cacheResult = std::ranges::all_of(imports, [](const auto &path) {
        return checkCache(path);
    });

    if (!cacheResult && !imports.empty()) {
        ErrorPrintln("Error: Cache is invalid due to changes in imported files. {}",_sourcePath.string());
        std::exit(-1);
    }

    auto file = std::ifstream(_sourcePath);
    const auto code = std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
    auto sourceCode = prepare::Prepare(code);
    const auto result =seg::TopTokenize(sourceCode);
    auto cache = astClass(result,_sourcePath.parent_path());

    auto cacheTable = cache.ExtractExportSymbols();
    json j = FormatExportTable(cacheTable);
    j["hash"] = hash;

    std::ofstream out(jsonCachePath);
    out << j.dump(4);
    out.close();
    return cache.ExtractExportSymbols();
}
