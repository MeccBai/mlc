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