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

    enum class GlobalStatement {
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
        std::string name;
        unsigned int size;
    };

    class EnumDefinition {
        std::string name;
        std::vector<std::string> values;
    };

    class StructDefinition {
        std::string name;
        std::vector<CompileType> members; // type and name
    };

    class ArrayType {
        std::string name;
        std::weak_ptr<CompileType> baseType;
        std::size_t size; // 0 for incomplete array
    };

    class PointerType {
        std::string name;
        std::weak_ptr<CompileType> baseType;
    };
} // namespace mlc::ast::Type


export namespace mlc::ast {

    class SubScope;
    class AssignStatement;
    class VariableStatement;
    class FunctionCallStatement;

    using Expression = std::unique_ptr<std::variant<VariableStatement, AssignStatement, FunctionCallStatement>>;

    using Statement = std::variant<VariableStatement, AssignStatement, FunctionCallStatement, SubScope>;

    class AssignStatement {
        std::string variableName;
        Expression value;
    };

    class VariableStatement {
        std::string name;
        Type::CompileType varType;
        std::optional<Expression> initializer;
    };

    class FunctionCallStatement {
        std::string functionName;
        std::vector<Expression> arguments;
    };

    class SubScope {
        std::vector<Statement> statements;
        SubScopeType scopeType;
    };

    class FunctionScope {
        std::vector<Statement> statements;
        Type::CompileType returnType;
    };

} // namespace mlc::ast
