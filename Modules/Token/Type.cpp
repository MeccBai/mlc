//
// Created by Administrator on 2026/2/27.
//

module Token;

import :Type;
import :Statement;
import :Expression;

import aux;
import std;
import Parser;
namespace ast = mlc::ast;
namespace type = ast::Type;

//std::variant<BaseType, StructDefinition, EnumDefinition, PointerType, ArrayType>



bool type::IsArrayOrPointer(const sPtr<CompileType> &_type) {
    return std::holds_alternative<ArrayType>(*_type) || std::holds_alternative<PointerType>(*_type);
}
size_t type::GetSize(const sPtr<CompileType> &_type) {

    return std::visit([]<typename T0>(T0 &&arg) -> size_t {
        using T = std::decay_t<T0>;
        return arg.Size();
    }, *_type);

}


type::ArrayType * type::GetArrayType(const std::shared_ptr<CompileType> &_type) {
    if (_type == nullptr) {
        return nullptr;
    }
    return std::get_if<ArrayType>(&*_type);
}

type::PointerType * type::GetPointerType(const std::shared_ptr<CompileType> &_type) {
    if (_type == nullptr) {
        return nullptr;
    }
    return std::get_if<PointerType>(&*_type);
}

type::BaseType * type::GetBaseType(const std::shared_ptr<CompileType> &_type) {
    if (_type == nullptr) {
        return nullptr;
    }
    return std::get_if<BaseType>(&*_type);
}

type::StructDefinition * type::GetStructDef(const std::shared_ptr<CompileType> &_type) {
    if (_type == nullptr) {
        return nullptr;
    }
    return std::get_if<StructDefinition>(&*_type);
}


type::EnumDefinition * type::GetEnumDef(const std::shared_ptr<CompileType> &_type) {
    if (_type == nullptr) {
        return nullptr;
    }
    return std::get_if<EnumDefinition>(&*_type);
}