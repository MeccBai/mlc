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

    class BreakStatement {
    };

    class ContinueStatement {
    };

    const std::unordered_map<std::string_view, BaseOperator> BaseOperators = {
        {"+", BaseOperator::Add}, {"-", BaseOperator::Sub}, {"*", BaseOperator::Mul},
        {"/", BaseOperator::Div}, {"%", BaseOperator::Mod}, {"==", BaseOperator::Equal},
        {"!=", BaseOperator::NotEqual}, {">", BaseOperator::Greater}, {"<", BaseOperator::Less},
        {">=", BaseOperator::GreaterEqual}, {"<=", BaseOperator::LessEqual}, {"&&", BaseOperator::And},
        {"||", BaseOperator::Or}, {"!", BaseOperator::Not}, {"&", BaseOperator::BitAnd},
        {"|", BaseOperator::BitOr}, {"^", BaseOperator::BitXor}, {"~", BaseOperator::BitNot},
        {">>", BaseOperator::ShiftRight}, {"<<", BaseOperator::ShiftLeft}, {".", BaseOperator::Dot},
        {"->", BaseOperator::Arrow}, {"[]", BaseOperator::Subscript}, {"@", BaseOperator::AddressOf},
        {"$", BaseOperator::Dereference},
    };

    class FunctionCall;
    class CompositeExpression;
    class FunctionDeclaration;
    using Variable = VariableStatement;
    class InitializerList;

    class MemberAccess {
    public:
        MemberAccess(Type::sPtr<Type::StructDefinition> _structDef, const size_t _index) : StructDef(
            std::move(_structDef)), Index(_index), Name(StructDef->Members[Index].Name) {
        }

        const Type::sPtr<Type::StructDefinition> StructDef;
        const size_t Index;
        const std::string Name;

        [[nodiscard]] std::shared_ptr<Type::CompileType> GetType() const {
            if (StructDef && Index < StructDef->Members.size()) {
                return StructDef->Members[Index].Type;
            }
            return nullptr;
        }
    };

    class ConstValue {
    public:
        explicit ConstValue(std::string_view _value, bool _isChar = false);
        const std::string Value;
        const bool IsChar;
        Type::sPtr<Type::CompileType> Type;
        [[nodiscard]] std::shared_ptr<Type::CompileType> GetType() const;
    private:
        static std::string processLiteral(std::string_view raw, bool isChar) {
            if (!isChar) return std::string(raw);
            if (raw.size() >= 2 && raw[0] == '\\') {
                switch (raw[1]) {
                    case 'n': return "10";   // 换行
                    case 't': return "9";    // 制表
                    case 'r': return "13";   // 回车
                    case '\\': return "92";  // 反斜杠
                    case '0': return "0";    // 空字符
                    default: return std::format("{}", static_cast<int>(raw[1]));
                }
            }
            return std::format("{}", static_cast<int>(raw[0]));
        }
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

        [[nodiscard]] ConstValue *GetConstValue() const {
            if (!Storage) return nullptr;
            return std::get_if<ConstValue>(&*Storage);
        }

        [[nodiscard]] EnumValue *GetEnumValue() const {
            if (!Storage) return nullptr;
            return std::get_if<EnumValue>(&*Storage);
        }

        [[nodiscard]] Type::sPtr<Variable> *GetVariable() const {
            if (!Storage) return nullptr;
            return std::get_if<Type::sPtr<Variable> >(&*Storage);
        }

        [[nodiscard]] Type::sPtr<FunctionCall> *GetFunctionCall() const {
            if (!Storage) return nullptr;
            return std::get_if<Type::sPtr<FunctionCall> >(&*Storage);
        }

        [[nodiscard]] Type::sPtr<CompositeExpression> *GetCompositeExpression() const {
            if (!Storage) return nullptr;
            return std::get_if<Type::sPtr<CompositeExpression> >(&*Storage);
        }

        [[nodiscard]] Type::sPtr<MemberAccess> *GetMemberAccess() const {
            if (!Storage) return nullptr;
            return std::get_if<Type::sPtr<MemberAccess> >(&*Storage);
        }

        [[nodiscard]] Type::sPtr<InitializerList> *GetInitializerList() const {
            if (!Storage) return nullptr;
            return std::get_if<Type::sPtr<InitializerList> >(&*Storage);
        }


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

            if (Components.size() == 1&& Operators[0] == BaseOperator::AddressOf || Operators[0] == BaseOperator::Dereference) {
                auto * isOpFirst = const_cast<bool*>(&isOperatorFirst);
                *isOpFirst = true;
            }
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

    const std::unordered_map<BaseOperator, int> OperatorPriority = {
        {BaseOperator::Dot, 1}, {BaseOperator::Arrow, 1}, {BaseOperator::Subscript, 1}, // 访问最强
        {BaseOperator::AddressOf, 2}, {BaseOperator::Dereference, 2}, // 指针紧随其后 (@, $)
        {BaseOperator::Not, 2}, {BaseOperator::BitNot, 2}, // 单目取反
        {BaseOperator::Mul, 3}, {BaseOperator::Div, 3}, {BaseOperator::Mod, 3}, // 乘除
        {BaseOperator::Add, 4}, {BaseOperator::Sub, 4}, // 加减
        {BaseOperator::ShiftLeft, 5}, {BaseOperator::ShiftRight, 5}, // 位移
        {BaseOperator::Greater, 6}, {BaseOperator::Less, 6}, // 比较
        {BaseOperator::GreaterEqual, 6}, {BaseOperator::LessEqual, 6},
        {BaseOperator::Equal, 7}, {BaseOperator::NotEqual, 7}, // 相等判定
        {BaseOperator::BitAnd, 8},
        {BaseOperator::BitXor, 9},
        {BaseOperator::BitOr, 10},
        {BaseOperator::And, 11},
        {BaseOperator::Or, 12},
    };

    template<typename _type>
    concept isExpression = requires
    {
        requires (
            std::is_same_v<std::remove_cvref_t<_type>, ConstValue> ||
            std::is_same_v<std::remove_cvref_t<_type>, EnumValue> ||
            std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<Variable> > ||
            std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<FunctionCall> > ||
            std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<CompositeExpression> > ||
            std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<MemberAccess> > ||
            std::is_same_v<std::remove_cvref_t<_type>, Type::sPtr<InitializerList> > ||
            std::is_same_v<std::remove_cvref_t<_type>, Expression> // 允许 Expression 对象的拷贝/移动
        );
    };

    template<isExpression _type>
    [[nodiscard]] Type::sPtr<Expression> MakeExpression(_type &&_val) {
        return std::make_shared<Expression>(std::forward<_type>(_val));
    }

    Type::sPtr<CompositeExpression> MakeCompExpr(std::vector<Type::sPtr<Expression> > &&_components,
                                                 std::vector<BaseOperator> &&_operators,
                                                 const bool _isOperatorFirst = false) {
        return std::make_shared<CompositeExpression>(std::move(_components), std::move(_operators),
                                                     _isOperatorFirst);
    }

    Type::sPtr<CompositeExpression> MakeCompExpr(std::vector<Type::sPtr<Expression> > &_components,
                                                 std::vector<BaseOperator> &_operators,
                                                 const bool _isOperatorFirst = false) {
        return std::make_shared<CompositeExpression>(_components, _operators, _isOperatorFirst);
    }

    Type::sPtr<InitializerList> MakeInitializerList(std::vector<Type::sPtr<Expression> > &&_values) {
        return std::make_shared<InitializerList>(std::move(_values));
    }

    Type::sPtr<InitializerList> MakeInitializerList(std::vector<Type::sPtr<Expression> > &_values) {
        return std::make_shared<InitializerList>(_values);
    }

    bool ConstExpressionCheck(const std::shared_ptr<Expression> &_expr);
}
