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


// Get*Type 函数已移至 Type.cppm 中作为内联模板实现

