//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;

namespace ast = mlc::ast;
using size_t = std::size_t;
using astClass = mlc::parser::AbstractSyntaxTree;


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

using exprTree = astClass::AbstractSyntaxTree::exprTree;

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

void dumpFragments(const exprTree &fragment, const int indent = 0) {
    // 构造缩进字符串
    const std::string padding(indent * 2, ' ');

    // 使用 std::visit 来处理 variant 的两种可能
    std::visit([&]<typename T0>(T0 &&arg) {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, std::string_view>) {
            // --- 情况 1：原子内容 (字符串) ---
            if (fragment.isOperator) {
                std::cout << padding << "[OP] " << arg << "\n";
            } else {
                std::cout << padding << "[ATOM] " << arg << "\n";
            }
        } else if constexpr (std::is_same_v<T, std::vector<exprTree> >) {
            // --- 情况 2：嵌套集合 (vector) ---
            std::cout << padding << "{\n";
            for (const auto &subFragment: arg) {
                dumpFragments(subFragment, indent + 1); // 递归：缩进+1
            }
            std::cout << padding << "}\n";
        }
    }, fragment.data);
}

using VariableContext = astClass::ContextTable<ast::VariableStatement>;
using FunctionContext = astClass::ContextTable<ast::FunctionDeclaration>;

std::string operatorToString(ast::BaseOperator op) {
    for (const auto& [str, val] : ast::BaseOperators) {
        if (val == op) return std::string(str);
    }
    return "UnknownOp";
}

void dumpExpression(const ast::Expression& expr, int indent = 0) {
    const std::string padding(indent * 2, ' ');

    if (!expr.Storage) {
        std::cout << padding << "[Empty Expression]\n";
        return;
    }

    std::visit([&]<typename type>(type&& arg) {
        using T = std::decay_t<type>;

        if constexpr (std::is_same_v<T, ast::ConstValue>) {
            std::cout << padding << "[Const] " << arg.Value << "\n";
        }
        else if constexpr (std::is_same_v<T, ast::Variable>) {
            std::cout << padding << "[Var] " << arg.Name << "\n";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::FunctionCall>>) {
            std::cout << padding << "[FuncCall] " << arg->FunctionName << " (\n";
            for (const auto& param : arg->Arguments) {
                dumpExpression(param, indent + 1);
            }
            std::cout << padding << ")\n";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::CompositeExpression>>) {
            std::cout << padding << "[Composite]\n";
            std::cout << padding << "  Operators: ";
            for (const auto& op : arg->Operators) {
                std::cout << operatorToString(op) << " ";
            }
            std::cout << "\n";
            std::cout << padding << "  Components:\n";
            for (const auto& comp : arg->Components) {
                dumpExpression(comp, indent + 2);
            }
        }
    }, *expr.Storage);
}


ast::Expression astClass::expressionParser(ContextTable<ast::VariableStatement> &_contextTable,
                                           const std::string_view _expressionContent) {
    if (const auto pos = _expressionContent.find('{'); pos != std::string_view::npos) {
        if (_expressionContent.back() != '}') {
            ErrorPrintln("MLC Syntax Error: Unmatched opening brace '{' in expression: {}", _expressionContent);
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
    std::cout << "\n--- AST Expression Dump ---\n";
    dumpExpression(result);
    std::cout << "---------------------------\n\n";

    return result;
}

ast::Expression astClass::constExpressionParser(const std::string_view _constExpressionContent) {
    auto dummyContext = ContextTable<ast::VariableStatement>{};
    auto tree = expressionParser(dummyContext, _constExpressionContent);
    return tree;
}

ast::Expression astClass::expressionTreeParser(ContextTable<ast::VariableStatement> &_context,
                                               exprTree _expressionContent) {
    if (std::holds_alternative<std::string_view>(_expressionContent.data)) {
        auto str = std::get<std::string_view>(_expressionContent.data);

        while (!str.empty() && std::isspace(str.front())) str.remove_prefix(1);
        while (!str.empty() && std::isspace(str.back())) str.remove_suffix(1);

        if (str.empty()) {
            ErrorPrintln("Empty expression atom");
            std::exit(-1);
        }

        if (str.back() == ')') {
            auto pos = str.find('(');
            if (pos != std::string_view::npos) {
                auto funcName = str.substr(0, pos);
                auto argsStr = str.substr(pos + 1, str.length() - pos - 2);

                std::vector<ast::Expression> args;
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
                    if (start < argsStr.length()) {
                        args.push_back(expressionParser(_context, argsStr.substr(start)));
                    }
                }
                return ast::Expression(std::make_shared<ast::FunctionCall>(funcName, args));
            }
        }

        if (std::isdigit(str[0]) || str[0] == '"' || str[0] == '\'') {
            return ast::Expression(ast::ConstValue(str));
        }

        std::shared_ptr<ast::Variable> variable;
        for (auto& weakVar : _context) {
            if (auto var = weakVar.lock()) {
                if (var->Name == str) {
                    variable = var;
                    break;
                }
            }
        }
        if (variable.operator->()==nullptr) {
            for (auto& var : variableSymbolTable) {
                if (var->Name == str) {
                    variable = var;
                    break;
                }
            }
        }

        return ast::Expression(variable);
    }

    auto& fragments = std::get<std::vector<exprTree>>(_expressionContent.data);

    if (fragments.size() == 1) {
        return expressionTreeParser(_context, fragments[0]);
    }

    int maxPriority = -1;
    int splitIndex = -1;

    for (int i = 0; i < fragments.size(); ++i) {
        if (fragments[i].isOperator) {
            auto opStr = std::get<std::string_view>(fragments[i].data);
            auto op = toBaseOperator(opStr);
            const auto priority = operatorPriority.at(op);

            if (i == 0 || fragments[i-1].isOperator) {
                if (priority > maxPriority) {
                    maxPriority = priority;
                    splitIndex = i;
                }
            } else {
                if (priority >= maxPriority) {
                    maxPriority = priority;
                    splitIndex = i;
                }
            }
        }
    }

    if (splitIndex == -1) {
        return expressionTreeParser(_context, fragments[0]);
    }

    auto opStr = std::get<std::string_view>(fragments[splitIndex].data);
    auto op = toBaseOperator(opStr);

    if (splitIndex == 0) {
        std::vector<exprTree> rightFragments(fragments.begin() + 1, fragments.end());
        exprTree rightTree(rightFragments);
        if (rightFragments.size() == 1) rightTree = rightFragments[0];

        auto rightExpr = expressionTreeParser(_context, rightTree);

        return ast::Expression(std::make_shared<ast::CompositeExpression>(
            std::vector<ast::Expression>{rightExpr},
            std::vector<ast::BaseOperator>{op}
        ));
    }

    std::vector<exprTree> leftFragments(fragments.begin(), fragments.begin() + splitIndex);
    std::vector<exprTree> rightFragments(fragments.begin() + splitIndex + 1, fragments.end());

    exprTree leftTree(leftFragments);
    if (leftFragments.size() == 1) leftTree = leftFragments[0];

    exprTree rightTree(rightFragments);
    if (rightFragments.size() == 1) rightTree = rightFragments[0];

    auto leftExpr = expressionTreeParser(_context, leftTree);
    auto rightExpr = expressionTreeParser(_context, rightTree);

    return ast::Expression(std::make_shared<ast::CompositeExpression>(
        std::vector<ast::Expression>{leftExpr, rightExpr},
        std::vector<ast::BaseOperator>{op}
    ));
}
