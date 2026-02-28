//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;

namespace ast = mlc::ast;
using size_t = std::size_t;
using astClass = mlc::parser::AbstractSyntaxTree;
using exprTree = ast::exprTree;
namespace type = ast::Type;

std::set operators = {
    '+', '-', '*', '/', '%',
    '=', '!', '<', '>',
    '&', '|', '^', '~', '.', '@', '$'
};

bool isOpChar(const char c) {
    return operators.contains(c);
}


using BaseOperator = ast::BaseOperator;
const std::unordered_map<ast::BaseOperator, int> operatorPriority = {
    {BaseOperator::Dot, 1}, {BaseOperator::Arrow, 1}, {BaseOperator::Subscript, 1}, // 访问最强
    {BaseOperator::AddressOf, 2}, {BaseOperator::Dereference, 2}, // 指针紧随其后 (@, $)
    {BaseOperator::Not, 2}, {BaseOperator::BitNot, 2}, // 单目取反
    {BaseOperator::Mul, 3}, {BaseOperator::Div, 3}, {BaseOperator::Mod, 3}, // 乘除
    {BaseOperator::Add, 4}, {BaseOperator::Sub, 4}, // 加减
    {BaseOperator::ShiftLeft, 5}, {BaseOperator::ShiftRight, 5}, // 位移
    {BaseOperator::Greater, 6}, {BaseOperator::Less, 6}, // 比较
    {BaseOperator::GreaterEqual, 6}, {BaseOperator::LessEqual, 6},
    {BaseOperator::Equal, 7}, {BaseOperator::NotEqual, 7}, // 相等判定
    {BaseOperator::BitAnd, 8},
    {BaseOperator::BitXor, 9},
    {BaseOperator::BitOr, 10},
    {BaseOperator::And, 11},
    {BaseOperator::Or, 12},
};


ast::BaseOperator toBaseOperator(const std::string_view _token) {
    if (ast::BaseOperators.contains(_token)) {
        return ast::BaseOperators.at(_token);
    }
    ErrorPrintln("Invalid operator : '{}'", _token);
    std::exit(-1);
}


std::string_view stripOuterBrackets(const std::string_view _expr) {
    // 1. 基本检查：长度至少要有 2 (即 "()")
    if (_expr.size() < 2 || _expr.front() != '(' || _expr.back() != ')') {
        return _expr;
    }

    // 2. 深度检查：确保开头的 '(' 匹配的就是最后的 ')'
    int level = 0;
    for (size_t i = 0; i < _expr.size(); ++i) {
        if (_expr[i] == '(') level++;
        else if (_expr[i] == ')') level--;

        // 如果在还没到最后一个字符时，level 就归零了
        // 说明这只是并列关系，比如 (a+b)*(c+d)，不能剥壳！
        if (level == 0 && i < _expr.size() - 1) {
            return _expr;
        }
    }

    // 3. 如果能走到这里，说明首尾是真正的“套娃”关系
    // 递归调用，防止有多层括号如 "((a+b))"
    return stripOuterBrackets(_expr.substr(1, _expr.size() - 2));
}

bool isNameChar(const char c) {
    return std::isalnum(c) || c == '_';
}

exprTree deepSplit(std::string_view expr) {
    expr = stripOuterBrackets(expr);
    if (expr.empty()) return exprTree("", false);

    std::vector<exprTree> fragments;
    int bracketLevel = 0;
    size_t start = 0;
    bool foundInfixOp = false;

    for (size_t i = 0; i < expr.length(); ++i) {
        const char c = expr[i];
        if (c == '(' || c == '[' || c == '{') {
            bracketLevel++;
            continue;
        }
        if (c == ')' || c == ']' || c == '}') {
            bracketLevel--;
            continue;
        }
        if (bracketLevel == 0 && isOpChar(c)) {
            if (c == '.') {
                bool isFloat = false;
                if (i > 0 && i < expr.length() - 1) {
                    if (std::isdigit(expr[i - 1]) && std::isdigit(expr[i + 1])) {
                        isFloat = true;
                    }
                }
                if (isFloat) continue;
            }
            std::string_view op = expr.substr(i, 1);
            if (i + 1 < expr.length()) {
                if (const char next = expr[i + 1];
                    (c == '-' && next == '>') || (c == '=' && next == '=') ||
                    (c == '&' && next == '&') || (c == '|' && next == '|') ||
                    (c == '<' && next == '=') || (c == '>' && next == '=') || (c == '<' && next == '<') || (
                        c == '>' && next == '>')) {
                    op = expr.substr(i, 2);
                }
            }

            if (i == 0) continue; // 跳过前缀算子

            foundInfixOp = true;
            if (i > start) fragments.push_back(deepSplit(expr.substr(start, i - start)));
            fragments.emplace_back(op, true);
            i += (op.length() - 1);
            start = i + 1;
        }
    }

    if (foundInfixOp) {
        if (start < expr.length()) fragments.push_back(deepSplit(expr.substr(start)));
        return exprTree(std::move(fragments));
    }

    // --- 核心修复：处理后缀 [] ---
    if (expr.back() == ']') {
        // 从后往前找匹配的 '['
        size_t bLevel = 0;
        for (int i = expr.size() - 1; i >= 0; --i) {
            if (expr[i] == ']') bLevel++;
            else if (expr[i] == '[') bLevel--;

            if (bLevel == 0) {
                // 找到了对应的 [
                if (i > 0) {
                    // 如果 [ 前面还有东西，说明是 a[i] 格式
                    const std::string_view arrayName = expr.substr(0, i);
                    const std::string_view indexExpr = expr.substr(i + 1, expr.size() - i - 2);

                    fragments.push_back(deepSplit(arrayName)); // 递归处理 data
                    fragments.emplace_back("[]", true); // 存入下标算子
                    fragments.push_back(deepSplit(indexExpr)); // 递归处理 i + j
                    return exprTree(std::move(fragments));
                }
                break;
            }
        }
    }

    // --- 原有逻辑：处理前缀算子 (如 &x, *p) ---
    if (isOpChar(expr[0])) {
        fragments.emplace_back(expr.substr(0, 1), true);
        fragments.push_back(deepSplit(expr.substr(1)));
        return exprTree(std::move(fragments));
    }

    // 最终：真正的原子
    return exprTree(expr, false);
}

void mlc::ast::dumpFragments(const exprTree &fragment, const int indent) {
    std::visit([&]<typename T>(T &&arg) {
        using Type = std::decay_t<T>;

        if constexpr (std::is_same_v<Type, std::string_view>) {
            // 无论是 Op 还是 Atom，都得打印出来！
            if (fragment.isOperator) {
                std::print("<{}>", arg);
            } else {
                // 这里加个空格或者符号，确保它不和前后的内容粘在一起
                std::print("{}", arg);
            }
        } else if constexpr (std::is_same_v<Type, std::vector<exprTree> >) {
            std::print("[");
            for (size_t i = 0; i < arg.size(); ++i) {
                // 递归处理子片段
                dumpFragments(arg[i], indent + 1);

                // 只有在元素之间加空格
                if (i < arg.size() - 1) {
                    std::print(" ");
                }
            }
            std::print("]");
        }
    }, fragment.data);

    // 只有顶层调用才负责换行
    if (indent == 0) {
        std::println("");
    }
}

using VariableContext = astClass::ContextTable<ast::VariableStatement>;
using FunctionContext = astClass::ContextTable<ast::FunctionDeclaration>;

std::string operatorToString(ast::BaseOperator op) {
    for (const auto &[str, val]: ast::BaseOperators) {
        if (val == op) return std::string(str);
    }
    return "UnknownOp";
}

template<typename type>
using sPtr = std::shared_ptr<type>;

sPtr<ast::Expression> astClass::expressionParser(ContextTable<ast::VariableStatement> &_contextTable,
                                                 const std::string_view _expressionContent) {
    if (const auto pos = _expressionContent.find('{'); pos != std::string_view::npos) {
        if (_expressionContent.back() != '}') {
            ErrorPrintln("MLC Syntax Error: Unmatched opening brace '{{' in expression: {}", _expressionContent);
            std::exit(-1);
        }
    }

    if (const auto pos = _expressionContent.find('='); pos != std::string_view::npos) {
        if (_expressionContent[pos + 1] != '=') {
            ErrorPrintln("MLC Syntax Error: Unexpected assignment operator '=' in expression: {}", _expressionContent);
            std::exit(-1);
        }
    }


    const auto expressionTree = deepSplit(_expressionContent);

    ast::dumpFragments(expressionTree);

    auto result = expressionTreeParser(_contextTable, expressionTree);

    return result;
}

sPtr<ast::Expression> astClass::constExpressionParser(const std::string_view _constExpressionContent) {
    auto dummyContext = ContextTable<ast::VariableStatement>{};
    return expressionParser(dummyContext, _constExpressionContent);
}

// --- 1. 原子解析：处理最基础的变量、常量、函数调用 ---
sPtr<ast::Expression> astClass::parseAtom(ContextTable<ast::VariableStatement> &_context, std::string_view str) {
    while (!str.empty() && std::isspace(str.front())) str.remove_prefix(1);
    while (!str.empty() && std::isspace(str.back())) str.remove_suffix(1);

    if (str == "nullptr") {
        return std::make_shared<ast::Expression>(ast::Expression(ast::ConstValue("nullptr")));
    }

    if (str.empty()) {
        ErrorPrintln("Empty expression atom");
        std::exit(-1);
    }

    // 函数调用处理
    if (str.back() == ')') {
        if (const auto pos = str.find('('); pos != std::string_view::npos) {
            auto funcName = str.substr(0, pos);
            const auto argsStr = str.substr(pos + 1, str.length() - pos - 2);
            std::vector<sPtr<ast::Expression> > args;
            if (!argsStr.empty()) {
                int bracketLevel = 0;
                size_t start = 0;
                for (size_t i = 0; i < argsStr.length(); ++i) {
                    if (argsStr[i] == '(' || argsStr[i] == '[' || argsStr[i] == '{') bracketLevel++;
                    else if (argsStr[i] == ')' || argsStr[i] == ']' || argsStr[i] == '}') bracketLevel--;
                    else if (argsStr[i] == ',' && bracketLevel == 0) {
                        args.emplace_back(expressionParser(_context, argsStr.substr(start, i - start)));
                        start = i + 1;
                    }
                }
                if (start < argsStr.length()) args.emplace_back(expressionParser(_context, argsStr.substr(start)));
            }
            auto functionPtr = FindFunction(funcName);
            if (!functionPtr) {
                if (funcName.ends_with('p')) {
                    size_t level = 0;
                    while (funcName.length() > level && funcName[funcName.length() - 1 - level] == 'p') {
                        level++;
                    }
                    if (const auto optType = findType(funcName.substr(0, funcName.length() - level))) {
                        auto typePtr = optType.value();
                        const auto pType = std::make_shared<type::PointerType>(level);
                        pType->Finalize(typePtr);
                        typePtr = std::make_shared<type::CompileType>(*pType);
                        functionSymbolTable.emplace_back(std::make_shared<ast::FunctionDeclaration>(
                            ast::FunctionDeclaration(std::string(funcName), typePtr, {}, true)));
                        const auto isTypeConvert = const_cast<bool *>(&functionSymbolTable.back().get()->IsTypeConvert);
                        *isTypeConvert = true;
                        if (args.size() > 1) {
                            ErrorPrintln("Error: Type Converty '{}' require unique variable", funcName);
                            std::exit(-1);
                        }
                        if (!type::IsArrayOrPointer(args[0]->GetType())) {
                            ErrorPrintln("Error: Type Converty '{}' require array or pointer", funcName);
                        }
                        return std::make_shared<ast::Expression>(
                            ast::Expression(std::make_shared<ast::FunctionCall>(functionSymbolTable.back(), args)));
                    }
                }
                ErrorPrintln("Error: Undefined function '{}'\n", funcName);
                std::exit(-1);
            }
            return std::make_shared<ast::Expression>(
                ast::Expression(std::make_shared<ast::FunctionCall>(functionPtr.value(), args)));
        }
    }

    // 常量处理
    if (std::isdigit(str[0]) || str[0] == '"' || str[0] == '\'')
        return std::make_shared<ast::Expression>(ast::Expression(ast::ConstValue(str)));

    // 变量处理
    auto typeOpt = FindVariable(str, _context);
    if (typeOpt) {
        return std::make_shared<ast::Expression>(ast::Expression(typeOpt.value()));
    }
    typeOpt = FindVariable(str);
    if (typeOpt) {
        return std::make_shared<ast::Expression>(ast::Expression(typeOpt.value()));
    }

    if (const auto pos = str.find("::"); pos != std::string_view::npos) {
        auto left = str.substr(0, pos);
        auto right = str.substr(pos + 2);
        const auto enumType = FindEnum(left);
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
        return std::make_shared<ast::Expression>(ast::Expression(ast::EnumValue(enumType.value(), index.value())));
    }
    ErrorPrintln("Error: Undefined variable '{}'\n", str);
    std::exit(-1);
}

// --- 2. 链式访问处理器：贪婪收割所有 . 和 -> ---
sPtr<ast::Expression> astClass::handleMemberAccess(ContextTable<ast::VariableStatement> &_context,
                                                   const std::vector<exprTree> &fragments,
                                                   const int splitIndex) {
    // 1. 获取最左侧的基准表达式 (可以是复杂表达式，不仅是原子)
    std::vector<exprTree> leftFrags(fragments.begin(), fragments.begin() + splitIndex);

    // [关键修改]：左侧也必须通过解析器递归解析
    sPtr<ast::Expression> currentExpr = expressionTreeParser(
        _context, leftFrags.size() == 1 ? leftFrags[0] : exprTree(leftFrags));

    // 2. 迭代处理剩余的成员访问算符和成员名 (从 splitIndex 开始)
    for (size_t i = splitIndex; i < fragments.size(); i += 2) {
        const auto op = toBaseOperator(std::get<std::string_view>(fragments[i].data));

        // [边界检查]：确保碎片列表中有对应的成员名
        if (i + 1 >= fragments.size()) {
            ErrorPrintln("Invalid member access expression.");
            std::exit(-1);
        }

        const auto memberName = std::get<std::string_view>(fragments[i + 1].data);

        auto currentType = currentExpr->GetType();

        // 3. 如果是 ->，解开一层指针 (迭代解引用)
        if (op == BaseOperator::Arrow) {
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
                    auto structPtr = std::make_shared<ast::Type::StructDefinition>(*structDef);
                    auto memberAccess = std::make_shared<ast::MemberAccess>(structPtr, idx);

                    // 将本次访问封装进 CompositeExpression
                    currentExpr = std::make_shared<ast::Expression>(
                        std::make_shared<ast::CompositeExpression>(
                            std::vector<sPtr<ast::Expression> >{
                                currentExpr, std::make_shared<ast::Expression>(memberAccess)
                            },
                            std::vector<BaseOperator>{op}
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
                                                      const std::vector<exprTree> &fragments,
                                                      const int splitIndex) {
    const std::vector leftPart(fragments.begin(), fragments.begin() + splitIndex);
    const sPtr<ast::Expression> leftExpr = expressionTreeParser(
        _context, leftPart.size() == 1 ? leftPart[0] : exprTree(leftPart));
    const auto &opFrag = fragments[splitIndex];
    const auto op = toBaseOperator(std::get<std::string_view>(opFrag.data));
    const exprTree &rightFragment = fragments[splitIndex + 1];
    const sPtr<ast::Expression> indexExpr = expressionTreeParser(_context, rightFragment);
    if (!type::IsIntegerType(*(indexExpr->GetType()))) {
        ErrorPrintln("Subscript operator '[]' requires an integer index.");
        std::exit(-1);
    }
    return std::make_shared<ast::Expression>(
        std::make_shared<ast::CompositeExpression>(
            std::vector{leftExpr, indexExpr},
            std::vector{op}
        )
    );
}

// --- 3. 调度中心：主解析器 ---
sPtr<ast::Expression> astClass::expressionTreeParser(ContextTable<ast::VariableStatement> &_context,
                                                     const exprTree &_expressionContent) {
    // --- 1. 处理原子 (叶子节点) ---
    if (const auto atomPtr = std::get_if<std::string_view>(&_expressionContent.data)) {
        // 使用你写的 parseAtom 或之前的逻辑处理常量、变量、函数
        return parseAtom(_context, *atomPtr);
    }


    // --- 2. 处理容器 (std::vector<exprTree>) ---
    const auto fragments = std::get<std::vector<exprTree> >(_expressionContent.data);

    // 安全检查：如果容器里只有一个元素，剥壳重来
    if (fragments.size() == 1) return expressionTreeParser(_context, fragments[0]);

    // 查找当前层级结合力最弱的分割点
    int splitIndex = findSplitOperator(fragments);

    // 如果没找到操作符，说明是某种未处理的语法错误
    if (splitIndex == -1) return expressionTreeParser(_context, fragments[0]);

    const auto opStr = std::get<std::string_view>(fragments[splitIndex].data);
    const auto op = toBaseOperator(opStr);

    // --- 3. 拦截成员访问 (最高优先级处理) ---
    // 这里顺应你的想法：一旦遇到 . 或 -> 且它是当前的分割点（或者手动拦截）
    if (op == BaseOperator::Dot || op == BaseOperator::Arrow) {
        // 注意：因为 findSplitOperator 找的是最低优先级，
        // 如果它找到了 . 说明这一层只有后缀运算。
        return handleMemberAccess(_context, fragments, splitIndex);
    }

    if (op == BaseOperator::Subscript) {
        return handleSubscriptAccess(_context, fragments, splitIndex);
    }

    if (op == BaseOperator::AddressOf) {
        if (fragments.size() != 2) {
            ErrorPrintln("Address-of operator '@' must be followed by exactly one operand.");
            std::exit(-1);
        }
        return std::make_shared<ast::Expression>(ast::Expression(std::make_shared<ast::CompositeExpression>(
            std::vector{expressionTreeParser(_context, fragments[1])},
            std::vector{op}
        )));
    }

    // --- 4. 递归构建 CompositeExpression ---
    // 左侧：从开始到 splitIndex 之前的所有片段
    const std::vector leftPart(fragments.begin(), fragments.begin() + splitIndex);
    const exprTree leftTree = (leftPart.size() == 1) ? leftPart[0] : exprTree(leftPart);

    // 右侧：从 splitIndex 之后到最后的所有片段
    const std::vector rightPart(fragments.begin() + splitIndex + 1, fragments.end());
    const exprTree rightTree = (rightPart.size() == 1) ? rightPart[0] : exprTree(rightPart);

    if (leftPart.empty()) {
        // 关键：不能直接返回右边，得给它套个“单目”的壳子！
        return std::make_shared<ast::Expression>(
            std::make_shared<ast::CompositeExpression>(
                std::vector{expressionTreeParser(_context, rightTree)},
                std::vector{op}
            )
        );
    }

    const auto leftExpr = expressionTreeParser(_context, leftTree);
    const auto rightExpr = expressionTreeParser(_context, rightTree);
    type::ValidateType(leftExpr->GetType(), rightExpr->GetType(), "Binary operator type mismatch");

    return std::make_shared<ast::Expression>(
        std::make_shared<ast::CompositeExpression>(
            std::vector{
                leftExpr, rightExpr
            },
            std::vector{op}
        )
    );
}


bool isUnary(ast::BaseOperator op) {
    switch (op) {
        case BaseOperator::Sub: // -a
        case BaseOperator::AddressOf: // &a
        case BaseOperator::Dereference: // *a
        case BaseOperator::Not: // !a
        case BaseOperator::BitNot: // ~a
            return true;
        default:
            return false;
    }
}
#if 0
int astClass::findSplitOperator(const std::vector<exprTree> &fragments) {
    int maxPriority = -1;
    int splitIndex = -1;

    for (int i = 0; i < static_cast<int>(fragments.size()); ++i) {
        if (fragments[i].isOperator) {
            const auto opStr = std::get<std::string_view>(fragments[i].data);
            auto op = toBaseOperator(opStr);
            const auto priority = operatorPriority.at(op);

            // 寻找优先级最低的操作符（priority值最大的）作为树根
            if (i == 0 || fragments[i - 1].isOperator) {
                // 单目或前缀
                if (priority > maxPriority) {
                    maxPriority = priority;
                    splitIndex = i;
                }
            } else {
                // 二元
                if (priority >= maxPriority) {
                    maxPriority = priority;
                    splitIndex = i;
                }
            }
        }
    }

    return splitIndex;
}
#endif
int astClass::findSplitOperator(const std::vector<exprTree> &fragments) {
    int maxPriority = -1;
    int splitIndex = -1;

    for (int i = 0; i < static_cast<int>(fragments.size()); ++i) {
        if (fragments[i].isOperator) {
            const auto opStr = std::get<std::string_view>(fragments[i].data);
            auto op = toBaseOperator(opStr);
            const auto priority = operatorPriority.at(op);

            // 【关键修改】：实现左结合性
            // 找到优先级最高的（数值最大）的运算符作为分割点
            if (priority > maxPriority) {
                maxPriority = priority;
                splitIndex = i;
            }
            // 如果优先级相同，且它是左结合的运算符（如 []），保留之前找到的最左边的那个
            // 也就是在 if (priority > maxPriority) 中就已经包含了对左结合的处理
        }
    }
    return splitIndex;
}
