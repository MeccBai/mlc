//
// Created by Administrator on 2026/2/27.
//

export module Token:Statement;
import :Type;
import :Expression;

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
        EnumDefinition,
        VariableDeclaration,
        ImportFile,
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
} // namespace mlc::ast

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

        explicit SubScope(std::vector<std::shared_ptr<Statement> > _statements,
                          const SubScopeType _type = SubScopeType::AnonymousBlock) : Statements(std::move(
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
                                     const bool _isExported = false,
                                     const bool _isTypeConvert = false) : IsVarList(_isVarList),
                                                                          IsTypeConvert(_isTypeConvert),
                                                                          Name(std::move(_name)),
                                                                          Parameters(std::move(_args)),
                                                                          ReturnType(_returnType),
                                                                          IsExported(_isExported) {
            if (!_isTypeConvert) {
                Type::IsValidName(Name);
                std::ranges::for_each(Parameters, [](const auto &param) {
                    Type::IsValidName(param->Name);
                });
            }
        }

        const bool IsVarList;
        const bool IsTypeConvert;
        const std::string Name;
        const Args Parameters;
        const std::shared_ptr<Type::CompileType> ReturnType;
        const bool IsExported;
    };

    class FunctionScope {
    public:
        using Args = std::vector<std::shared_ptr<VariableStatement> >;

        FunctionScope(std::string _name, std::vector<std::shared_ptr<Statement> > _statements,
                      std::shared_ptr<Type::CompileType> _returnType,
                      Args _args, const bool _isVarList = false,
                      const bool _isExported = false) : IsVarList(_isVarList),
                                                        IsExported(_isExported),
                                                        Name(std::move(_name)),
                                                        Statements(std::move(_statements)),
                                                        Parameters(std::move(_args)),
                                                        ReturnType(std::move(_returnType)) {
            Type::IsValidName(Name);
            std::ranges::for_each(Parameters, [](const auto &param) {
                Type::IsValidName(param->Name);
            });
        }

        FunctionScope(const FunctionDeclaration &_functionDeclaration,
                      std::vector<std::shared_ptr<Statement> > _statements, const bool _isExported = false)
            : IsVarList(_functionDeclaration.IsVarList),
              IsExported(_isExported),
              Name(_functionDeclaration.Name),
              Statements(std::move(_statements)),
              Parameters(_functionDeclaration.Parameters), ReturnType(_functionDeclaration.ReturnType) {
        }

        const bool IsVarList;
        const bool IsExported = false;
        const std::string Name;
        const std::vector<std::shared_ptr<Statement> > Statements;
        const Args Parameters;
        const std::shared_ptr<Type::CompileType> ReturnType;

        [[nodiscard]] Type::sPtr<FunctionDeclaration> ToDeclaration() const;
    };

    // 通用 Statement 萃取模板
    template<typename T>
    T* GetStatement(Statement *_stmt) {
        return _stmt ? std::get_if<T>(_stmt) : nullptr;
    }

    // 向后兼容别名
    inline SubScope* GeSubScope(Statement *_stmt) { return GetStatement<SubScope>(_stmt); }
    inline AssignStatement* GetAssignStatement(Statement *_stmt) { return GetStatement<AssignStatement>(_stmt); }
    inline VariableStatement* GetVariableStatement(Statement *_stmt) { return GetStatement<VariableStatement>(_stmt); }
    inline ReturnStatement* GetReturnStatement(Statement *_stmt) { return GetStatement<ReturnStatement>(_stmt); }
    inline FunctionCallStatement* GetFunctionCallStatement(Statement *_stmt) { return GetStatement<FunctionCallStatement>(_stmt); }
    inline ContinueStatement* GetContinueStatement(Statement *_stmt) { return GetStatement<ContinueStatement>(_stmt); }
    inline BreakStatement* GetBreakStatement(Statement *_stmt) { return GetStatement<BreakStatement>(_stmt); }

    using GlobalStatement = std::variant<Type::StructDefinition, Type::EnumDefinition, FunctionScope, VariableStatement>;
} // namespace mlc::ast
