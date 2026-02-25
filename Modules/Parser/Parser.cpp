//
// Created by Administrator on 2026/2/20.
//

module Parser;
import std;
import aux;

using astClass = mlc::parser::AbstractSyntaxTree;

template<typename type>
using sPtr = std::shared_ptr<type>;
namespace ast = mlc::ast;
using size_t = std::size_t;
//[if(p==0){a.x=10;}else{a.y=10;}]

template<typename type>
using sPtr = std::shared_ptr<type>;

bool isLeftExpression(const std::shared_ptr<ast::Expression> &_expression) {
    if (const auto vPtr = std::get_if<std::shared_ptr<ast::Variable> >(_expression->Storage.get()); vPtr != nullptr) {
        return true;
    }
    if (const auto fPtr = std::get_if<std::shared_ptr<ast::FunctionCall> >(_expression->Storage.get());
        fPtr != nullptr) {
        return false;
    }
    if (const auto cPtr = std::get_if<ast::ConstValue>(_expression->Storage.get()); cPtr != nullptr) {
        return false;
    }
    if (const auto compPtr = std::get_if<std::shared_ptr<ast::CompositeExpression> >(_expression->Storage.get());
        compPtr != nullptr) {
        auto &operators = compPtr->get()->Operators;
        if (!operators.empty() && !compPtr->get()->isOperatorFirst) {
            // 只有当第一个操作符是访问类操作符（. 或 ->）时，才可能是左值
            if (operators[0] == ast::BaseOperator::Dot) return true;
            if (operators[0] == ast::BaseOperator::Arrow) return true;
            if (operators[0] == ast::BaseOperator::Subscript) return true;
        }
    }
    return false;
}

std::vector<std::string_view> argSplit(const std::string_view str) {
    std::vector<std::string_view> results;
    int depth = 0;
    size_t start = 0;

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '(' || str[i] == '{' || str[i] == '[') depth++;
        else if (str[i] == ')' || str[i] == '}' || str[i] == ']') depth--;
        else if (str[i] == ',' && depth == 0) {
            results.push_back(str.substr(start, i - start));
            start = i + 1;
        }
    }
    results.push_back(str.substr(start)); // 别忘了最后一块
    if (*results.rbegin()->rbegin() == ';') {
        *results.rbegin() = results.back().substr(0, results.back().size() - 2);
    }
    if (*results.rbegin()->rbegin() == ')') {
        *results.rbegin() = results.back().substr(0, results.back().size() - 1);
    }
    return results;
}

std::vector<std::string_view> split(std::string_view str, std::string_view delimiter) {
    std::vector<std::string_view> result;
    size_t start = 0;
    while (true) {
        const size_t pos = str.find(delimiter, start);
        if (pos == std::string_view::npos) {
            result.push_back(str.substr(start));
            break;
        }
        result.push_back(str.substr(start, pos - start));
        start = pos + delimiter.length();
    }
    return result;
}

astClass::StatementTable<ast::Statement> astClass::statementParser(ContextTable<ast::VariableStatement> &_context,
                                                                   const std::string_view _statementContent) {
    if (_statementContent.starts_with("if(") ||
        _statementContent.starts_with("while(") ||
        _statementContent.starts_with("switch(") ||
        _statementContent.starts_with("do{") ||
        _statementContent.starts_with("else{") ||
        _statementContent.starts_with("{")) {
        auto sub = subScopeParser(_context, _statementContent);
        return std::vector{std::static_pointer_cast<ast::Statement>(sub)};
    }

    if (_statementContent.starts_with("return")) {
        // 检查 return 之后是否紧跟合法边界
        // 比如：return;  return (x);  return x;
        auto trim = [](std::string_view str) {
            while (!str.empty() && std::isspace(str.front())) str.remove_prefix(1);
            while (!str.empty() && std::isspace(str.back())) str.remove_suffix(1);
            return str;
        };

        bool isStandalone = (_statementContent.length() == 6 ||
                             std::isspace(_statementContent[6]) ||
                             _statementContent[6] == '(' ||
                             _statementContent[6] == ';');

        if (isStandalone) {
            auto returnBody = _statementContent.substr(6);

            // 移除首尾空格和末尾分号（如果有的话）
            // 你应该有一个统一的 Trim 函数
            auto trimmedBody = trim(returnBody);
            if (!trimmedBody.empty() && trimmedBody.back() == ';') {
                trimmedBody.remove_suffix(1);
                trimmedBody = trim(trimmedBody);
            }

            if (trimmedBody.empty()) {
                return std::vector{std::make_shared<ast::Statement>(ast::ReturnStatement(nullptr))};
            }

            return std::vector{
                std::make_shared<ast::Statement>(
                    ast::ReturnStatement(
                        expressionParser(_context, trimmedBody)
                    ))
            };
        };
    }

    if (_statementContent.starts_with("break;")) {
        return std::vector{std::make_shared<ast::Statement>(ast::BreakStatement())};
    }
    if (_statementContent.starts_with("continue;")) {
        return std::vector{std::make_shared<ast::Statement>(ast::ContinueStatement())};
    }

    if (_statementContent.find(' ') != std::string_view::npos) {
        return variableParser(_context, _statementContent);
    }
    if (auto pos = _statementContent.find('$'); pos != std::string_view::npos) {
        auto pos2 = _statementContent.find('=');
        if (pos2 > pos) {
            return variableParser(_context, _statementContent) ;
        }
    }
    if (const auto pos = _statementContent.find('='); pos != std::string_view::npos) {
        const auto left = _statementContent.substr(0, pos);
        auto right = _statementContent.substr(pos + 1, _statementContent.length() - pos - 2);
        auto leftExpr = expressionParser(_context, left);
        auto rightExpr = expressionParser(_context, right);
        auto leftType = leftExpr->GetType();
        auto rightType = rightExpr->GetType();
        ValidateType(leftType, rightType, left);
        if (!isLeftExpression(leftExpr)) {
            ErrorPrintln("{} is not a valid left-hand expression in assignment\n", left);
            std::exit(-1);
        }
        return std::vector{std::make_shared<ast::Statement>(ast::AssignStatement(leftExpr, rightExpr))};
    }
    if (_statementContent.find('(') != std::string_view::npos) {
        if (const auto pos = _statementContent.find("if("); pos == 0) {



            return std::vector{
                std::static_pointer_cast<ast::Statement>(subScopeParser(_context, _statementContent))
            };
        }
        if (const auto pos = _statementContent.find("while("); pos == 0) {

            auto sub = subScopeParser(_context, _statementContent);
            return std::vector{std::make_shared<ast::Statement>(*sub)};
        }
        if (const auto pos = _statementContent.find("switch("); pos == 0) {
            return std::vector{
                std::static_pointer_cast<ast::Statement>(subScopeParser(_context, _statementContent))
            };
        }
        if (const auto pos = _statementContent.find("do{"); pos == 0) {
            return std::vector{
                std::static_pointer_cast<ast::Statement>(subScopeParser(_context, _statementContent))
            };
        }
        const auto functionName = _statementContent.substr(0, _statementContent.find('('));
        const auto argsStr = _statementContent.substr(_statementContent.find('(') + 1,
                                                      _statementContent.length() -
                                                      functionName.length() - 1);
        std::vector<sPtr<ast::Expression> > args = argSplit(argsStr) | std::views::transform(
                                                       [this, &_context](std::string_view arg) {
                                                           return expressionParser(_context, arg);
                                                       }) | std::ranges::to<std::vector<sPtr<ast::Expression> > >();

        auto decl = std::shared_ptr<ast::FunctionDeclaration>(nullptr);

        for (const auto &function: functionSymbolTable) {
            if (function->Name == functionName) {
                if (!function->IsVarList && args.size() != function->Parameters.size()) {
                    ErrorPrintln("Error: Function '{}' expects {} arguments but {} were provided\n",
                                 functionName, function->Parameters.size(), args.size());
                    std::exit(-1);
                } else {
                    for (auto funcArgs = function->Parameters;
                         auto [exp, arg]: std::views::zip(args, funcArgs)) {
                        auto exprType = exp->GetType();
                        auto paramType = arg->VarType;
                        if (exprType == nullptr || paramType == nullptr) {
                            throw std::runtime_error("Type inference failed for argument: " + arg->Name);
                        }
                        auto getName = [](const ast::Type::CompileType &type) -> std::string {
                            return std::visit([](auto &&t) -> std::string {
                                return std::string(t.Name);
                            }, type);
                        };

                        std::string expectedName = getName(*exprType);

                        if (std::string actualName = getName(*paramType); expectedName != actualName) {
                            ErrorPrintln(
                                "Error: Argument type mismatch for parameter '{}'. Expected '{}', got '{}'\n",
                                arg->Name, actualName, expectedName);
                            std::exit(-1);
                        }

                        decl = function;
                    }
                }
                break;
            }
        }
        return std::vector{std::make_shared<ast::Statement>(
                ast::Statement(ast::FunctionCallStatement(decl, args)))};
    }
    ErrorPrintln("Invalid statement '{}'\n", _statementContent);
    std::exit(-1);
}


ast::Type::EnumDefinition astClass::enumDefParser(std::string_view _enumContent) {
    const auto pos = _enumContent.find(' ');
    const auto enumName = _enumContent.substr(pos, _enumContent.find('{') - pos);
    const auto memberStr = _enumContent.substr(_enumContent.find('{') + 1,
                                               _enumContent.rfind('}') - _enumContent.find('{') - 1);
    auto memberDefs = split(memberStr, ",");

    auto options = memberDefs | std::views::transform([](std::string_view member) {
        return std::string(member);
    }) | std::ranges::to<std::vector<std::string> >();

    return ast::Type::EnumDefinition(enumName, options);
}


astClass::AbstractSyntaxTree(const std::vector<seg::TokenStatement> &tokens) {
    std::vector<std::string_view> groups[6];

    std::ranges::for_each(tokens, [&](const auto &t) {
        groups[static_cast<size_t>(t.type)].emplace_back(t.content);
    });

    auto &functions = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDefinition)];
    auto &structs = groups[static_cast<size_t>(ast::GlobalStateType::StructDefinition)];
    auto &enums = groups[static_cast<size_t>(ast::GlobalStateType::EnumDefinition)];
    auto &varDecls = groups[static_cast<size_t>(ast::GlobalStateType::VariableDeclaration)];
    auto &funcDecls = groups[static_cast<size_t>(ast::GlobalStateType::FunctionDeclaration)];

    functionSymbolTable.reserve(ast::Type::BaseTypes.size() + functions.size() + funcDecls.size());
    for (auto type: ast::Type::BaseTypes) {
        auto typePtr = std::make_shared<ast::Type::CompileType>(type);

        typeSymbolTable.emplace_back(typePtr);
        functionSymbolTable.emplace_back(
            std::make_shared<ast::FunctionDeclaration>(
                ast::FunctionDeclaration(
                    type.Name,
                    typePtr,
                    {},
                    true
                )
            )
        );
    }
    for (auto &enumDef: enums) {
        auto enumParsed = enumDefParser(enumDef);
        auto enumPtr = std::make_shared<ast::Type::CompileType>(enumParsed);
        typeSymbolTable.emplace_back(enumPtr);
    }
    for (auto structDefs = structDefParser(structs); auto &structDef: structDefs) {
        auto structPtr = std::make_shared<ast::Type::CompileType>(structDef);
        typeSymbolTable.emplace_back(structPtr);
    }

    for (auto &decl: funcDecls) {
        auto declParsed = functionDeclParser(decl);
        functionSymbolTable.emplace_back(std::make_shared<ast::FunctionDeclaration>(declParsed));
    }

    for (auto &varDecl: varDecls) {
        auto dummyContext = ContextTable<ast::VariableStatement>{};
        for (const auto varParsed = variableParser(dummyContext, varDecl); auto &v: varParsed) {
            auto vPtr = std::get_if<ast::VariableStatement>(v.operator->());
            auto shadowPtr = std::shared_ptr<ast::VariableStatement>(v,vPtr);
            variableSymbolTable.emplace_back(shadowPtr);
        }
    }

    for (auto &func: functions) {
        const auto [decl, body] = functionDeclSpliter(func);
        functionSymbolTable.emplace_back(std::make_shared<ast::FunctionDeclaration>(decl));
    }

    for (auto &func: functions) {
        auto decl = functionDefParser(func);
        functionScopeTable.emplace_back(std::make_shared<ast::FunctionScope>(decl));
    }

    return;
}

auto astClass::findType(const std::string_view _typeName) const -> std::shared_ptr<ast::Type::CompileType> {
    for (const auto &typePtr: typeSymbolTable) {
        if (std::visit([](auto &&arg) -> std::string_view {
            return arg.Name; // 只要所有子类都有 Name，这个就能编译通过
        }, *typePtr) == _typeName) {
            return typePtr;
        }
    }
    return {};
}
