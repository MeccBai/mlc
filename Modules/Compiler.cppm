//
// Created by Administrator on 2026/2/20.
//

export module Compiler;

import std;
import Token;

namespace mlc::seg {

    export struct TokenStatement {
        ast::GlobalStateType type{};
        std::string_view content{};
    };

    export auto TopTokenize(std::string_view _source) -> std::vector<TokenStatement>;

    export auto TokenizeFunctionBody(std::string_view _source) -> std::vector<std::string_view>;


}