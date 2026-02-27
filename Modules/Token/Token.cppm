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
        explicit BaseType(const std::string_view _name, const std::size_t _size) : Name(_name), size(_size) {
        }
        const std::string Name;
        std::size_t Size() const {return size;}
        bool operator ==(const BaseType &_other) const {
            return _other.Name == Name;
        }
        bool operator >(const BaseType &_other) const {
            return size > _other.Size();
        }
        bool operator <(const BaseType &_other) const {
            return size < _other.Size();
        }
    private:
        const std::size_t size;
    };

    extern const std::vector<BaseType> BaseTypes;

    class EnumDefinition {
    public:
        explicit EnumDefinition(const std::string_view _name, std::vector<std::string> &_values) : Name(_name),
            Values(std::move(_values)) {
        }

        const std::string Name;
        const std::vector<std::string> Values;

        static std::size_t Size() {
            return 4;
        }

    };

    struct StructMember {
        std::string Name;
        std::shared_ptr<CompileType> Type;
    };

    class StructDefinition {
    public:
        explicit StructDefinition(const std::string_view _name, std::vector<StructMember> &_members,const bool _isExported = false)
            : Name(_name), Members(std::move(_members)), IsExported(_isExported) {
        }

        const std::string Name;
        const std::vector<StructMember> Members; // type and name
        const bool IsExported ;

        [[nodiscard]] size_t Size() const;
    };

    class ArrayType {
    public:
        [[nodiscard]] std::string GetTypeName() const;

        explicit ArrayType(std::shared_ptr<CompileType> _baseType, std::size_t _length) : BaseType(std::move(_baseType)),
            Length(_length), Name(this->GetTypeName()) {
        }

        [[nodiscard]] std::weak_ptr<CompileType> GetBaseType() const {
            return BaseType;
        }

        const std::shared_ptr<CompileType> BaseType;
        const std::size_t Length; // 0 for incomplete array
        const std::string Name;

        [[nodiscard]] size_t Size() const;
    };

    class PointerType {
    public:
        explicit PointerType(std::shared_ptr<CompileType> _baseType, const size_t _pointerLevel = 1)
            : PointerLevel(_pointerLevel), BaseType(std::move(_baseType)) {
        }

        explicit PointerType(const size_t _pointerLevel = 1) : PointerLevel(_pointerLevel) {
        }

        const std::string Name;

        void Finalize(std::shared_ptr<CompileType> _baseType) {
            BaseType = std::move(_baseType);
            const std::string baseName = std::visit([](auto &&t) -> std::string {
                return std::string(t.Name);
            }, *BaseType);
            const std::string suffix(PointerLevel, '$');
            const_cast<std::string &>(Name) = baseName + suffix;
        }

        [[nodiscard]] std::string GetTypeName() const {
            return Name;
        }

        [[nodiscard]] std::weak_ptr<CompileType> GetBaseType() const {
            return BaseType;
        }

        const size_t PointerLevel;

        std::shared_ptr<CompileType> BaseType;

        static std::size_t Size() {
            return 8;
        }
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

    class SubScope;
    class AssignStatement;
    class VariableStatement;
    class ReturnStatement;
    class SwitchCaseScope;

    class BreakStatement {
    };

    class ContinueStatement {
    };


    extern const std::unordered_map<std::string_view, BaseOperator> BaseOperators;

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
        explicit ConstValue(const std::string_view _value) : Value(_value) {
        }

        const std::string Value;

        [[nodiscard]] std::shared_ptr<Type::CompileType> GetType() const;
    };

    class Expression {
    public:
        using Data = std::variant<ConstValue, std::shared_ptr<Variable>, std::shared_ptr<FunctionCall>, std::shared_ptr<
            CompositeExpression>, std::shared_ptr<MemberAccess>, std::shared_ptr<InitializerList> >;
        std::shared_ptr<Data> Storage;

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

export namespace mlc::ast {
    using Statement = std::variant<
        VariableStatement, AssignStatement, FunctionCallStatement,
        ReturnStatement, SubScope, ContinueStatement, BreakStatement>;

    class AssignStatement {
    public:
        explicit AssignStatement(std::shared_ptr<Expression> _baseValue,
                                 std::shared_ptr<Expression> _value) : BaseValue(std::move(_baseValue)),
                                                                       Value(std::move(_value)) {
        }

        const std::shared_ptr<Expression> BaseValue;
        const std::shared_ptr<Expression> Value;
    };

    class VariableStatement {
    public:
        void InitListValidCheck() const;

        explicit VariableStatement(const std::string_view _name, const std::shared_ptr<Type::CompileType> &_varType,
                                   std::shared_ptr<Expression> _initializer = nullptr)
            : Name(_name), VarType(_varType), Initializer(std::move(_initializer)) {
            InitListValidCheck();
        }

        const std::string Name;
        const std::shared_ptr<Type::CompileType> VarType;
        std::shared_ptr<Expression> Initializer;
    };

    class ReturnStatement {
    public:
        explicit ReturnStatement(std::shared_ptr<Expression> _returnValue = nullptr) : ReturnValue(
            std::move(_returnValue)) {
        }

        const std::shared_ptr<Expression> ReturnValue;
    };

    class SubScope {
    public:
        explicit SubScope(std::vector<std::shared_ptr<Statement> > _statements, const SubScopeType _type,
                          std::shared_ptr<Expression> _condition) : Statements(std::move(
                                                                        _statements)), Condition(std::move(_condition)),
                                                                    ScopeType(_type) {
        }

        explicit SubScope(std::vector<std::shared_ptr<Statement> > _statements, const SubScopeType _type = SubScopeType::AnonymousBlock) : Statements(std::move(
                                                                        _statements)), ScopeType(_type) {
        }

        const std::vector<std::shared_ptr<Statement> > Statements;
        const std::shared_ptr<Expression> Condition;
        const SubScopeType ScopeType;
    };

    class FunctionDeclaration {
    public:
        using Args = std::vector<std::shared_ptr<VariableStatement> >;

        explicit FunctionDeclaration(std::string _name, const std::shared_ptr<Type::CompileType> &_returnType,
                                     Args _args, const bool _isVarList = false,
                                     const bool _isExported = false) : IsVarList(_isVarList),
                                                                       Name(std::move(_name)),
                                                                       Parameters(std::move(_args)),
                                                                       ReturnType(_returnType),
                                                                       IsExported(_isExported) {
        }

        const bool IsVarList;
        const std::string Name;
        const Args Parameters;
        const std::shared_ptr<Type::CompileType> ReturnType;
        const bool IsExported ;
    };

    class FunctionScope {
    public:
        using Args = std::vector<std::shared_ptr<VariableStatement> >;

        FunctionScope(std::string _name, std::vector<std::shared_ptr<Statement> > _statements,
                      std::shared_ptr<Type::CompileType> _returnType,
                      Args _args, const bool _isVarList = false,const bool _isExported = false) : IsVarList(_isVarList),
                                                                   Name(std::move(_name)),
                                                                   Statements(std::move(_statements)),
                                                                   Parameters(std::move(_args)),
                                                                   ReturnType(std::move(_returnType)),IsExported(_isExported) {
        }

        FunctionScope(const FunctionDeclaration &_functionDeclaration,
                      std::vector<std::shared_ptr<Statement> > _statements,const bool _isExported = false)
            : IsVarList(_functionDeclaration.IsVarList),
              Name(_functionDeclaration.Name),
              Statements(std::move(_statements)),
              Parameters(_functionDeclaration.Parameters),
              ReturnType(_functionDeclaration.ReturnType),IsExported(_isExported) {
        }

        const bool IsVarList;
        const bool IsExported = false;
        const std::string Name;
        const std::vector<std::shared_ptr<Statement> > Statements;
        const Args Parameters;
        const std::shared_ptr<Type::CompileType> ReturnType;

        [[nodiscard]] FunctionDeclaration ToDeclaration() const;
    };
} // namespace mlc::ast


export namespace mlc::ast {
    using GlobalStatement = std::variant<Type::StructDefinition, Type::EnumDefinition, FunctionScope, VariableStatement>
    ;

    void ValidateType(const std::shared_ptr<ast::Type::CompileType> &targetType,
                      const std::shared_ptr<ast::Type::CompileType> &actualType,
                      std::string_view contextInfo);
}
