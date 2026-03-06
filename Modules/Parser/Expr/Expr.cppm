//
// Created by Administrator on 2026/3/6.
//

export module Parser:Expr;

import std;
import Token;
import aux;

export namespace mlc::ast {
    struct exprTree;
    using FragmentData = std::variant<std::string_view, std::vector<exprTree> >;

    struct exprTree {
        FragmentData data; // 核心内容
        bool isOperator; // 是否是操作符（对于嵌套集合，通常设为 false）

        explicit exprTree(std::string_view s, const bool op) : data(s), isOperator(op) {}

        explicit exprTree(std::vector<exprTree> v) : data(std::move(v)), isOperator(false) {}
    };

    void dumpFragments(const exprTree &fragment, int indent = 0);

    exprTree deepSplit(std::string_view expr);

   BaseOperator toBaseOperator(const std::string_view _token) {
        if (BaseOperators.contains(_token)) {
            return BaseOperators.at(_token);
        }
        ErrorPrintln("Invalid operator : '{}'", _token);
        std::exit(-1);
    }

    bool isNameChar(const char c) {
        return std::isalnum(c) || c == '_';
    }


}
