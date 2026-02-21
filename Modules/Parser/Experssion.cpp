//
// Created by Administrator on 2026/2/21.
//

module Parser;
import std;
import aux;

namespace ast = mlc::ast;
using size_t = std::size_t;
using astClass = mlc::parser::AbstractSyntaxTree;

struct ExprFragment {
    std::string_view content;
    bool isOperator;
};

bool isOpChar(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
           c == '=' || c == '!' || c == '<' || c == '>' ||
           c == '&' || c == '|' || c == '^' || c == '~';
}

std::vector<ExprFragment> linearSplit(std::string_view expr) {
    std::vector<ExprFragment> fragments;
    int bracketLevel = 0;
    size_t start = 0;

    for (size_t i = 0; i < expr.length(); ++i) {
        const char c = expr[i];
        // 括号保护：在括号内时，无视任何运算符
        if (c == '(' || c == '{' || c == '[') { bracketLevel++; continue; }
        if (c == ')' || c == ']' || c == '}') { bracketLevel--; continue; }

        if (bracketLevel == 0) {
            // 检查是否遇到二元运算符（注意处理 ==, && 等双字符）
            if (isOpChar(c)) {
                // 1. 存入之前的操作数
                if (i > start) fragments.push_back({expr.substr(start, i - start), false});

                // 2. 存入当前的运算符（这里暂定单字符，双字符需额外判断 i+1）
                if (i + 1 < expr.length() && isOpChar(expr[i + 1])) {
                    fragments.push_back({expr.substr(i, 2), true});
                    i++;
                } else {
                    fragments.push_back({expr.substr(i, 1), true});
                }

                start = i + 1;
            }
        }
    }
    // 放入最后一个操作数
    if (start < expr.length()) fragments.push_back({expr.substr(start), false});



    return fragments;
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

    for (const auto fragments = linearSplit(_expressionContent); const auto&[content, isOperator] : fragments) {
        std::println("[{}] {}", isOperator ? "OP" : "EXPR", content);
    }

    std::exit(0);
}