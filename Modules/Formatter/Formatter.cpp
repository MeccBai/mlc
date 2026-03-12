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

json formatStruct(const sComType &_type) {
    json j;
    const auto *const structType = type::GetType<type::StructDefinition>(_type);
    j[afmt::Kind] = afmt::Struct;
    j[afmt::Name] = structType->Name;
    j[afmt::Members] = json::array();

    auto formatMember = [](const sComType &_member) {
        if (type::GetType<type::StructDefinition>(_member)) {
            json j;
            j[afmt::Kind] = afmt::Struct;
            j[afmt::Name] = type::GetTypeName(*_member);
            return j;
        }
        return afmt::formatType(_member);
    };

    for (const auto &[fieldName, fieldType]: structType->Members) {
        json fieldJson;
        fieldJson[afmt::Name] = fieldName;
        fieldJson[afmt::Type] = formatMember(fieldType);
        j[afmt::Members].push_back(fieldJson);
    }
    return j;
}

json formatPointer(const sComType &_type) {
    json j;
    const auto *const pointerType = type::GetType<type::PointerType>(_type);
    j[afmt::Kind] = afmt::Pointer;
    j[afmt::BaseType] = type::GetTypeName(*pointerType->BaseType);
    j[afmt::Level] = pointerType->PointerLevel;
    j[afmt::Name] = std::string(type::GetTypeName(*pointerType->BaseType)) +
                    std::string(pointerType->PointerLevel, 'p');
    return j;
}

json formatEnum(const sComType &_type) {
    json j;
    const auto *const enumType = type::GetType<type::EnumDefinition>(_type);
    j[afmt::Kind] = afmt::Enum;
    j[afmt::Name] = enumType->Name;
    j[afmt::Options] = json::array();
    for (const auto &option: enumType->Values) {
        j[afmt::Options].push_back(option);
    }
    return j;
}

json afmt::formatType(const sComType &_type) {
    if (std::holds_alternative<type::StructDefinition>(*_type)) {
        return formatStruct(_type);
    }
    if (std::holds_alternative<type::PointerType>(*_type)) {
        return formatPointer(_type);
    }
    if (std::holds_alternative<type::EnumDefinition>(*_type)) {
        return formatEnum(_type);
    }
    if (auto *const baseType = type::GetType<type::BaseType>(_type); baseType) {
        json j;
        j[Kind] = BaseType;
        j[Name] = type::GetTypeName(*_type);
        return j;
    }
    ErrorPrintln("Invalid type for export : {}", type::GetTypeName(*_type));
    std::exit(-1);
}

json formatFunction(const sFunc &_func) {
    json j;
    j[afmt::Name] = _func->Name;
    j[afmt::Parameters] = json::array();
    for (const auto &param: _func->Parameters) {
        json paramJson;
        paramJson[afmt::Name] = param->Name;
        paramJson[afmt::Type] = afmt::formatType(param->VarType);
        j[afmt::Parameters].push_back(paramJson);
    }
    j[afmt::ReturnType] = afmt::formatType(_func->ReturnType);
    j[afmt::IsVarList] = _func->IsVarList;
    return j;
}

sComType parseEnumType(astClass &_ast, const json &_json) {
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
    return ast::Make<type::CompileType>(type::EnumDefinition(name, options, false));
}

std::set<sComType> parseStructTypes(astClass &_ast, const std::vector<json> &_jArray, std::set<sComType> &_types) {
    std::set<sComType> structTypes;
    auto localFind = [&structTypes, _types, _ast](const std::string_view _name) -> std::optional<sComType> {
        for (auto type: _types) {
            if (std::holds_alternative<type::StructDefinition>(*type) &&
                std::get<type::StructDefinition>(*type).Name == _name) {
                return type;
            }
        }
        for (auto type: structTypes) {
            if (std::holds_alternative<type::StructDefinition>(*type) &&
                std::get<type::StructDefinition>(*type).Name == _name) {
                return type;
            }
        }
        if (const auto type = _ast.FindType(_name)) {
            return type;
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
                auto memberTypePtr = localFind(memberType[afmt::Name].get<std::string>());
                if (!memberTypePtr) {
                    memberTypePtr = _ast.FindType(memberType[afmt::Name].get<std::string>());
                    if (!memberTypePtr) {
                        ErrorPrintln("Error: Unknown type '{}' for struct member '{}'\n",
                                     memberType[afmt::Name].get<std::string>(), memberName);
                        std::exit(-1);
                    }
                }
                structMembers.push_back({memberName, memberTypePtr.value()});
            }
        }
        structTypes.insert(ast::Make<type::CompileType>(type::StructDefinition(name, structMembers)));
    }

    for (const auto &[ptrType, baseTypeName]: lazyPointers) {
        auto baseType = _ast.FindType(baseTypeName);
        if (!baseType) {
            baseType = localFind(baseTypeName);
            if (!baseType) {
                ErrorPrintln("Error: Unknown type '{}' for struct member pointer", baseTypeName);
                std::exit(-1);
            }
        }
        auto *specificPtr = type::GetType<ast::Type::PointerType>(ptrType);
        specificPtr->Finalize(baseType.value());
    }

    return structTypes;
}

sComType parsePointerType(astClass &_ast, const json &_json, std::set<sComType> &_types) {
    auto localFind = [_types, _ast](const std::string_view _name) -> std::optional<sComType> {
        for (auto type: _types) {
            if (std::holds_alternative<type::StructDefinition>(*type) &&
                std::get<type::StructDefinition>(*type).Name == _name) {
                return type;
            }
        }
        if (const auto type = _ast.FindType(_name)) {
            return type;
        }
        return std::nullopt;
    };
    if (!_json.contains(afmt::BaseType) || !_json[afmt::BaseType].is_string()) {
        ErrorPrintln("Invalid pointer type: missing or invalid 'baseType' field.");
        std::exit(-1);
    }
    if (!_json.contains(afmt::Level) || !_json[afmt::Level].is_number_unsigned()) {
        ErrorPrintln("Invalid pointer type: missing or invalid 'level' field.");
        std::exit(-1);
    }
    const auto baseType = localFind(_json[afmt::BaseType][afmt::Name].get<std::string>());
    const auto level = _json[afmt::Level].get<size_t>();
    const auto pointerType = std::make_shared<type::PointerType>(level);
    pointerType->Finalize(baseType.value());
    return ast::Make<type::CompileType>(*pointerType);
}

sComType afmt::parseCompileType(astClass &_ast, const json &_json, std::set<sComType> &_types) {
    if (!_json.contains(Kind) || !_json[Kind].is_string()) {
        ErrorPrintln("Invalid type: missing or invalid 'kind' field.");
        std::exit(-1);
    }
    const auto kind = _json[Kind].get<std::string>();
    if (kind == Struct) {
        ErrorPrintln("Struct type must be defined before use.");
        std::exit(-1);
    }
    if (kind == Pointer) {
        return parsePointerType(_ast, _json, _types);
    }
    if (kind == Enum) {
        return parseEnumType(_ast, _json);
    }
    ErrorPrintln("Invalid type: unknown kind '{}'.", kind);
    std::exit(-1);
}

sFunc parseFunction(const astClass &_ast, const json &_func, std::set<sComType> &_types) {
    if (!_func.contains(afmt::Name) || !_func[afmt::Name].is_string()) {
        ErrorPrintln("Invalid function: missing or invalid 'name' field.");
        std::exit(-1);
    }
    if (!_func.contains(afmt::Parameters) || !_func[afmt::Parameters].is_array()) {
        ErrorPrintln("Invalid function: missing or invalid 'parameters' field.");
        std::exit(-1);
    }
    if (!_func.contains(afmt::ReturnType) || !_func[afmt::ReturnType].is_object()) {
        ErrorPrintln("Invalid function: missing or invalid 'returnType' field.");
        std::exit(-1);
    }
    auto localFind = [_types, _ast](const std::string_view _name) -> std::optional<sComType> {
        for (auto type: _types) {
            if (std::holds_alternative<type::StructDefinition>(*type) &&
                std::get<type::StructDefinition>(*type).Name == _name) {
                return type;
            }
        }
        if (const auto type = _ast.FindType(_name)) {
            return type;
        }
        return std::nullopt;
    };
    const auto funcName = _func[afmt::Name].get<std::string>();
    auto returnType = localFind(_func[afmt::ReturnType][afmt::Name].get<std::string>());
    const bool isVarList = _func[afmt::IsVarList].get<bool>();
    if (_func[afmt::ReturnType][afmt::Kind] == afmt::Pointer) {
        const auto baseType = _func[afmt::ReturnType][afmt::BaseType].get<std::string>();
        auto baseTypePtr = localFind(baseType);
        if (!baseTypePtr) {
            baseTypePtr = _ast.FindType(baseType);
            if (!baseTypePtr) {
                ErrorPrintln("Error: Unknown return type '{}' for function '{}'\n", baseType, funcName);
                std::exit(-1);
            }
        }
        returnType = std::make_shared<type::CompileType>(
            type::PointerType(_func[afmt::ReturnType][afmt::Level].get<size_t>()));
        auto returnBaseTypePtr = baseTypePtr.value();
        auto *ptr = type::GetType<type::PointerType>(*returnType);
        ptr->Finalize(returnBaseTypePtr);
    }

    ast::FunctionDeclaration::Args parameters;
    for (const auto &param: _func[afmt::Parameters]) {
        if (!param.contains(afmt::Name) || !param[afmt::Name].is_string()) {
            ErrorPrintln("Invalid function parameter: missing or invalid 'name' field.");
            std::exit(-1);
        }
        if (!param.contains(afmt::Type) || !param[afmt::Type].is_object()) {
            ErrorPrintln("Invalid function parameter: missing or invalid 'type' field.");
            std::exit(-1);
        }
        const auto paramName = param[afmt::Name].get<std::string>();
        const auto paramType = param[afmt::Type][afmt::Name].get<std::string>();
        auto paramTypePtr = localFind(paramType);
        if (param[afmt::Type][afmt::Kind] == afmt::Pointer) {
            auto baseType = param[afmt::Type][afmt::BaseType].get<std::string>();
            auto baseTypePtr = localFind(baseType);
            if (!baseTypePtr) {
                baseTypePtr = _ast.FindType(baseType);
                if (!baseTypePtr) {
                    ErrorPrintln("Error: Unknown type '{}' for function parameter '{}'\n", baseType, paramName);
                    std::exit(-1);
                }
            }
            paramTypePtr = std::make_shared<type::CompileType>(
                type::PointerType(param[afmt::Type][afmt::Level].get<size_t>()));
            auto paramBaseTypePtr = baseTypePtr.value();
            auto *ptr = type::GetType<type::PointerType>(*paramTypePtr);
            ptr->Finalize(paramBaseTypePtr);
        }
        if (!paramTypePtr) {
            ErrorPrintln("Error: Unknown type '{}' for function parameter '{}'\n", paramType, paramName);
            std::exit(-1);
        }
        parameters.push_back(ast::Make<ast::Variable>(ast::Variable(paramName, paramTypePtr.value(), nullptr)));
    }
    return std::make_shared<ast::FunctionDeclaration>(
        ast::FunctionDeclaration(funcName, returnType.value(), parameters, isVarList, false));
}

afmt::astClass::ExportTable afmt::ParseExportTable(astClass &_ast, const std::filesystem::path &_importPath) {
    std::ifstream file(_importPath);
    const auto &jsonCachePath = _importPath;
    if (auto jsonPath = jsonCachePath.parent_path() / ".cache" / jsonCachePath.filename().replace_extension(".json");
        std::filesystem::exists(jsonPath) && checkCache(_importPath)) {
        auto jsonFile = std::ifstream(jsonPath);
        if (!jsonFile.is_open()) {
            ErrorPrintln("Error: failed to open '{}'", jsonPath.string());
            std::exit(-1);
        }
        const auto jsonContent = std::string((std::istreambuf_iterator(jsonFile)), std::istreambuf_iterator<char>());
        const auto content = json::parse(jsonContent);
        const auto& types = content[Types];
        const auto& functions = content[Functions];
        auto typeTable = std::set<sComType>();
        auto structs = std::vector<json>();
        auto enums = std::vector<json>();
        for (const auto &type: types) {
            if (type[Kind] == Struct) {
                structs.push_back(type);
            }
            if (type[Kind] == Enum) {
                enums.push_back(type);
            }
        }
        auto FindType = [&typeTable](const std::string_view _name) -> std::optional<sComType> {
            for (const auto &type: typeTable) {
                if (std::holds_alternative<type::StructDefinition>(*type) &&
                    std::get<type::StructDefinition>(*type).Name == _name) {
                    return type;
                }
                if (std::holds_alternative<type::EnumDefinition>(*type) &&
                    std::get<type::EnumDefinition>(*type).Name == _name) {
                    return type;
                }
            }
            return std::nullopt;
        };

        for (const auto &type: enums) {
            auto typePtr = afmt::parseCompileType(_ast, type, typeTable);
            if (FindType(type[afmt::Name].get<std::string>())) {
                ErrorPrintln("Error: Duplicate type name '{}' in export table.\n", type[afmt::Name].get<std::string>());
                std::exit(-1);
            }
            typeTable.insert(typePtr);
        }
        auto structTypes = parseStructTypes(_ast, structs, typeTable);
        typeTable.insert(structTypes.begin(), structTypes.end());

        auto funcTable = std::set<sFunc>();
        for (const auto &func: functions) {
            funcTable.insert(parseFunction(_ast, func, typeTable));
        }

        auto result = astClass::ExportTable(typeTable, funcTable);
        return result;
    }
    return GenerateCache(_importPath);
}

json afmt::FormatExportTable(const astClass::ExportTable &_exportTable) {
    json j;
    j[Types] = json::array();

    auto [typeTable, functionTable] = _exportTable;

    for (const auto &typeEntry: typeTable) {
        j[Types].push_back(formatType(typeEntry));
    }
    j[Functions] = json::array();
    for (const auto &funcEntry: functionTable) {
        j[Functions].push_back(formatFunction(funcEntry));
    }
    return j;
}
