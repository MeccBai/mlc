//
// Created by Administrator on 2026/2/27.
//

export module Token:Expression;

import :Type;

import std;
using size_t = std::size_t;

export namespace mlc::ast {
    enum class BaseOperator {
        // --- 算术运算 (Arithmetic) ---
        Add,
        Sub,
        Mul,
        Div,
        Mod,

        // --- 关系运算 (Relational) ---
        Equal,
        NotEqual,
        Greater,
        Less,
        GreaterEqual,
        LessEqual,

        // --- 逻辑运算 (Logical) ---
        And,
        Or,
        Not,

        // --- 位运算 (Bitwise) ---
        BitAnd,
        BitOr,
        BitXor,
        BitNot,
        ShiftLeft,
        ShiftRight,

        // --- 成员访问 (Access) ---
        Dot, // .
        Arrow, // ->
        Subscript, // [ ]

        // --- 指针操作 (Pointer) ---
        Dereference, // *
        AddressOf, // &
    };

    class SubScope;
    class AssignStatement;
    class VariableStatement;
    class ReturnStatement;
    class SwitchCaseScope;

    class BreakStatement {
    };

    class ContinueStatement {
    };


    class FunctionCall;
    class CompositeExpression;
    class FunctionDeclaration;
    using Variable = VariableStatement;
    class InitializerList;

    class MemberAccess {
    public:
        MemberAccess(std::shared_ptr<Type::StructDefinition> _structDef, const size_t _index) : StructDef(
                std::move(_structDef)), Index(_index), Name(StructDef->Members[Index].Name) {
        }

        std::shared_ptr<Type::StructDefinition> StructDef;
        const size_t Index;
        const std::string Name;
    };

    class ConstValue {
    public:
        explicit ConstValue(const std::string_view _value,
                            const bool _isChar = false) : Value(_value), IsChar(_isChar) {
        }


        const std::string Value;
        const bool IsChar;

        [[nodiscard]] std::shared_ptr<Type::CompileType> GetType() const;
    };

    class EnumValue {
    public:
        explicit EnumValue(const std::shared_ptr<Type::EnumDefinition> &_enumDef, const size_t _index)
            : Index(_index), EnumDef(_enumDef) {
        }

        const size_t Index;
        const std::shared_ptr<Type::EnumDefinition> EnumDef;

        [[nodiscard]] std::shared_ptr<Type::CompileType> GetType() const {
            return std::make_shared<Type::CompileType>(*EnumDef);
        }
    };

    class Expression {
    public:
        using Data = std::variant<ConstValue, EnumValue, Type::sPtr<Variable>, Type::sPtr<FunctionCall>,
            Type::sPtr<CompositeExpression>, Type::sPtr<MemberAccess>, Type::sPtr<InitializerList> >;
        Type::sPtr<Data> Storage;

        template<typename T>
            requires (
                !std::is_same_v<std::remove_cvref_t<T>, Expression> &&
                std::is_constructible_v<Data, T>
            )
        explicit Expression(T &&_val)
            : Storage(std::make_shared<Data>(std::forward<T>(_val))) {
        }

        Expression() : Storage(std::make_shared<Data>(ConstValue("0"))) {
        }

        ~Expression();

        Expression(const Expression &) = default;

        Expression(Expression &&) noexcept = default;

        Expression &operator=(const Expression &) = default;

        Expression &operator=(Expression &&) noexcept = default;

        [[nodiscard]] std::shared_ptr<Type::CompileType> GetType() const;
    };

    class InitializerList {
    public:
        const std::string Name = "InitializerList";
        std::vector<std::shared_ptr<Expression> > Values;

        explicit InitializerList(std::vector<std::shared_ptr<Expression> > _values) : Values(std::move(_values)) {
        }
    };

    using FunctionCallStatement = FunctionCall;

    class FunctionCall {
    public:
        explicit FunctionCall(const std::shared_ptr<FunctionDeclaration> &_functionDecl,
                              std::vector<std::shared_ptr<Expression> > &_arguments)
            : FunctionDecl(_functionDecl), Arguments(std::move(_arguments)) {
        }

        const std::shared_ptr<FunctionDeclaration> FunctionDecl;
        const std::vector<std::shared_ptr<Expression> > Arguments;
    };

    class CompositeExpression {
    public:
        explicit CompositeExpression(std::vector<std::shared_ptr<Expression> > _components,
                                     std::vector<BaseOperator> _operators,
                                     const bool _isOperatorFirst = false) : Operators(std::move(_operators)),
                                                                            Components(std::move(_components)),
                                                                            isOperatorFirst(_isOperatorFirst) {
        }

        const std::vector<BaseOperator> Operators;
        const std::vector<std::shared_ptr<Expression> > Components;
        const bool isOperatorFirst;

        [[nodiscard]] std::shared_ptr<Type::CompileType> GetResultType() const {
            if (Components.empty()) {
                return nullptr;
            }
            return Components[0]->GetType();
        }
    };
}
