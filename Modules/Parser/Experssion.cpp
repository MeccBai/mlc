//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;

namespace ast = mlc::ast;
using size_t = std::size_t;
using astClass = mlc::parser::AbstractSyntaxTree;
using exprTree = mlc::exprTree;

std::set operators = {
    '+', '-', '*', '/', '%',
    '=', '!', '<', '>',
    '&', '|', '^', '~', '.', '@', '$'
};

bool isOpChar(const char c) {
    return operators.contains(c);
}

void mlc::ValidateType(const std::shared_ptr<ast::Type::CompileType> &targetType,
                       const std::shared_ptr<ast::Type::CompileType> &actualType,
                       const std::string_view contextInfo) {
    // 1. 安全检查：如果任何一边类型丢失，说明推导系统出了大问题
    if (!targetType || !actualType) {
        ErrorPrintln("Compiler internal error.\n", contextInfo);
        std::exit(-1);
    }

    // 2. 提取名称的辅助 Lambda
    auto getName = [](const ast::Type::CompileType &type) -> std::string {
        return std::visit([](auto &&t) -> std::string {
            // 这里假设你的 CompileType 各个分支都有 Name 成员
            return std::string(t.Name);
        }, type);
    };

    // 3. 获取类型名称进行比对
    std::string expectedName = getName(*targetType);
    std::string actualName = getName(*actualType);

    if (expectedName != actualName) {
        ErrorPrintln("Error: Type mismatch for {}. Expected '{}', got '{}'\n",
                     contextInfo, expectedName, actualName);
        std::exit(-1);
    }
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
            // --- 你的双字合并逻辑 ---
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

void mlc::dumpFragments(const exprTree &fragment, const int indent) {
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

template<typename _type>
using sPtr = std::shared_ptr<_type>;

sPtr<ast::Expression> astClass::expressionParser(ContextTable<ast::VariableStatement> &_contextTable,
                                                 const std::string_view _expressionContent) {
    if (const auto pos = _expressionContent.find('{'); pos != std::string_view::npos) {
        if (_expressionContent.back() != '}') {
            ErrorPrintln("MLC Syntax Error: Unmatched opening brace '{{' in expression: {}", _expressionContent);
            std::exit(-1);
        }
    }

    if (const auto pos = _expressionContent.find('='); pos != std::string_view::npos) {
        ErrorPrintln("MLC Syntax Error: Unexpected assignment operator '=' in expression: {}", _expressionContent);
        std::exit(-1);
    }

    const auto expressionTree = deepSplit(_expressionContent);

    dumpFragments(expressionTree);

    auto result = expressionTreeParser(_contextTable, expressionTree);

    return result;
}

sPtr<ast::Expression> astClass::constExpressionParser(const std::string_view _constExpressionContent) {
    auto dummyContext = ContextTable<ast::VariableStatement>{};
    auto tree = expressionParser(dummyContext, _constExpressionContent);
    return tree;
}

// --- 1. 原子解析：处理最基础的变量、常量、函数调用 ---
sPtr<ast::Expression> astClass::parseAtom(ContextTable<ast::VariableStatement> &_context, std::string_view str) {
    while (!str.empty() && std::isspace(str.front())) str.remove_prefix(1);
    while (!str.empty() && std::isspace(str.back())) str.remove_suffix(1);

    if (str.empty()) {
        ErrorPrintln("Empty expression atom");
        std::exit(-1);
    }

    // 函数调用处理
    if (str.back() == ')') {
        auto pos = str.find('(');
        if (pos != std::string_view::npos) {
            auto funcName = str.substr(0, pos);
            auto argsStr = str.substr(pos + 1, str.length() - pos - 2);
            std::vector<sPtr<ast::Expression> > args;
            if (!argsStr.empty()) {
                int bracketLevel = 0;
                size_t start = 0;
                for (size_t i = 0; i < argsStr.length(); ++i) {
                    if (argsStr[i] == '(' || argsStr[i] == '[' || argsStr[i] == '{') bracketLevel++;
                    else if (argsStr[i] == ')' || argsStr[i] == ']' || argsStr[i] == '}') bracketLevel--;
                    else if (argsStr[i] == ',' && bracketLevel == 0) {
                        args.push_back(expressionParser(_context, argsStr.substr(start, i - start)));
                        start = i + 1;
                    }
                }
                if (start < argsStr.length()) args.push_back(expressionParser(_context, argsStr.substr(start)));
            }

            std::shared_ptr<ast::FunctionDeclaration> functionPtr = nullptr;
            for (auto &function: functionSymbolTable) {
                if (function->Name == funcName) {
                    functionPtr = function;
                    break;
                }
            }
            return std::make_shared<ast::Expression>(
                ast::Expression(std::make_shared<ast::FunctionCall>(functionPtr, args)));
        }
    }

    // 常量处理
    if (std::isdigit(str[0]) || str[0] == '"' || str[0] == '\'')
        return std::make_shared<ast::Expression>(ast::Expression(ast::ConstValue(str)));

    // 变量处理
    for (auto &var: _context)
        if (var->Name == str)
            return std::make_shared<ast::Expression>(ast::Expression(var));
    for (auto &var: variableSymbolTable)
        if (var->Name == str)
            return std::make_shared<ast::Expression>(ast::Expression(var));

    ErrorPrintln("Error: Undefined variable '{}'\n", str);
    std::exit(-1);
}

// --- 2. 链式访问处理器：贪婪收割所有 . 和 -> ---
sPtr<ast::Expression> astClass::handleMemberAccess(ContextTable<ast::VariableStatement> &_context,
                                                   const std::vector<exprTree> &fragments,
                                                   int splitIndex) {
    // 1. 解析左侧 (必须是原子变量，因为我们禁止了嵌套)
    std::vector<exprTree> leftFrags(fragments.begin(), fragments.begin() + splitIndex);
    if (leftFrags.size() > 1) {
        ErrorPrintln("Nested member access (e.g., a.b.c) is currently disabled.");
        std::exit(-1);
    }
    sPtr<ast::Expression> leftExpr = parseAtom(_context, std::get<std::string_view>(leftFrags[0].data));

    // 2. 获取单次访问的信息
    auto opStr = std::get<std::string_view>(fragments[splitIndex].data);
    auto op = toBaseOperator(opStr);
    auto memberName = std::get<std::string_view>(fragments[splitIndex + 1].data);

    // 3. 语义检查：解引用与匹配
    auto currentType = leftExpr->GetType();

    // 如果是 ->，解开一层指针
    if (op == ast::BaseOperator::Arrow) {
        if (auto ptr = std::get_if<ast::Type::PointerType>(currentType.get())) {
            currentType = ptr->GetBaseType().lock();
        } else {
            ErrorPrintln("'->' must be used with a pointer type.");
            std::exit(-1);
        }
    }

    // 4. 结构体成员查找
    if (auto structDef = std::get_if<ast::Type::StructDefinition>(currentType.get())) {
        for (size_t idx = 0; idx < structDef->Members.size(); ++idx) {
            if (structDef->Members[idx].Name == memberName) {
                // 返回 MemberAccess 节点
                auto structPtr = std::make_shared<ast::Type::StructDefinition>(*structDef);

                // 重点：必须把 leftExpr 传给构造函数，否则后端不知道是从哪个变量偏移！
                auto memberAccess = std::make_shared<ast::MemberAccess>(structPtr, idx);
                return std::make_shared<ast::Expression>(ast::Expression(std::make_shared<ast::CompositeExpression>(
                    std::vector
                    {leftExpr, std::make_shared<ast::Expression>(ast::Expression(memberAccess))},
                    std::vector{op},
                    op == BaseOperator::AddressOf
                )));
            }
        }
    }

    ErrorPrintln("Type has no member named '{}'", memberName);
    std::exit(-1);
}

// --- 3. 调度中心：主解析器 ---
sPtr<ast::Expression> astClass::expressionTreeParser(ContextTable<ast::VariableStatement> &_context,
                                                     const exprTree &_expressionContent) {
    // --- 1. 处理原子 (叶子节点) ---
    if (auto atomPtr = std::get_if<std::string_view>(&_expressionContent.data)) {
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

    auto opStr = std::get<std::string_view>(fragments[splitIndex].data);
    auto op = toBaseOperator(opStr);

    // --- 3. 拦截成员访问 (最高优先级处理) ---
    // 这里顺应你的想法：一旦遇到 . 或 -> 且它是当前的分割点（或者手动拦截）
    if (op == ast::BaseOperator::Dot || op == ast::BaseOperator::Arrow) {
        // 注意：因为 findSplitOperator 找的是最低优先级，
        // 如果它找到了 . 说明这一层只有后缀运算。
        return handleMemberAccess(_context, fragments, splitIndex);
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
    std::vector<exprTree> leftPart(fragments.begin(), fragments.begin() + splitIndex);
    exprTree leftTree = (leftPart.size() == 1) ? leftPart[0] : exprTree(leftPart);

    // 右侧：从 splitIndex 之后到最后的所有片段
    std::vector<exprTree> rightPart(fragments.begin() + splitIndex + 1, fragments.end());
    exprTree rightTree = (rightPart.size() == 1) ? rightPart[0] : exprTree(rightPart);

    if (leftPart.empty()) {
        // 关键：不能直接返回右边，得给它套个“单目”的壳子！
        return std::make_shared<ast::Expression>(
            std::make_shared<ast::CompositeExpression>(
                std::vector{expressionTreeParser(_context, rightTree)},
                std::vector{op}
            )
        );
    }

    return std::make_shared<ast::Expression>(
        std::make_shared<ast::CompositeExpression>(
            std::vector{
                expressionTreeParser(_context,leftTree),
                expressionTreeParser(_context, rightTree)
            },
            std::vector{op}
        )
    );
}

int astClass::findSplitOperator(const std::vector<exprTree> &fragments) {
    int maxPriority = -1;
    int splitIndex = -1;

    for (int i = 0; i < (int) fragments.size(); ++i) {
        if (fragments[i].isOperator) {
            auto opStr = std::get<std::string_view>(fragments[i].data);
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
