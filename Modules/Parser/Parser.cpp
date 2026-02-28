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
namespace type = ast::Type;
//[if(p==0){a.x=10;}else{a.y=10;}]

template<typename type>
using sPtr = std::shared_ptr<type>;


ast::Type::EnumDefinition astClass::enumDefParser(std::string_view _enumContent) {
    const auto pos = _enumContent.find(' ');
    const auto enumName = _enumContent.substr(pos, _enumContent.find('{') - pos);
    const auto memberStr = _enumContent.substr(_enumContent.find('{') + 1,
                                               _enumContent.rfind('}') - _enumContent.find('{') - 1);
    auto memberDefs = split(memberStr, ",");

    auto options = memberDefs | std::views::transform([](const std::string_view member) {
        return std::string(member);
    }) | std::ranges::to<std::vector<std::string> >();

    auto name = enumName;
    while (!name.empty() && std::isspace(name.front())) name.remove_prefix(1);
    while (!name.empty() && std::isspace(name.back())) name.remove_suffix(1);

    return ast::Type::EnumDefinition(name, options);
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
                    type.Name,typePtr,{},
                    true,true,true
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
            auto shadowPtr = std::shared_ptr<ast::VariableStatement>(v, vPtr);
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
}

ast::Expression astClass::GetBaseTypeDefaultValue(const sPtr<type::BaseType> &_type) const {
    const auto func = FindFunction(_type->Name);
    if (!func) {
        ErrorPrintln("Compiler Internal Error '{}'\n", _type->Name);
        std::exit(-1);
    }
    const std::string value = _type->Name.starts_with("f") ? "0.0" : "0";
    auto args = std::vector{std::make_shared<ast::Expression>(ast::ConstValue(value))};
    return ast::Expression(
        std::make_shared<ast::FunctionCall>(func.value(), args)
    );
}

ast::Expression astClass::GetStructDefaultValue(const sPtr<type::StructDefinition> &_type) {
    std::vector<std::shared_ptr<ast::Expression> > memberInits;
    for (const auto &[name, type]: _type->Members) {
        memberInits.emplace_back(std::make_shared<ast::Expression>(GetDefaultValue(type)));
    }
    return ast::Expression(std::make_shared<ast::InitializerList>(memberInits));
}

ast::Expression astClass::GetDefaultValue(const sPtr<type::CompileType> &_type) {
    if (const auto baseTypePtr = std::get_if<type::BaseType>(&*_type)) {
        return GetBaseTypeDefaultValue(std::make_shared<type::BaseType>(*baseTypePtr));
    }
    if (std::get_if<type::EnumDefinition>(&*_type)) {
        return ast::Expression(ast::ConstValue("0"));
    }
    if (const auto structDefPtr = std::get_if<type::StructDefinition>(&*_type)) {
        return GetStructDefaultValue(std::make_shared<type::StructDefinition>(*structDefPtr));
    }
    if (const auto arrayTypePtr = std::get_if<type::ArrayType>(&*_type)) {
        return ast::Expression(std::make_shared<ast::InitializerList>(std::vector(
            arrayTypePtr->Length, std::make_shared<ast::Expression>(GetDefaultValue(arrayTypePtr->BaseType)))));
    }
    if (std::get_if<type::PointerType>(&*_type)) {
        return ast::Expression(ast::ConstValue("nullptr"));
    }
    return ast::Expression(ast::ConstValue("0"));
}

ast::Expression astClass::fillDefaultValue(const sPtr<type::CompileType> &_type,
                                           const sPtr<ast::Expression> &_initExpr) {
    if (_initExpr == nullptr) {
        return std::visit([this]<typename T0>(T0 &&arg) -> ast::Expression {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, type::ArrayType>) {
                auto defaultValues = std::views::iota(0ULL, arg.Length)
                                     | std::views::transform([&](auto) {
                                         return std::make_shared<ast::Expression>(GetDefaultValue(arg.BaseType));
                                     }) | std::ranges::to<std::vector<std::shared_ptr<ast::Expression> > >();
                return ast::Expression(std::make_shared<ast::InitializerList>(std::move(defaultValues)));
            } else if constexpr (std::is_same_v<T, type::StructDefinition>) {
                auto memberInits = arg.Members
                                   | std::views::transform([this](const auto &member) {
                                       return std::make_shared<ast::Expression>(GetDefaultValue(member.Type));
                                   }) | std::ranges::to<std::vector<std::shared_ptr<ast::Expression> > >();
                return ast::Expression(std::make_shared<ast::InitializerList>(std::move(memberInits)));
            }
            return ast::Expression(ast::ConstValue("0"));
        }, *_type);
    }
    if (const auto list = std::get_if<sPtr<ast::InitializerList> >(&*_initExpr->Storage)) {
        return std::visit([&]<typename T0>(T0 &&arg) -> ast::Expression {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, type::ArrayType> || std::is_same_v<T, type::StructDefinition>) {
                size_t expectedSize = 0;
                if constexpr (std::is_same_v<T, type::ArrayType>) {
                    expectedSize = arg.Length;
                } else {
                    expectedSize = arg.Members.size();
                }
                if (list->get()->Values.size() > expectedSize) {
                    ErrorPrintln("Error: Too many initializers. Expected {}, got {}.\n",
                                 expectedSize, list->get()->Values.size());
                    std::exit(-1);
                }
                std::vector<sPtr<ast::Expression> > filledValues = list->get()->Values;
                filledValues.reserve(expectedSize);
                auto newDefaultValue = [&](auto i) {
                    sPtr<type::CompileType> typeToFill;
                    if constexpr (std::is_same_v<T, type::ArrayType>) {
                        typeToFill = arg.BaseType;
                    } else {
                        typeToFill = arg.Members[i].Type;
                    }
                    return std::make_shared<ast::Expression>(GetDefaultValue(typeToFill));
                };
                auto newDefaults = std::views::iota(list->get()->Values.size(), expectedSize)
                                   | std::views::transform(newDefaultValue);
                std::ranges::copy(newDefaults, std::back_inserter(filledValues));
                return ast::Expression(std::make_shared<ast::InitializerList>(std::move(filledValues)));
            }
            return *_initExpr;
        }, *_type);
    }
    return *_initExpr;
}

auto astClass::findType(const std::string_view _typeName) const -> sOptional<ast::Type::CompileType> {
    for (const auto &typePtr: typeSymbolTable) {
        if (std::visit([](auto &&arg) -> std::string_view {
            return arg.Name;
        }, *typePtr) == _typeName) {
            return typePtr;
        }
    }
    return std::nullopt;
}
