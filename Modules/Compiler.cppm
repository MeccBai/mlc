//
// Created by Administrator on 2026/2/20.
//

export module Compiler;

import std;
import Token;
import Process;

namespace mlc::seg {

    struct TokenStatement {
        ast::GlobalStatement type{};
        std::string_view content{};
    };

    export auto TopTokenize(std::string_view _source) -> std::vector<TokenStatement>;

}