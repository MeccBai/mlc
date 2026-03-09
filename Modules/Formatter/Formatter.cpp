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

namespace ast = mlc::ast;
namespace type = ast::Type;
namespace afmt = ast::fmt;

using json = afmt::json;
using sComType = afmt::sComType;
using sFunc = std::shared_ptr<ast::FunctionDeclaration>;
using astClass = mlc::parser::AbstractSyntaxTree;
using size_t = std::size_t;

json formatStruct(const sComType &_type) {
    json j;
    const auto *const structType = std::get_if<type::StructDefinition>(&*_type);
    j[afmt::Kind] = afmt::Struct;
    j[afmt::Name] = structType->Name;
    j[afmt::Members] = json::array();
    for (const auto &[fieldName, fieldType]: structType->Members) {
        json fieldJson;
        fieldJson[afmt::Name] = fieldName;
        fieldJson[afmt::Type] = afmt::formatType(fieldType);
        j[afmt::Members].push_back(fieldJson);
    }
    return j;
}

json formatPointer(const sComType &_type) {
    json j;
    const auto *const pointerType = std::get_if<type::PointerType>(&*_type);
    j[afmt::Kind] = afmt::Pointer;
    j[afmt::BaseType] = type::GetTypeName(*pointerType->BaseType);
    j[afmt::Level] = pointerType->PointerLevel;
    j[afmt::Name] = std::string(type::GetTypeName(*pointerType->BaseType)) +
                    std::string(pointerType->PointerLevel, 'p');
    return j;
}

json formatEnum(const sComType &_type) {
    json j;
    const auto *const enumType = std::get_if<type::EnumDefinition>(&*_type);
    j[afmt::Kind] = afmt::Enum;
    j[afmt::Name] = enumType->Name;
    j[afmt::Options] = json::array();
    for (const auto &option: enumType->Values) {
        j[afmt::Options].push_back(option);
    }
    return j;
}

json formatType(const sComType &_type) {
    if (std::holds_alternative<type::StructDefinition>(*_type)) {
        return formatStruct(_type);
    }
    if (std::holds_alternative<type::PointerType>(*_type)) {
        return formatPointer(_type);
    }
    if (std::holds_alternative<type::EnumDefinition>(*_type)) {
        return formatEnum(_type);
    } {
        ErrorPrintln("Invalid type for export : {}", type::GetTypeName(*_type));
        std::exit(-1);
    }
}

json formatFunction(const sFunc &_func) {
    json j;
    j[afmt::Name] = _func->Name;
    j[afmt::Parameters] = json::array();
    for (const auto &param: _func->Parameters) {
        json paramJson;
        paramJson[afmt::Name] = param->Name;
        paramJson[afmt::Type] = formatType(param->VarType);
        j[afmt::Parameters].push_back(paramJson);
    }
    j[afmt::ReturnType] = formatType(_func->ReturnType);
    return j;
}

sComType parseEnumType(astClass &_ast, json &_json) {
    if (!_json.contains(afmt::Name) || !_json[afmt::Name].is_string()) {
        ErrorPrintln("Invalid enum type: missing or invalid 'name' field.");
        std::exit(-1);
    }
    if (!_json.contains(afmt::Options) || !_json[afmt::Options].is_array()) {
        ErrorPrintln("Invalid enum type: missing or invalid 'options' field.");
        std::exit(-1);
    }
    auto options = std::vector<std::string>();
    for (const auto &option: _json[afmt::Options]) {
        if (!option.is_string()) {
            ErrorPrintln("Invalid enum type: all options must be strings.");
            std::exit(-1);
        }
        options.push_back(option.get<std::string>());
    }
    const auto name = _json[afmt::Name].get<std::string>();
    return ast::Make<type::CompileType>(type::EnumDefinition(name, options));
}

std::vector<sComType> parseStructTypes(astClass &_ast, std::vector<json> &_jArray) {
    std::vector<sComType> structTypes;
    auto localStructFind = [&structTypes](const std::string_view _name) -> std::optional<sComType> {
        for (const auto &type: structTypes) {
            if (const auto *const structDef = std::get_if<type::StructDefinition>(&*type)) {
                if (structDef->Name == _name) {
                    return type;
                }
            }
        }
        return std::nullopt;
    };

    std::vector<std::pair<type::sPtr<type::CompileType>, std::string> > lazyPointers;
    for (const auto &j: _jArray) {
        if (!j.contains(afmt::Name) || !j[afmt::Name].is_string()) {
            ErrorPrintln("Invalid struct type: missing or invalid 'name' field.");
            std::exit(-1);
        }
        auto name = j[afmt::Name].get<std::string>();
        auto members = j[afmt::Members];
        std::vector<type::StructMember> structMembers;
        for (const auto &member: members) {
            if (!member.contains(afmt::Name) || !member[afmt::Name].is_string()) {
                ErrorPrintln("Invalid struct member: missing or invalid 'name' field.");
                std::exit(-1);
            }
            const auto memberName = member[afmt::Name].get<std::string>();
            if (!member.contains(afmt::Type) || !member[afmt::Type].is_object()) {
                ErrorPrintln("Invalid struct member: missing or invalid 'type' field.");
                std::exit(-1);
            }
            if (auto memberType = member[afmt::Type]; memberType[afmt::Kind] == afmt::Pointer) {
                auto pointer = std::make_shared<type::CompileType>(type::PointerType(1));
                lazyPointers.emplace_back(pointer, memberType[afmt::BaseType].get<std::string>());
                structMembers.push_back({memberName, pointer});
            } else {
                structMembers.push_back({memberName, afmt::parseCompileType(_ast, memberType)});
            }
        }
        structTypes.push_back(ast::Make<type::CompileType>(type::StructDefinition(name, structMembers)));
    }

    for (const auto &[ptrType, baseTypeName]: lazyPointers) {
        auto baseType = _ast.FindType(baseTypeName);
        if (!baseType) {
            baseType = localStructFind(baseTypeName);
            if (!baseType) {
                ErrorPrintln("Error: Unknown type '{}' for struct member pointer\n", baseTypeName);
                std::exit(-1);
            }
        }
        auto *specificPtr = std::get_if<ast::Type::PointerType>(ptrType.get());
        specificPtr->Finalize(baseType.value());
    }

    return structTypes;
}

sComType parsePointerType(astClass &_ast, json &_json) {
    if (!_json.contains(afmt::BaseType) || !_json[afmt::BaseType].is_string()) {
        ErrorPrintln("Invalid pointer type: missing or invalid 'baseType' field.");
        std::exit(-1);
    }
    if (!_json.contains(afmt::Level) || !_json[afmt::Level].is_number_unsigned()) {
        ErrorPrintln("Invalid pointer type: missing or invalid 'level' field.");
        std::exit(-1);
    }
    const auto baseType = afmt::parseCompileType(_ast, _json[afmt::BaseType]);
    const auto level = _json[afmt::Level].get<size_t>();
    const auto pointerType = std::make_shared<type::PointerType>(level);
    pointerType->Finalize(baseType);
    return ast::Make<type::CompileType>(*pointerType);
}

sComType afmt::parseCompileType(astClass &_ast, json &_j) {
    if (!_j.contains(Kind) || !_j[Kind].is_string()) {
        ErrorPrintln("Invalid type: missing or invalid 'kind' field.");
        std::exit(-1);
    }
    const auto kind = _j[Kind].get<std::string>();
    if (kind == Struct) {
        ErrorPrintln("Struct type must be defined before use.");
        std::exit(-1);
    }
    if (kind == Pointer) {
        return parsePointerType(_ast, _j);
    }
    if (kind == Enum) {
        return parseEnumType(_ast, _j);
    }
    ErrorPrintln("Invalid type: unknown kind '{}'.", kind);
    std::exit(-1);
}

afmt::astClass::ExportTable ParseExportTable(astClass &_ast, const std::filesystem::path &_importPath) {
    std::ifstream file(_importPath);
    auto fileContent = std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    if (fileContent.empty()) {
        ErrorPrintln("Error: failed to read '{}'", _importPath.string());
        std::exit(-1);
    }
    auto json = json::parse(fileContent);
}


std::string afmt::FormatExportTable(astClass &_ast, const std::filesystem::path &_sourcePath) {
    json j;
    j[Types] = json::array();

    auto [typeTable, functionTable] = _ast.ExtractExportSymbols(_sourcePath);

    for (const auto &typeEntry: typeTable) {
        j[Types].push_back(formatType(typeEntry));
    }
    j[Functions] = json::array();
    for (const auto &funcEntry: functionTable) {
        j[Functions].push_back(formatFunction(funcEntry));
    }
    return j.dump(4);
}
