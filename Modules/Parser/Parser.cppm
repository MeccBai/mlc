//
// Created by Administrator on 2026/2/20.
//

export module Parser;

import Token;
import std;
import Compiler;

export namespace mlc::parser {
    class AbstractSyntaxTree {
    public:
        template<typename _type>
        using SymbolTable = std::vector<std::shared_ptr<_type>>;

        template<typename _type>
        using ContextTable = std::vector<std::weak_ptr<_type>>;

        struct functionWarper {
            ast::FunctionDeclaration decl;
            std::string_view body;
        };

        ast::FunctionScope functionDefParser(std::string_view _functionContent);
        [[nodiscard]] ast::FunctionDeclaration functionDeclParser(std::string_view _functionContent) const;
        [[nodiscard]] functionWarper functionDeclSpliter(std::string_view _functionHeader) const;

        struct exprTree;
        using FragmentData = std::variant<std::string_view, std::vector<exprTree>>;

        struct exprTree {
            FragmentData data; // 核心内容
            bool isOperator; // 是否是操作符（对于嵌套集合，通常设为 false）

            explicit exprTree(std::string_view s, const bool op) : data(s), isOperator(op) {}

            explicit exprTree(std::vector<exprTree> v) : data(std::move(v)), isOperator(false) {}
        };

        struct caseBlock {
            ast::Expression condition; // case 条件表达式
            std::vector<ast::Statement> statements; // case 块内的语句
        };

        ast::Statement statementParser(std::string_view statementContent) {
            ContextTable<ast::VariableStatement> dummyContext;
            return statementParser(dummyContext, statementContent);
        }

        ast::Statement statementParser(ContextTable<ast::VariableStatement> &_context,std::string_view statementContent);

        caseBlock caseBlockParser(ContextTable<ast::VariableStatement> &_context,std::string_view statementContent);

        ast::Expression expressionParser(ContextTable<ast::VariableStatement> &_context,
                                         std::string_view _expressionContent);

        ast::Expression constExpressionParser(std::string_view _constExpressionContent);

        std::vector<ast::VariableStatement> globalVariableParser(std::string_view variableContent);

        std::vector<ast::VariableStatement> localVariableParser(ContextTable<ast::VariableStatement> &_context,std::string_view variableContent);

        std::vector<ast::VariableStatement> variableParser(ContextTable<ast::VariableStatement> &_context,std::string_view variableContent);

        ast::SubScope subScopeParser(ContextTable<ast::VariableStatement> &_context,std::string_view _subScopeContent);
        ast::SubScope subScopeParser(const std::string_view _subScopeContent) {
            ContextTable<ast::VariableStatement> dummyContext;
            return subScopeParser(dummyContext, _subScopeContent);
        }

        ast::Expression expressionTreeParser(ContextTable<ast::VariableStatement> &_context,
                                             exprTree _expressionContent);

        ast::Expression expressionParser(const std::string_view _expressionContent) {
            ContextTable<ast::VariableStatement> dummyContext;
            return expressionParser(dummyContext, _expressionContent);
        }

        [[nodiscard]] std::vector<ast::Type::StructDefinition> structDefParser(const std::vector<std::string_view> &_structContents) const;

        static ast::Type::EnumDefinition enumDefParser(std::string_view _enumContent);

        std::unordered_map<std::string, std::shared_ptr<ast::Type::CompileType>> typeMap;

        SymbolTable<ast::Type::CompileType> typeSymbolTable;
        SymbolTable<ast::VariableStatement> variableSymbolTable;
        SymbolTable<ast::FunctionDeclaration> functionSymbolTable;
        SymbolTable<ast::FunctionScope> functionScopeTable;

        std::vector<ast::GlobalStatement> tree;

        explicit AbstractSyntaxTree(const std::vector<seg::TokenStatement> &tokens);

    private:
        [[nodiscard]] std::weak_ptr<ast::Type::CompileType> findType(std::string_view _typeName) const;
    };
} // namespace mlc::parser

std::vector<std::string_view> split(std::string_view str, std::string_view delimiter);