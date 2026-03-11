//
// Created by Administrator on 2026/2/22.
//

module Parser;
import aux;
import :Decl;

std::pair<std::string_view,std::string_view> getFunctionHeader(const std::string_view _functionContent) {
    const auto bracketEndPos = _functionContent.find("){");
    const auto functionBody = _functionContent.substr(bracketEndPos + 2, _functionContent.length() - bracketEndPos - 3);
    const auto functionHeader =  _functionContent.substr(0, bracketEndPos + 1);
    return {functionHeader, functionBody};
}

ast::FunctionScope astClass::functionDefParser(const std::string_view _functionContent,const bool _isExported) {
    ContextTable<ast::VariableStatement> context;
    auto [header,body] = getFunctionHeader(_functionContent);
    auto statements = seg::TokenizeFunctionBody(body);
    auto functionDecl = ast::FunctionDeclaration(functionDeclParser(header));
    const auto functionDeclPtr = std::make_shared<ast::FunctionDeclaration>(functionDecl);
    for (const auto args = functionDecl.Parameters;
         const auto &arg: args) {
        context.insert({arg->Name,arg});
    }
    const auto statementsTemp = statements | std::views::transform(
                                    [&](const std::string_view statement) {
                                        return statementParser(context, statement,functionDeclPtr);
                                    }) | std::ranges::to<std::vector<std::vector<std::shared_ptr<
                                    ast::Statement> > > >();

    auto statementsParsed = statementsTemp | std::views::join | std::ranges::to<std::vector<std::shared_ptr<
                                ast::Statement> > >();

    if (statementsParsed.empty()) {
        ErrorPrintln("Error: function '{}' must have a return statement.\n", functionDecl.Name);
        std::exit(-1);
    }
    if (auto *returnStatement = std::get_if<ast::ReturnStatement>(statementsParsed.back().get()); !returnStatement) {
        ErrorPrintln("Error: function '{}' must have a return statement.\n", functionDecl.Name);
        std::exit(-1);
    }

    return {functionDecl, statementsParsed,_isExported};
}

sPtr<ast::Variable> normalArgParser(const std::string_view _argContent,auto findType) {
    const auto argPack = split(_argContent, " ");
    if (argPack.size() != 2) {
        ErrorPrintln("Error: Invalid argument declaration '{}'\n", _argContent);
        std::exit(-1);
    }
    const auto type = argPack[0];
    const auto name = argPack[1];
    const auto typePtr = findType(type);
    if (name.find('[') != std::string_view::npos || name.find(']') != std::string_view::npos) {
        ErrorPrintln("Error: Function Argument not allow array type '{}'\n", name);
        std::exit(-1);
    }
    if (typePtr == std::nullopt) {
        ErrorPrintln("Error: Unknown type '{}'\n", type);
        std::exit(-1);
    }
    const auto &currentType = typePtr.value();
    if (name.empty()) {
        ErrorPrintln("Error: Invalid argument declaration '{}'\n", _argContent);
        std::exit(-1);
    }
    return ast::Make<ast::Variable>(ast::Variable(std::string(name), currentType, nullptr));
}

sPtr<ast::Variable> pointerArgParser(const std::string_view _argContent,auto findType) {
    const auto first = _argContent.find_first_of('$');
    const auto last = _argContent.find_last_of('$');
    auto level = last - first + 1;
    auto typeName = _argContent.substr(0, first);
    const auto name = _argContent.substr(last + 1);
    if (const auto typePtr = findType(typeName); typePtr == std::nullopt) {
        ErrorPrintln("Error: Unknown type '{}'\n", typeName);
        std::exit(-1);
    }
    auto typePtr = findType(typeName).value();
    const auto pType = std::make_shared<type::PointerType>(level);
    pType->Finalize(typePtr);
    typePtr = std::make_shared<type::CompileType>(*pType);
    if (name.empty()) {
        ErrorPrintln("Error: Invalid argument declaration '{}'\n", _argContent);
        std::exit(-1);
    }
    return ast::Make<ast::Variable>(ast::Variable(std::string(name), typePtr, nullptr));
}

sPtr<ast::VariableStatement> astClass::functionArgParser(const std::string_view _argContent) const {
    if (_argContent.empty()) {
        ErrorPrintln("Error: Empty argument declaration\n");
        std::exit(-1);
    }
    if (_argContent.find(' ') != std::string_view::npos) {
        return normalArgParser(_argContent, [this](auto & arg){return FindType(arg);});
    }
    if (_argContent.find('$') != std::string_view::npos) {
        return pointerArgParser(_argContent, [this](auto & arg){return FindType(arg);});
    }
    ErrorPrintln("Error: Invalid argument declaration '{}'\n", _argContent);
    std::exit(-1);
}

std::pair<std::string_view,size_t> getPointerLevel(const std::string_view _typeName) {
    size_t level = 0;
    for (const auto c: _typeName) {
        if (c == '$') {
            level++;
        }
    }
    return {_typeName.substr(0,_typeName.size()-level), level};
}

ast::FunctionDeclaration astClass::functionDeclParser(
    const std::string_view _functionContent,const bool _isExported) const {
    //void func(int a,int b)
    std::string_view funcContent = _functionContent;

    bool isPointer = false;
    size_t pointerLevel = 0;
    auto returnPos = funcContent.find(' ');
    if (returnPos == std::string_view::npos) {
        returnPos = funcContent.find('$');
        isPointer = true;
    }
    std::string_view returnType = funcContent.substr(0, returnPos + (isPointer ? 1 : 0));
    if (isPointer) {
        auto [baseType, level] = getPointerLevel(returnType);
        returnType = baseType;
        pointerLevel = level;
    }
    auto returnTypePtr = FindType(returnType);
    if (!returnTypePtr) {
        ErrorPrintln("Error: Unknown return type '{}'\n", returnType);
        std::exit(-1);
    }
    if (isPointer) {
        const auto pType = std::make_shared<type::PointerType>(pointerLevel);
        pType->Finalize(returnTypePtr.value());
        returnTypePtr = std::make_shared<type::CompileType>(*pType);
    }
    else {
        returnTypePtr = std::make_shared<type::CompileType>(*returnTypePtr.value());
    }

    const auto leftBracket = funcContent.find('(');
    const auto rightBracket = funcContent.find(')');
    const auto argsList = funcContent.substr(leftBracket + 1, rightBracket - leftBracket - 1);
    const auto functionName = funcContent.substr(returnType.length() + 1, leftBracket - returnType.length() - 1);


    if (argsList == std::string_view("...")) {
        return ast::FunctionDeclaration(std::string(functionName), returnTypePtr.value(), {}, true,_isExported);
    }

    if (argsList.empty()) {
        return ast::FunctionDeclaration(std::string(functionName), returnTypePtr.value(), {},false,_isExported);
    }

    auto toArg = [this](const std::string_view arg) {
        return functionArgParser(arg);
    };

    const std::vector<sPtr<ast::VariableStatement> > args = argSplit(argsList)
    | std::views::transform(toArg) | std::ranges::to<std::vector>();

    return ast::FunctionDeclaration(std::string(functionName), returnTypePtr.value(), args,false,_isExported);
}

astClass::functionWarper astClass::functionDeclSpliter(const std::string_view _functionHeader,const bool _isExported) const {
    const auto bracketEndPos = _functionHeader.find("){");
    const auto functionBody = _functionHeader.substr(bracketEndPos + 2, _functionHeader.length() - bracketEndPos - 3);
    const auto functionHeader = _functionHeader.substr(0, bracketEndPos + 1);
    const auto functionDecl = functionDeclParser(functionHeader,_isExported);
    return {functionDecl, functionBody};
}
