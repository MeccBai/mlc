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
    return MakeFuncDecl(FunctionDeclaration(Name, ReturnType, Parameters, IsVarList));
}

ast::SubScope * GeSubScope(ast::Statement *_stmt) {
    if (_stmt == nullptr) {
        return nullptr;
    }
    return  std::get_if<ast::SubScope>(_stmt);
}

ast::ReturnStatement * GetReturnStatement(ast::Statement *_stmt) {
    if (_stmt == nullptr) {
        return nullptr;
    }
    return std::get_if<ast::ReturnStatement>(_stmt);
}

ast::AssignStatement * GetAssignStatement(ast::Statement *_stmt) {
    if (_stmt == nullptr) {
        return nullptr;
    }
    return std::get_if<ast::AssignStatement>(_stmt);
}

ast::VariableStatement * GetVariableStatement(ast::Statement *_stmt) {
    if (_stmt == nullptr) {
        return nullptr;
    }
    return std::get_if<ast::VariableStatement>(_stmt);
}

ast::FunctionCall * GetFunctionCall(ast::Statement * _stmt) {
    if (_stmt == nullptr) {
        return nullptr;
    }
    return std::get_if<ast::FunctionCall>(_stmt);
}

ast::ContinueStatement * GetContinueStatement(ast::Statement * _stmt) {
    if (_stmt == nullptr) {
        return nullptr;
    }
    return std::get_if<ast::ContinueStatement>(_stmt);
}

ast::BreakStatement * GetBreakStatement(ast::Statement * _stmt) {
    if (_stmt == nullptr) {
        return nullptr;
    }
    return std::get_if<ast::BreakStatement>(_stmt);
}
