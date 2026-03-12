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
struct TypeComp {
    bool operator ()(const std::shared_ptr<type::CompileType> &_lhs, const std::shared_ptr<type::CompileType> &_rhs) const {
        if (!_lhs || !_rhs) return _lhs < _rhs; // 处理空指针
        const std::size_t s1 = type::GetSize(_lhs);
        const std::size_t s2 = type::GetSize(_rhs);
        if (s1 != s2) return s1 < s2;

        auto addr1 = reinterpret_cast<std::size_t>(_lhs.get());
        auto addr2 = reinterpret_cast<std::size_t>(_rhs.get());
        return addr1 < addr2;
    }
};

export namespace mlc::parser {
    class AbstractSyntaxTree {
    public:
        template<typename _type>
        using SymbolTable = std::set<std::shared_ptr<_type> >;

        template<typename _type>
        using ContextTable = std::map<std::string, std::shared_ptr<_type> >;

        template<typename _type>
        using StatementTable = std::vector<std::shared_ptr<_type> >;

        template<typename _type>
        using sPtr = std::shared_ptr<_type>;

        template<typename _type>
        using sOptional = std::optional<std::shared_ptr<_type> >;

        struct functionWarper {
            ast::FunctionDeclaration decl;
            std::string_view body;
        };

        using ExportTable = std::pair<SymbolTable<type::CompileType>, SymbolTable<ast::FunctionDeclaration> >;

    private:

        ast::FunctionScope functionDefParser(std::string_view _functionContent,bool _isExported);

        [[nodiscard]] ast::FunctionDeclaration functionDeclParser(std::string_view _functionContent,
                                                                  bool isExported = false) const;

        [[nodiscard]] sPtr<ast::VariableStatement> functionArgParser(std::string_view _argContent) const;

        [[nodiscard]] functionWarper functionDeclSpliter(std::string_view _functionHeader,
                                                         bool _isExported = false) const;


        StatementTable<ast::Statement> statementParser(std::string_view statementContent) {
            ContextTable<ast::VariableStatement> dummyContext;
            return statementParser(dummyContext, statementContent, nullptr);
        }

        StatementTable<ast::Statement> statementParser(ContextTable<ast::VariableStatement> &_context,
                                                       std::string_view statementContent,
                                                       sPtr<ast::FunctionDeclaration> _currentFunc);

        struct caseBlock {
            sPtr<ast::Expression> condition; // case 条件表达式
            StatementTable<ast::Statement> statements; // case 块内的语句
        };

        sPtr<caseBlock> caseBlockParser(ContextTable<ast::VariableStatement> &_context,
                                        std::string_view statementContent,
                                        const sPtr<ast::FunctionDeclaration> &_currentFunc);

        sPtr<ast::Statement> handleSwitchBlock(std::string_view _subScopeContent,
                                               const ContextTable<ast::VariableStatement> &_context,
                                               const sPtr<ast::FunctionDeclaration> &_currentFunc);

        sPtr<ast::Statement> handleDoWhileBlock(std::string_view _subScopeContent,
                                                const ContextTable<ast::VariableStatement> &_context,
                                                auto &_bodyToStatements, sPtr<ast::FunctionDeclaration> _currentFunc);

        sPtr<ast::Expression> expressionParser(ContextTable<ast::VariableStatement> &_context,
                                               std::string_view _expressionContent);

        sPtr<ast::Expression> constExpressionParser(std::string_view _constExpressionContent);

        StatementTable<ast::Statement> globalVariableParser(std::string_view _variableContent);

        StatementTable<ast::Statement> localVariableParser(ContextTable<ast::VariableStatement> &_context,
                                                           std::string_view _variableContent);

        StatementTable<ast::Statement> variableParser(ContextTable<ast::VariableStatement> &_context,
                                                      std::string_view _variableContent);

        sPtr<ast::Statement> subScopeParser(const ContextTable<ast::VariableStatement> &_context,
                                            std::string_view _subScopeContent,
                                            const sPtr<ast::FunctionDeclaration> &_currentFunc);

        sPtr<ast::Expression> handleSubscriptAccess(ContextTable<ast::VariableStatement> &_context,
                                                    const std::vector<ast::exprTree> &fragments,
                                                    int splitIndex);

        sPtr<ast::Statement> subScopeParser(const std::string_view _subScopeContent,
                                            const sPtr<ast::FunctionDeclaration>& _currentFunc) {
            const ContextTable<ast::VariableStatement> dummyContext{};
            return subScopeParser(dummyContext, _subScopeContent, _currentFunc);
        }

        sPtr<ast::Expression> expressionTreeParser(ContextTable<ast::VariableStatement> &_context,
                                                   const ast::exprTree &_expressionContent);

        // Helper methods for expression parsing
        sPtr<ast::Expression> parseAtom(ContextTable<ast::VariableStatement> &_context, std::string_view str);

        sPtr<ast::Expression> parseFunctionCallExpr(
            ContextTable<ast::VariableStatement> &_context,
            std::string_view str);

        StatementTable<ast::Statement> parseFunctionCallStatement(
            ContextTable<ast::VariableStatement> &_context,
            std::string_view _content);

        sPtr<ast::Expression> handleMemberAccess(ContextTable<ast::VariableStatement> &_context,
                                                 const std::vector<ast::exprTree> &fragments,
                                                 int splitIndex);

        static int findSplitOperator(const std::vector<ast::exprTree> &fragments);

        sPtr<ast::Expression> expressionParser(const std::string_view _expressionContent) {
            ContextTable<ast::VariableStatement> dummyContext;
            return expressionParser(dummyContext, _expressionContent);
        }

        [[nodiscard]] std::vector<ast::Type::StructDefinition> structDefParser(
            const std::vector<seg::TokenStatement> &_structContents) const;

        static ast::Type::EnumDefinition enumDefParser(std::string_view _enumContent,bool _isExported);

        std::unordered_map<std::string, sPtr<ast::Type::CompileType> > typeMap;
        std::set<std::shared_ptr<ast::Type::CompileType>,TypeComp> typeSymbolTable;
        SymbolTable<ast::VariableStatement> variableSymbolTable;
        SymbolTable<ast::FunctionDeclaration> functionSymbolTable;
        SymbolTable<ast::FunctionScope> functionScopeTable;

        [[nodiscard]] static std::vector<std::filesystem::path> getImportPaths(
            const std::vector<std::string> &_tokens, const std::filesystem::path &_currentPath);

        sPtr<ast::Expression> initExprParser(std::string_view _initExpr, const sPtr<type::CompileType> &_currentType,
                                             ContextTable<ast::VariableStatement> &_context,
                                             std::string_view _realName);

    public:


        [[nodiscard]] ast::Expression GetBaseTypeDefaultValue(const sPtr<type::BaseType> &_type) const;
        [[nodiscard]] ast::Expression GetBaseTypeDefaultValue(const type::BaseType &_type) const;

        ast::Expression GetStructDefaultValue(const sPtr<type::StructDefinition> &_type);
        ast::Expression GetStructDefaultValue(const type::StructDefinition &_type);

        ast::Expression GetDefaultValue(const sPtr<type::CompileType> &_type);

        ast::Expression FillDefaultValue(const sPtr<type::CompileType> &_type,
                                         const std::shared_ptr<ast::Expression> &_initExpr = nullptr);

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
            if (const auto it = _context.find(std::string(name)); it != _context.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        [[nodiscard]] sOptional<ast::VariableStatement> FindVariable(const std::string_view name) const {
            if (const auto it = variableSymbolTable.find(
                    std::make_shared<ast::VariableStatement>(name, nullptr, nullptr));
                it != variableSymbolTable.end()) {
                return *it;
            }
            return std::nullopt;
        }

        [[nodiscard]] sOptional<type::EnumDefinition> FindEnum(const std::string_view _name) const {
            for (const auto &type: typeSymbolTable) {
                if (auto *const enumDefPtr = type::GetType<ast::Type::EnumDefinition>(type)) {
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


        [[nodiscard]] sOptional<type::CompileType> FindType(std::string_view _typeName) const;

        explicit AbstractSyntaxTree(const std::vector<seg::TokenStatement> &tokens, const std::filesystem::path& _currentDirPath);

        ExportTable ExtractExportSymbols();

        static void SetSysLibPath(const std::filesystem::path& _path) {
            sysLibPath = _path;
        }

        static std::vector<std::filesystem::path> GetImportPaths(const std::filesystem::path &_importPath);

        using ASTExport = struct {
            std::set<std::shared_ptr<ast::Type::CompileType>,TypeComp> &typeSymbolTable;
            SymbolTable<ast::VariableStatement> &variableSymbolTable;
            SymbolTable<ast::FunctionDeclaration>& functionSymbolTable;
            SymbolTable<ast::FunctionScope> &functionScopeTable;
        };

        auto ExportAST() -> ASTExport {
            return {typeSymbolTable, variableSymbolTable, functionSymbolTable, functionScopeTable};
        }

        static std::filesystem::path sysLibPath;
    };
} // namespace mlc::parser
