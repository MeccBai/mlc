//
// Created by Administrator on 2026/3/6.
//
module Token;

import :Type;

import std;
using size_t = std::size_t;
namespace ast = mlc::ast;
namespace type=ast::Type;


size_t type::GetSize(const sPtr<CompileType> &_type) {
    return std::visit([]<typename T0>(T0 &&arg) -> size_t {
        using T = std::decay_t<T0>;
        return arg.Size();
    }, *_type);
}

size_t type::GetSize(const CompileType* _type) {
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
