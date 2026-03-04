//
// Created by Administrator on 2026/3/4.
//

export module Token:Factory;

import :Type;
import :Expression;
import :Statement;

export namespace mlc::ast {

    template<typename _type>
    concept IsTokenType = requires
    {
      requires (
        std::is_same_v<std::remove_cvref_t<_type>, ConstValue> ||
        std::is_same_v<std::remove_cvref_t<_type>, EnumValue> ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<Variable> > ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<FunctionCall> > ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<CompositeExpression> > ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<MemberAccess> > ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<InitializerList> > ||
        std::is_same_v<std::remove_cvref_t<_type>, Expression> ||
        std::is_same_v<std::remove_cvref_t<_type>, VariableStatement> ||
        std::is_same_v<std::remove_cvref_t<_type>, MemberAccess> ||
        std::is_same_v<std::remove_cvref_t<_type>, FunctionCall> ||
        std::is_same_v<std::remove_cvref_t<_type>, FunctionDeclaration> ||
        std::is_same_v<std::remove_cvref_t<_type>, FunctionScope> ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::StructDefinition> ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::EnumDefinition> ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::ArrayType> ||
        std::is_same_v<std::remove_cvref_t<_type>, Type::CompileType>||
        std::is_same_v<std::remove_cvref_t<_type>, Statement> ||
        std::is_same_v<std::remove_cvref_t<_type>, SubScope>
    );
    };

    template<IsTokenType _type>
    Type::sPtr<_type> Make(_type &&_value) {
        return std::make_shared<_type>(std::forward<_type>(_value));
    }
    template<IsTokenType _type>
    Type::sPtr<_type> Make(_type &_value) {
        return std::make_shared<_type>(_value);
    }

    Type::sPtr<VariableStatement> MakeVariable(VariableStatement &&_variable) {
        return std::make_shared<VariableStatement>(std::move(_variable));
    }

    Type::sPtr<VariableStatement> MakeVariable(VariableStatement &_variable) {
        return std::make_shared<VariableStatement>(_variable);
    }

    Type::sPtr<MemberAccess> MakeMemberAccess(MemberAccess &&_memberAccess) {
        return std::make_shared<MemberAccess>(std::move(_memberAccess));
    }

    Type::sPtr<MemberAccess> MakeMemberAccess(MemberAccess &_memberAccess) {
        return std::make_shared<MemberAccess>(_memberAccess);
    }

    Type::sPtr<FunctionCall> MakeFunctionCall(FunctionCall &&_functionCall) {
        return std::make_shared<FunctionCall>(std::move(_functionCall));
    }

    Type::sPtr<FunctionCall> MakeFunctionCall(FunctionCall &_functionCall) {
        return std::make_shared<FunctionCall>(_functionCall);
    }

    Type::sPtr<FunctionDeclaration> MakeFuncDecl(FunctionDeclaration &&_functionDecl) {
        return std::make_shared<FunctionDeclaration>(std::move(_functionDecl));
    }

    Type::sPtr<FunctionDeclaration> MakeFuncDecl(FunctionDeclaration &_functionDecl) {
        return std::make_shared<FunctionDeclaration>(_functionDecl);
    }

    Type::sPtr<FunctionScope> MakeFuncScope(FunctionScope &&_functionScope) {
        return std::make_shared<FunctionScope>(std::move(_functionScope));
    }

    Type::sPtr<FunctionScope> MakeFuncScope(FunctionScope &_functionScope) {
        return std::make_shared<FunctionScope>(_functionScope);
    }

    Type::sPtr<Type::StructDefinition> MakeStructDef(Type::StructDefinition &&_structDef) {
        return std::make_shared<Type::StructDefinition>(std::move(_structDef));
    }

    Type::sPtr<Type::StructDefinition> MakeStructDef(Type::StructDefinition &_structDef) {
        return std::make_shared<Type::StructDefinition>(_structDef);
    }

    Type::sPtr<Type::CompileType> MakeCompileType(Type::CompileType &&_compileType) {
        return std::make_shared<Type::CompileType>(std::move(_compileType));
    }

    Type::sPtr<Type::CompileType> MakeCompileType(Type::CompileType &_compileType) {
        return std::make_shared<Type::CompileType>(_compileType);
    }

    Type::sPtr<Type::EnumDefinition> MakeEnumDef(Type::EnumDefinition &&_enumDef) {
        return std::make_shared<Type::EnumDefinition>(std::move(_enumDef));
    }

    Type::sPtr<Type::EnumDefinition> MakeEnumDef(Type::EnumDefinition &_enumDef) {
        return std::make_shared<Type::EnumDefinition>(_enumDef);
    }

    Type::sPtr<Type::ArrayType> MakeArrayType(Type::ArrayType &&_arrayType) {
        return std::make_shared<Type::ArrayType>(std::move(_arrayType));
    }

    Type::sPtr<Type::ArrayType> MakeArrayType(Type::ArrayType &_arrayType) {
        return std::make_shared<Type::ArrayType>(_arrayType);
    }



}
