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
    Type::sPtr<std::remove_cvref_t<_type>> Make(_type &&_value) {
        return std::make_shared<std::remove_cvref_t<_type>>(std::forward<_type>(_value));
    }

    template<IsTokenType _type>
    Type::sPtr<std::remove_cvref_t<_type>> Make(_type &_value) {
        return std::make_shared<std::remove_cvref_t<_type>>(std::forward<_type>(_value));
    }


}
