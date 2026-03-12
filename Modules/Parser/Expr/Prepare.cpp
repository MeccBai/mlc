//
// Created by Administrator on 2026/3/6.
//
module Parser;
import std;
import aux;
import :Decl;

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
    return stripOuterBrackets(_expr.substr(1, _expr.size() - 2));
}

ast::exprTree ast::deepSplit(std::string_view expr) {
    expr = stripOuterBrackets(expr);
    if (expr.empty()) return ast::exprTree("", false);

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
        return ast::exprTree(std::move(fragments));
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
                    return ast::exprTree(std::move(fragments));
                }
                break;
            }
        }
    }

    // --- 原有逻辑：处理前缀算子 (如 &x, *p) ---
    if (isOpChar(expr[0])) {
        fragments.emplace_back(expr.substr(0, 1), true);
        fragments.push_back(deepSplit(expr.substr(1)));
        return ast::exprTree(std::move(fragments));
    }

    // 最终：真正的原子
    return ast::exprTree(expr, false);
}

void ast::dumpFragments(const exprTree &fragment, const int indent) {
    std::visit([&]<typename T>(T &&arg) {
        using Type = std::decay_t<T>;

        if constexpr (std::is_same_v<Type, std::string_view>) {
            if (fragment.isOperator) {
                std::print("<{}>", arg);
            } else {
                std::print("{}", arg);
            }
        } else if constexpr (std::is_same_v<Type, std::vector<exprTree> >) {
            std::print("[");
            for (size_t i = 0; i < arg.size(); ++i) {
                if (i < arg.size() - 1) {
                    std::print(" ");
                }
            }
            std::print("]");
        }
    }, fragment.data);
    if (indent == 0) {
        std::println("");
    }
}
