


//
// Created by Administrator on 2026/2/20.
//

export module Token;

import std;

using size_t = std::size_t;
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

        const KeyWords Types = {"struct", "enum", "sizeof", "union"};
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
        explicit PointerType(const std::string_view _name,const size_t _pointerLevel = 1)
            : Name(_name), PointerLevel(_pointerLevel) {
        }

        const std::string Name;

        void Finalize(std::weak_ptr<CompileType> _baseType) {
            BaseType = std::move(_baseType);
        }

        [[nodiscard]] std::weak_ptr<CompileType> GetBaseType() const {
            return BaseType;
        }

        const size_t PointerLevel;

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

    extern const std::unordered_map<std::string_view, BaseOperator> BaseOperators;

    class ConstValue;
    class Variable;
    class FunctionCall;
    class CompositeExpression;

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

    class Expression {
    public:
        using Data = std::variant<ConstValue, Variable, std::shared_ptr<FunctionCall>, std::shared_ptr<CompositeExpression>>;
        std::shared_ptr<Data> Storage;

        // --- 宽松且安全的构造函数 ---
        template<typename T>
        requires (
            // 1. 排除掉 Expression 本身及其子类，防止拦截拷贝/移动构造
            !std::is_same_v<std::remove_cvref_t<T>, Expression> &&
            // 2. 只有当 T 能被用来构造 Data 时，才启用这个模板
            std::is_constructible_v<Data, T>
        )
        explicit Expression(T&& _val)
            : Storage(std::make_shared<Data>(std::forward<T>(_val))) {}


        // 默认构造：初始化一个空的或者默认状态的 Data
        Expression() : Storage(std::make_shared<Data>(ConstValue("0"))) {}

        // 显式声明，去 .cpp 里 = default
        ~Expression();

        // 拷贝与移动保持 default
        Expression(const Expression&) = default;
        Expression(Expression&&) noexcept = default;
        Expression& operator=(const Expression&) = default;
        Expression& operator=(Expression&&) noexcept = default;
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
        explicit VariableStatement(const std::string_view _name, const std::shared_ptr<Type::CompileType>& _varType,
                                   std::shared_ptr<Expression> _initializer = nullptr)
            : Name(_name), VarType(_varType),Initializer(std::move(_initializer)) {
        }

        const std::string Name;
        const std::shared_ptr<Type::CompileType> VarType;
        std::shared_ptr<Expression> Initializer;
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
