//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;

namespace ast = mlc::ast;
using size_t = std::size_t;
using astClass = mlc::parser::AbstractSyntaxTree;


bool isOpChar(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
           c == '=' || c == '!' || c == '<' || c == '>' ||
           c == '&' || c == '|' || c == '^' || c == '~' || c == '.';
}

struct exprTree;
using FragmentData = std::variant<std::string_view, std::vector<exprTree>>;

struct exprTree {
    FragmentData data; // 核心内容
    bool isOperator; // 是否是操作符（对于嵌套集合，通常设为 false）

    explicit exprTree(std::string_view s, bool op) : data(s), isOperator(op) {
    }

    explicit exprTree(std::vector<exprTree> v) : data(std::move(v)), isOperator(false) {
    }
};

std::string_view stripOuterBrackets(std::string_view expr) {
    // 1. 基本检查：长度至少要有 2 (即 "()")
    if (expr.size() < 2 || expr.front() != '(' || expr.back() != ')') {
        return expr;
    }

    // 2. 深度检查：确保开头的 '(' 匹配的就是最后的 ')'
    int level = 0;
    for (size_t i = 0; i < expr.size(); ++i) {
        if (expr[i] == '(') level++;
        else if (expr[i] == ')') level--;

        // 如果在还没到最后一个字符时，level 就归零了
        // 说明这只是并列关系，比如 (a+b)*(c+d)，不能剥壳！
        if (level == 0 && i < expr.size() - 1) {
            return expr;
        }
    }

    // 3. 如果能走到这里，说明首尾是真正的“套娃”关系
    // 递归调用，防止有多层括号如 "((a+b))"
    return stripOuterBrackets(expr.substr(1, expr.size() - 2));
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
        if (c == '(' || c == '[' || c == '{') { bracketLevel++; continue; }
        if (c == ')' || c == ']' || c == '}') { bracketLevel--; continue; }

        if (bracketLevel == 0 && isOpChar(c)) {
            // --- 你的双字合并逻辑 ---
            std::string_view op = expr.substr(i, 1);
            if (i + 1 < expr.length()) {
                char next = expr[i + 1];
                if ((c == '-' && next == '>') || (c == '=' && next == '=') ||
                    (c == '&' && next == '&') || (c == '|' && next == '|') ||
                    (c == '<' && next == '=') || (c == '>' && next == '=')) {
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
        int bLevel = 0;
        for (int i = expr.size() - 1; i >= 0; --i) {
            if (expr[i] == ']') bLevel++;
            else if (expr[i] == '[') bLevel--;

            if (bLevel == 0) { // 找到了对应的 [
                if (i > 0) { // 如果 [ 前面还有东西，说明是 a[i] 格式
                    std::string_view arrayName = expr.substr(0, i);
                    std::string_view indexExpr = expr.substr(i + 1, expr.size() - i - 2);

                    fragments.push_back(deepSplit(arrayName)); // 递归处理 data
                    fragments.emplace_back("[]", true);        // 存入下标算子
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
    std::string padding(indent * 2, ' ');

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

ast::Expression expressionParser(const exprTree & _exprTree) {
    return {};
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

    return {};
    std::exit(0);
}
