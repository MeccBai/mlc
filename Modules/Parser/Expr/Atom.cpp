//
// Created by Administrator on 2026/3/5.
//
module Parser;
import std;
import aux;
import :Decl;

// ========== parseAtom 辅助函数 ==========
std::vector<sPtr<ast::Expression>> parseCallArgs(
    astClass &self,
    astClass::ContextTable<ast::VariableStatement> &_context,
    const std::string_view _argsStr) {

    std::vector<sPtr<ast::Expression>> args;
    if (_argsStr.empty()) return args;

    int bracketLevel = 0;
    size_t start = 0;
    for (size_t i = 0; i < _argsStr.length(); ++i) {
        if (_argsStr[i] == '(' || _argsStr[i] == '[' || _argsStr[i] == '{') bracketLevel++;
        else if (_argsStr[i] == ')' || _argsStr[i] == ']' || _argsStr[i] == '}') bracketLevel--;
        else if (_argsStr[i] == ',' && bracketLevel == 0) {
            args.emplace_back(self.expressionParser(_context, _argsStr.substr(start, i - start)));
            start = i + 1;
        }
    }
    if (start < _argsStr.length()) {
        args.emplace_back(self.expressionParser(_context, _argsStr.substr(start)));
    }
    return args;
}

// 解析函数调用表达式
sPtr<ast::Expression> parseFunctionCallExpr(
    astClass &self,
    astClass::ContextTable<ast::VariableStatement> &_context,
    std::string_view str) {

    const auto pos = str.find('(');
    auto funcName = str.substr(0, pos);
    const auto argsStr = str.substr(pos + 1, str.length() - pos - 2);
    auto args = parseCallArgs(self, _context, argsStr);

    auto functionPtr = self.FindFunction(funcName);
    if (!functionPtr) {
        // 尝试指针类型转换函数
        if (funcName.ends_with('p')) {
            size_t level = 0;
            while (funcName.length() > level && funcName[funcName.length() - 1 - level] == 'p') {
                level++;
            }
            if (const auto optType = self.findType(funcName.substr(0, funcName.length() - level))) {
                auto typePtr = optType.value();
                const auto pType = std::make_shared<type::PointerType>(level);
                pType->Finalize(typePtr);
                typePtr = std::make_shared<type::CompileType>(*pType);
                self.functionSymbolTable.emplace_back(std::make_shared<ast::FunctionDeclaration>(
                    ast::FunctionDeclaration(std::string(funcName), typePtr, {}, true)));
                const auto isTypeConvert = const_cast<bool *>(&self.functionSymbolTable.back().get()->IsTypeConvert);
                *isTypeConvert = true;
                if (args.size() > 1) {
                    ErrorPrintln("Error: Type Convert '{}' require unique variable", funcName);
                    std::exit(-1);
                }
                if (!type::IsArrayOrPointer(args[0]->GetType())) {
                    ErrorPrintln("Error: Type Convert '{}' require array or pointer", funcName);
                }
                return std::make_shared<ast::Expression>(
                    ast::Expression(std::make_shared<ast::FunctionCall>(self.functionSymbolTable.back(), args)));
            }
        }
        ErrorPrintln("Error: Undefined function '{}'\n", funcName);
        std::exit(-1);
    }
    return std::make_shared<ast::Expression>(
        ast::Expression(std::make_shared<ast::FunctionCall>(functionPtr.value(), args)));
}

// 解析枚举值表达式
sPtr<ast::Expression> parseEnumExpr(const astClass &self, std::string_view str) {
    const auto pos = str.find("::");
    auto left = str.substr(0, pos);
    auto right = str.substr(pos + 2);

    const auto enumType = self.FindEnum(left);
    if (!enumType) {
        ErrorPrintln("MLC Syntax Error: Undefined enum type '{}' in expression: {}", left, str);
        std::exit(-1);
    }
    const auto index = enumType.value()->GetValueIndex(right);
    if (!index) {
        ErrorPrintln("MLC Syntax Error: Undefined enum value '{}' for enum type '{}' in expression: {}",
                     right, left, str);
        std::exit(-1);
    }
    return std::make_shared<ast::Expression>(ast::EnumValue(enumType.value(), index.value()));
}

// ========== 主 parseAtom ==========

sPtr<ast::Expression> astClass::parseAtom(ContextTable<ast::VariableStatement> &_context, std::string_view str) {
    // 去除空白
    while (!str.empty() && std::isspace(str.front())) str.remove_prefix(1);
    while (!str.empty() && std::isspace(str.back())) str.remove_suffix(1);

    // null 字面量
    if (str == "null") {
        return std::make_shared<ast::Expression>(ast::ConstValue("null"));
    }

    if (str.empty()) {
        ErrorPrintln("Empty expression atom");
        std::exit(-1);
    }

    // 函数调用
    if (str.back() == ')' && str.find('(') != std::string_view::npos) {
        return parseFunctionCallExpr(*this, _context, str);
    }

    // 常量 (数字/字符串/字符)
    if (std::isdigit(str[0]) || str[0] == '"' || str[0] == '\'') {
        return std::make_shared<ast::Expression>(ast::ConstValue(str));
    }

    // 变量查找 (先局部后全局)
    if (auto typeOpt = FindVariable(str, _context)) {
        return std::make_shared<ast::Expression>(typeOpt.value());
    }
    if (auto typeOpt = FindVariable(str)) {
        return std::make_shared<ast::Expression>(typeOpt.value());
    }

    // 枚举值
    if (str.find("::") != std::string_view::npos) {
        return parseEnumExpr(*this, str);
    }

    ErrorPrintln("Error: Undefined variable '{}'\n", str);
    std::exit(-1);
}

// --- 2. 链式访问处理器：贪婪收割所有 . 和 -> ---
sPtr<ast::Expression> astClass::handleMemberAccess(ContextTable<ast::VariableStatement> &_context,
                                                   const std::vector<ast::exprTree> &fragments,
                                                   const int splitIndex) {
    // 1. 获取最左侧的基准表达式 (可以是复杂表达式，不仅是原子)
    std::vector<ast::exprTree> leftFrags(fragments.begin(), fragments.begin() + splitIndex);

    // [关键修改]：左侧也必须通过解析器递归解析
    sPtr<ast::Expression> currentExpr = expressionTreeParser(
        _context, leftFrags.size() == 1 ? leftFrags[0] : ast::exprTree(leftFrags));

    // 2. 迭代处理剩余的成员访问算符和成员名 (从 splitIndex 开始)
    for (size_t i = splitIndex; i < fragments.size(); i += 2) {
        const auto op = ast::toBaseOperator(std::get<std::string_view>(fragments[i].data));

        // [边界检查]：确保碎片列表中有对应的成员名
        if (i + 1 >= fragments.size()) {
            ErrorPrintln("Invalid member access expression.");
            std::exit(-1);
        }

        const auto memberName = std::get<std::string_view>(fragments[i + 1].data);

        auto currentType = currentExpr->GetType();

        // 3. 如果是 ->，解开一层指针 (迭代解引用)
        if (op == ast::BaseOperator::Arrow) {
            if (const auto ptr = std::get_if<ast::Type::PointerType>(currentType.get())) {
                currentType = ptr->GetBaseType().lock();
            } else {
                ErrorPrintln("'->' must be used with a pointer type.");
                std::exit(-1);
            }
        }

        // 4. 结构体成员查找与类型更新
        bool found = false;
        if (const auto structDef = std::get_if<ast::Type::StructDefinition>(currentType.get())) {
            for (size_t idx = 0; idx < structDef->Members.size(); ++idx) {
                if (structDef->Members[idx].Name == memberName) {
                    // 更新 currentExpr 为新的 MemberAccess 节点
                    const auto structPtr = ast::Make<type::StructDefinition>(*structDef);
                    auto memberAccess = ast::Make<ast::MemberAccess>(ast::MemberAccess(structPtr, idx));
                    // 将本次访问封装进 CompositeExpression
                    currentExpr = MakeExpression(MakeCompExpr(
                            std::vector{currentExpr, MakeExpression(memberAccess)},
                            std::vector{op}
                        )
                    );
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            ErrorPrintln("Type '{}' has no member named '{}'", GetTypeName(*currentType), memberName);
            std::exit(-1);
        }
    }

    return currentExpr;
}

sPtr<ast::Expression> astClass::handleSubscriptAccess(ContextTable<ast::VariableStatement> &_context,
                                                      const std::vector<ast::exprTree> &fragments,
                                                      const int splitIndex) {

    const std::vector leftPart(fragments.begin(), fragments.begin() + splitIndex);
    const sPtr<ast::Expression> leftExpr = expressionTreeParser(
        _context, leftPart.size() == 1 ? leftPart[0] : ast::exprTree(leftPart));
    const auto &opFrag = fragments[splitIndex];
    const auto op = ast::toBaseOperator(std::get<std::string_view>(opFrag.data));
    const ast::exprTree &rightFragment = fragments[splitIndex + 1];
    const sPtr<ast::Expression> indexExpr = expressionTreeParser(_context, rightFragment);
    if (!type::IsIntegerType(*(indexExpr->GetType()))) {
        ErrorPrintln("Subscript operator '[]' requires an integer index.");
        std::exit(-1);
    }
    return ast::MakeExpression(
        ast::MakeCompExpr(
            std::vector{leftExpr, indexExpr},
            std::vector{op}
        )
    );
}
