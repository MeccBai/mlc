
//
// Created by Administrator on 2026/2/20.
//

export module Token;

import std;
export namespace mlc::ast {
    enum class ScopeType { Global, Function, Condition, Loop, Anonymous, Struct };

    enum class StatementType {
        Declaration,
        Assignment,
        Calculation,
        Definition,
        FunctionCall,
        SubScope,
    };

    enum class GlobalStateType {
        FunctionDefinition,
        FunctionDeclaration,
        StructDefinition,
        Typedef,
        EnumDefinition,
        VariableDeclaration,
    };

    enum class SubScopeType {
        FunctionBody,
        IfBlock,
        ElseBlock,
        ForBlock,
        WhileBlock,
        DoWhileBlock,
        SwitchBlock,
        CaseBlock,
        DefaultBlock,
        AnonymousBlock,
    };

    namespace KeyWords {
        using KeyWords = std::unordered_set<std::string_view>;

        const KeyWords BaseType = {
            "int", "float", "double", "char", "void", "short", "long",
        };

        const KeyWords StorageClass = {"static", "extern"};

        const KeyWords ControlFlow = {
            "if", "else", "for", "while", "do", "switch", "case", "default",
        };

        const KeyWords Jump = {"return", "break", "continue"};

        const KeyWords Types = {"struct", "typedef", "enum", "sizeof", "union"};
    } // namespace KeyWords
} // namespace mlc::ast

export namespace mlc::ast::Type {
    class BaseType;
    class StructDefinition;
    class EnumDefinition;
    class PointerType;
    class ArrayType;

    using CompileType = std::variant<BaseType, StructDefinition, EnumDefinition, PointerType, ArrayType>;

    class BaseType {
    public:
        explicit BaseType(const std::string_view _name, std::size_t _size) : Name(_name), Size(_size) {
        }

        const std::string Name;
        const std::size_t Size;

        bool operator ==(const BaseType &_other) const {
            return _other.Name == Name;
        }

        bool operator >(const BaseType &_other) const {
            return Size > _other.Size;
        }

        bool operator <(const BaseType &_other) const {
            return Size < _other.Size;
        }
    };

    extern const std::vector<BaseType> BaseTypes;

    class EnumDefinition {
    public:
        explicit EnumDefinition(const std::string_view _name, std::vector<std::string> &_values) : Name(_name),
            Values(std::move(_values)) {
        }

        const std::string Name;
        const std::vector<std::string> Values;
    };

    struct StructMember {
        std::weak_ptr<CompileType> Type;
        std::string Name;
    };

    class StructDefinition {
    public:
        explicit StructDefinition(const std::string_view _name, std::vector<StructMember> &_members)
            : Name(_name), Members(std::move(_members)) {
        }

        const std::string Name;
        const std::vector<StructMember> Members; // type and name
    };

    class ArrayType {
    public:
        explicit ArrayType(const std::string_view _name, std::weak_ptr<CompileType> _baseType, std::size_t _size)
            : Name(_name), BaseType(std::move(_baseType)), Size(_size) {
        }

        const std::string Name;
        const std::weak_ptr<CompileType> BaseType;
        const std::size_t Size; // 0 for incomplete array
    };

    class PointerType {
    public:
        explicit PointerType(const std::string_view _name)
            : Name(_name) {
        }

        const std::string Name;

        void Finalize(std::weak_ptr<CompileType> _baseType) {
            BaseType = std::move(_baseType);
        }

        [[nodiscard]] std::weak_ptr<CompileType> GetBaseType() const {
            return BaseType;
        }

    private:
        std::weak_ptr<CompileType> BaseType;
    };
} // namespace mlc::ast::Type

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
        ShiftLeft,
        ShiftRight,

        // --- 成员访问 (Access) ---
        Dot, // .
        Arrow, // ->
        Subscript, // [ ]

        // --- 指针操作 (Pointer) ---
        Dereference, // *
        AddressOf, // &

        // --- 其他 (Other) ---
        SizeOf, // sizeof
    };

    class ConstValue;
    class Variable;
    class FunctionCall;
    class CompositeExpression;

    class Expression {
    public:
        using Data = std::variant<ConstValue, Variable, std::unique_ptr<FunctionCall>, std::unique_ptr<CompositeExpression>>;
        std::shared_ptr<Data> Storage;

        template<typename T>
        explicit Expression(T&& _val) : Storage(std::make_shared<Data>(std::forward<T>(_val))) {}
        // 1. 【核心：找回拷贝能力】
        Expression(const Expression&) = default;
        Expression& operator=(const Expression&) = default;

        // 2. 保留你的移动能力
        Expression(Expression&&) noexcept = default;
        Expression& operator=(Expression&&) noexcept = default;

        Expression() = default; // 建议补一个默认构造
        ~Expression();
    };

    class ConstValue {
    public:
        explicit ConstValue(const std::string_view _value) : Value(_value) {
        }

        const std::string Value;
    };

    class Variable {
    public:
        explicit Variable(const std::string_view _name, std::weak_ptr<Type::CompileType> _varType) : Name(_name),
            VarType(std::move(_varType)) {
        }

        const std::string Name;
        const std::weak_ptr<Type::CompileType> VarType;
    };

    class FunctionCall {
    public:
        explicit FunctionCall(const std::string_view _functionName, std::vector<Expression> &_arguments)
            : FunctionName(_functionName), Arguments(std::move(_arguments)) {
        }

        const std::string FunctionName;
        const std::vector<Expression> Arguments;
    };

    class CompositeExpression {
    public:
        explicit CompositeExpression(std::vector<Expression> _components,
                                     std::vector<BaseOperator> _operators) : Operators(std::move(_operators)),
                                                                             Components(std::move(_components)) {
        }

        const std::vector<BaseOperator> Operators;
        const std::vector<Expression> Components;
    };
}

export namespace mlc::ast {
    class SubScope;
    class AssignStatement;
    class VariableStatement;
    class ReturnStatement;
    class SwitchCaseScope;
    class CaseBlock;
    using FunctionCallStatement = FunctionCall;

    using Statement = std::variant<VariableStatement, AssignStatement, FunctionCallStatement, ReturnStatement, SubScope>
    ;

    class AssignStatement {
    public:
        explicit AssignStatement(Expression _baseValue, Expression _value) : BaseValue(std::move(_baseValue)),
                                                                             Value(std::move(_value)) {
        }

        const Expression BaseValue;
        const Expression Value;
    };

    class VariableStatement {
    public:
        explicit VariableStatement(const std::string_view _name, std::weak_ptr<Type::CompileType> _varType,
                                   std::optional<Expression> _initializer = std::nullopt)
            : Name(_name), VarType(std::move(_varType)), Initializer(std::move(_initializer)) {
        }

        std::string Name;
        std::weak_ptr<Type::CompileType> VarType;
        std::optional<Expression> Initializer;
    };

    class ReturnStatement {
    public:
        explicit ReturnStatement(std::optional<Expression> _returnValue = std::nullopt) : ReturnValue(
            std::move(_returnValue)) {
        }

        const std::optional<Expression> ReturnValue;
    };

    class SubScope {
    public:
        explicit SubScope(std::vector<Statement> _statements, const SubScopeType _type,
                          std::vector<Expression> _condition) : Statements(std::move(
                                                                    _statements)), Conditions(std::move(_condition)),
                                                                ScopeType(_type) {
        }

        const std::vector<Statement> Statements;
        const std::vector<Expression> Conditions;
        const SubScopeType ScopeType = SubScopeType::AnonymousBlock;
    };

    class FunctionDeclaration {
    public:
        using Args = std::vector<VariableStatement>;

         explicit FunctionDeclaration(std::string _name,const std::weak_ptr<Type::CompileType>&  _returnType,
                                    Args  _args, const bool _isVarList = false) : IsVarList(_isVarList),
            Name(std::move(_name)),
            Parameters(std::move(_args)), ReturnType(_returnType) {
        }

        const bool IsVarList;
        const std::string Name;
        const Args Parameters;
        const std::weak_ptr<Type::CompileType> ReturnType;
    };

    class FunctionScope {
    public:
        using Args = std::vector<VariableStatement>;

        FunctionScope(std::string _name, std::vector<Statement> _statements,
                               std::weak_ptr<Type::CompileType> _returnType,
                               Args _args, const bool _isVarList = false) : IsVarList(_isVarList),
                                                                            Name(std::move(_name)),
                                                                            Statements(std::move(_statements)),
                                                                            Parameters(std::move(_args)),
                                                                            ReturnType(std::move(_returnType)) {
        }

        const bool IsVarList;

        const std::string Name;
        const std::vector<Statement> Statements;
        const Args Parameters;
        const std::weak_ptr<Type::CompileType> ReturnType;

        [[nodiscard]] FunctionDeclaration ToDeclaration() const;

    };
} // namespace mlc::ast


export namespace mlc::ast {
    using GlobalStatement = std::variant<Type::StructDefinition, Type::EnumDefinition, FunctionScope, VariableStatement>
    ;
}
