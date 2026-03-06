//
// Created by Administrator on 2026/2/20.
//

export module Parser;

import Token;
import std;
import Compiler;

import :Expr;

namespace ast = mlc::ast;
namespace type = ast::Type;


export namespace mlc::parser {
    class AbstractSyntaxTree {
    public:
        template<typename _type>
        using SymbolTable = std::vector<std::shared_ptr<_type> >;

        template<typename _type>
        using ContextTable = std::vector<std::shared_ptr<_type> >;

        template<typename _type>
        using StatementTable = SymbolTable<_type>;

        template<typename _type>
        using sPtr = std::shared_ptr<_type>;

        template<typename _type>
        using sOptional = std::optional<std::shared_ptr<_type> >;

        struct functionWarper {
            ast::FunctionDeclaration decl;
            std::string_view body;
        };

        ast::FunctionScope functionDefParser(std::string_view _functionContent);

        [[nodiscard]] ast::FunctionDeclaration functionDeclParser(std::string_view _functionContent) const;

        [[nodiscard]] sPtr<ast::VariableStatement> functionArgParser(std::string_view _argContent) const;

        [[nodiscard]] functionWarper functionDeclSpliter(std::string_view _functionHeader) const;


        StatementTable<ast::Statement> statementParser(std::string_view statementContent) {
            ContextTable<ast::VariableStatement> dummyContext;
            return statementParser(dummyContext, statementContent);
        }

        StatementTable<ast::Statement> statementParser(ContextTable<ast::VariableStatement> &_context,
                                                       std::string_view statementContent);

        struct caseBlock {
            sPtr<ast::Expression> condition; // case 条件表达式
            SymbolTable<ast::Statement> statements; // case 块内的语句
        };

        sPtr<caseBlock> caseBlockParser(ContextTable<ast::VariableStatement> &_context,
                                        std::string_view statementContent);

        sPtr<ast::Statement> handleSwitchBlock(std::string_view _subScopeContent,
                                               const ContextTable<ast::VariableStatement> &_context);

        sPtr<ast::Statement> handleDoWhileBlock(std::string_view _subScopeContent,
                                                const ContextTable<ast::VariableStatement> &_context,
                                                auto &_bodyToStatements);

        sPtr<ast::Expression> expressionParser(ContextTable<ast::VariableStatement> &_context,
                                               std::string_view _expressionContent);

        sPtr<ast::Expression> constExpressionParser(std::string_view _constExpressionContent);

        StatementTable<ast::Statement> globalVariableParser(std::string_view _variableContent);

        StatementTable<ast::Statement> localVariableParser(ContextTable<ast::VariableStatement> &_context,
                                                           std::string_view _variableContent);

        StatementTable<ast::Statement> variableParser(ContextTable<ast::VariableStatement> &_context,
                                                      std::string_view _variableContent);

        sPtr<ast::Statement> subScopeParser(const ContextTable<ast::VariableStatement> &_context,
                                            std::string_view _subScopeContent);

        sPtr<ast::Expression> handleSubscriptAccess(ContextTable<ast::VariableStatement> &_context,
                                                    const std::vector<ast::exprTree> &fragments,
                                                    int splitIndex);

        sPtr<ast::Statement> subScopeParser(const std::string_view _subScopeContent) {
            constexpr ContextTable<ast::VariableStatement> dummyContext;
            return subScopeParser(dummyContext, _subScopeContent);
        }

        sPtr<ast::Expression> expressionTreeParser(ContextTable<ast::VariableStatement> &_context,
                                                   const ast::exprTree &_expressionContent);

        // Helper methods for expression parsing
        sPtr<ast::Expression> parseAtom(ContextTable<ast::VariableStatement> &_context, std::string_view str);

        sPtr<ast::Expression> handleMemberAccess(ContextTable<ast::VariableStatement> &_context,
                                                 const std::vector<ast::exprTree> &fragments,
                                                 int splitIndex);

        static int findSplitOperator(const std::vector<ast::exprTree> &fragments);

        sPtr<ast::Expression> expressionParser(const std::string_view _expressionContent) {
            ContextTable<ast::VariableStatement> dummyContext;
            return expressionParser(dummyContext, _expressionContent);
        }

        [[nodiscard]] std::vector<ast::Type::StructDefinition> structDefParser(
            const std::vector<std::string_view> &_structContents) const;

        static ast::Type::EnumDefinition enumDefParser(std::string_view _enumContent);

        std::unordered_map<std::string, sPtr<ast::Type::CompileType> > typeMap;
        SymbolTable<ast::Type::CompileType> typeSymbolTable;
        SymbolTable<ast::VariableStatement> variableSymbolTable;
        SymbolTable<ast::FunctionDeclaration> functionSymbolTable;
        SymbolTable<ast::FunctionScope> functionScopeTable;

        [[nodiscard]] sOptional<ast::FunctionDeclaration> FindFunction(const std::string_view name) const {
            for (const auto &func: functionSymbolTable) {
                if (func->Name == name) {
                    return func;
                }
            }
            return std::nullopt;
        }

        [[nodiscard]] static sOptional<ast::VariableStatement> FindVariable(const std::string_view name,
                                                                            const ContextTable<ast::VariableStatement> &
                                                                            _context) {
            for (const auto &var: _context) {
                if (var->Name == name) {
                    return var;
                }
            }
            return std::nullopt;
        }

        [[nodiscard]] sOptional<ast::VariableStatement> FindVariable(const std::string_view name) const {
            for (const auto &var: variableSymbolTable) {
                if (var->Name == name) {
                    return var;
                }
            }
            return std::nullopt;
        }

        [[nodiscard]] sOptional<type::EnumDefinition> FindEnum(const std::string_view _name) const {
            for (const auto &type: typeSymbolTable) {
                if (const auto enumDefPtr = std::get_if<ast::Type::EnumDefinition>(&*type)) {
                    if (enumDefPtr->Name == _name) {
                        return std::make_shared<type::EnumDefinition>(*enumDefPtr);
                    }
                }
            }
            return std::nullopt;
        }

        [[nodiscard]] static std::optional<const type::BaseType *> FindBaseType(const std::string_view _name) {
            for (const auto &type: ast::Type::BaseTypes) {
                if (type.Name == _name) {
                    return &type;
                }
            }
            return std::nullopt;
        }


        [[nodiscard]] ast::Expression GetBaseTypeDefaultValue(const sPtr<type::BaseType> &_type) const;

        ast::Expression GetStructDefaultValue(const sPtr<type::StructDefinition> &_type);

        ast::Expression GetDefaultValue(const sPtr<type::CompileType> &_type);

        ast::Expression fillDefaultValue(const sPtr<type::CompileType> &_type,
                                         const std::shared_ptr<ast::Expression> &_initExpr = nullptr);

        [[nodiscard]] sOptional<type::CompileType> findType(std::string_view _typeName) const;

        explicit AbstractSyntaxTree(const std::vector<seg::TokenStatement> &tokens);

        sPtr<ast::Expression> initExprParser(std::string_view _initExpr, const sPtr<type::CompileType> &_currentType,
                                             ContextTable<ast::VariableStatement> &_context,
                                             std::string_view _realName);
    };
} // namespace mlc::parser
