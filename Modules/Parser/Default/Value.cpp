//
// Created by Administrator on 2026/3/6.
//

module Parser;
import std;
import aux;
import :Decl;


ast::Expression astClass::GetBaseTypeDefaultValue(const sPtr<type::BaseType> &_type) const {
    if (const auto func = FindFunction(_type->Name); !func) {
        ErrorPrintln("Compiler Internal Error '{}'\n", _type->Name);
        std::exit(-1);
    }
    const std::string value = _type->Name.starts_with("f") ? "0.0" : "0";
    if (_type->Name == "i8") {
        return ast::Expression(ast::ConstValue("\\0", true));
    }
    return ast::Expression(ast::ConstValue(value));
}

ast::Expression astClass::GetStructDefaultValue(const sPtr<type::StructDefinition> &_type) {
    std::vector<std::shared_ptr<ast::Expression> > memberInits;
    for (const auto &[name, type]: _type->Members) {
        memberInits.emplace_back(ast::MakeExpression(GetDefaultValue(type)));
    }
    return ast::Expression(ast::MakeInitializerList(memberInits));
}

ast::Expression astClass::GetDefaultValue(const sPtr<type::CompileType> &_type) {
    if (const auto baseTypePtr = std::get_if<type::BaseType>(&*_type)) {
        return GetBaseTypeDefaultValue(std::make_shared<type::BaseType>(*baseTypePtr));
    }
    if (std::get_if<type::EnumDefinition>(&*_type)) {
        return ast::Expression(ast::ConstValue("0"));
    }
    if (const auto structDefPtr = std::get_if<type::StructDefinition>(&*_type)) {
        return GetStructDefaultValue(ast::Make<ast::Type::StructDefinition>(std::move(*structDefPtr)));
    }
    if (const auto arrayTypePtr = std::get_if<type::ArrayType>(&*_type)) {
        return ast::Expression(ast::MakeInitializerList(std::vector(
            arrayTypePtr->Length, ast::MakeExpression(GetDefaultValue(arrayTypePtr->BaseType)))));
    }
    if (std::get_if<type::PointerType>(&*_type)) {
        return ast::Expression(ast::ConstValue("null"));
    }
    return ast::Expression(ast::ConstValue("0"));
}

ast::Expression astClass::fillDefaultValue(const sPtr<type::CompileType> &_type,
                                           const sPtr<ast::Expression> &_initExpr) {
    if (_initExpr == nullptr) {
        if (const auto arrayPtr = type::GetType<type::ArrayType>(_type)) {
            auto defaultValues = std::views::iota(0ULL, arrayPtr->Length)
                                 | std::views::transform([&](auto) {
                                     return std::make_shared<ast::Expression>(GetDefaultValue(arrayPtr->BaseType));
                                 }) | std::ranges::to<std::vector<std::shared_ptr<ast::Expression> > >();
            return ast::Expression(ast::MakeInitializerList(std::move(defaultValues)));
        }
        if (const auto structPtr = type::GetType<type::StructDefinition>(_type)) {
            auto memberInits = structPtr->Members
                               | std::views::transform([this](const type::StructMember &member) {
                                   return std::make_shared<ast::Expression>(GetDefaultValue(member.Type));
                               }) | std::ranges::to<std::vector<std::shared_ptr<ast::Expression> > >();
            return ast::Expression(ast::MakeInitializerList(std::move(memberInits)));
        }
    }
    if (const auto list = std::get_if<sPtr<ast::InitializerList> >(&*_initExpr->Storage)) {
        size_t expectedSize = 0;
        sPtr<type::CompileType> typeToFill;
        const auto arrayPtr = type::GetType<type::ArrayType>(_type);
        const auto structPtr = type::GetType<type::StructDefinition>(_type);
        if (arrayPtr) {
            expectedSize = arrayPtr->Length;
        } else if (structPtr) {
            expectedSize = structPtr->Members.size();
        } else {
            ErrorPrintln("Error: Initializer list can only be used for arrays and structs.\n");
            std::exit(-1);
        }
        if (list->get()->Values.size() > expectedSize) {
            ErrorPrintln("Error: Too many initializers. Expected {}, got {}.\n",
                         expectedSize, list->get()->Values.size());
            std::exit(-1);
        }
        std::vector<sPtr<ast::Expression> > filledValues = list->get()->Values;
        filledValues.reserve(expectedSize);
        auto newDefaultValue = [&](auto i) {
            if (arrayPtr) {
                typeToFill = arrayPtr->BaseType;
            } else {
                typeToFill = structPtr->Members[i].Type;
            }
            return ast::MakeExpression(GetDefaultValue(typeToFill));
        };
        auto newDefaults = std::views::iota(list->get()->Values.size(), expectedSize)
                           | std::views::transform(newDefaultValue);

        std::ranges::copy(newDefaults, std::back_inserter(filledValues));
        return ast::Expression(ast::MakeInitializerList(std::move(filledValues)));
    }
    return *_initExpr;
}
