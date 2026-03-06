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

size_t ast::Type::StructDefinition::Size() const {
    size_t currentOffset = 0;
    size_t maxAlign = 1;
    for (const auto &[name, type]: Members) {
        const size_t mSize = std::visit([](auto &&t) { return t.Size(); }, *type);
        const size_t mAlign = mSize;
        currentOffset = (currentOffset + mAlign - 1) & ~(mAlign - 1);
        currentOffset += mSize;
        if (mAlign > maxAlign) maxAlign = mAlign;
    }
    return (maxAlign + currentOffset - 1ul) & ~(maxAlign - 1ul);
}


size_t ast::Type::ArrayType::Size() const {
    const size_t baseSize = std::visit([](auto &&t) { return t.Size(); }, *BaseType);
    return baseSize * Length;
}




