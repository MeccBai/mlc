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

        std::vector<ast::VariableStatement> variableParser(std::string_view variableContent);

        ast::FunctionScope functionDefParser(std::string_view _functionContent);
        ast::FunctionDeclaration functionDeclParser(std::string_view _functionContent);

        ast::SubScope subScopeParser(std::string_view subScopeContent);

        ast::Expression expressionParser(ContextTable<ast::VariableStatement>& _contextTable, std::string_view _expressionContent);
        ast::Expression expressionParser(std::string_view _expressionContent) {
            ContextTable<ast::VariableStatement> dummyContext;
            return expressionParser(dummyContext, _expressionContent);
        }

        std::vector<ast::Type::StructDefinition> structDefParser(const std::vector<std::string_view>& _structContents);

        ast::Type::EnumDefinition enumDefParser(std::string_view _enumContent);

        void typeDefParser(std::string_view _typedefContent);

        std::unordered_map<std::string, std::shared_ptr<ast::Type::CompileType>> typeMap;

        SymbolTable<ast::Type::CompileType> typeSymbolTable;
        SymbolTable<ast::VariableStatement> variableSymbolTable;
        SymbolTable<ast::FunctionDeclaration> functionSymbolTable;

        std::vector<ast::GlobalStatement> tree;

        explicit AbstractSyntaxTree(const std::vector<seg::TokenStatement> &tokens);
    };
}
