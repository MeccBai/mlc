//
// Created by Administrator on 2026/3/3.
//
module Token;

import :Type;
import :Statement;
import :Expression;

import aux;
import std;
import Parser;


std::string ast::Type::ArrayType::GetTypeName() const {
    std::string baseName = std::visit([](auto &&arg) -> std::string {
        // 这里调用各类型的 GetTypeName()
        return arg.Name;
    }, *BaseType);
    return std::format("{}[{}]", baseName, Length);
}

ast::Type::sPtr<ast::FunctionDeclaration> ast::FunctionScope::ToDeclaration() const {
    return Make<FunctionDeclaration>(FunctionDeclaration(Name, ReturnType, Parameters, IsVarList));
}

// Get*Statement 函数已移至 Statement.cppm 中作为内联模板实现
