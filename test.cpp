//
// Created by Administrator on 2026/2/21.
//

import Parser;
import std;
import Prepare;
import Compiler;

int main() {
    mlc::parser::AbstractSyntaxTree ast({});
    const std::string content = "do{int a;}while(0);";

    mlc::seg::TokenizeFunctionBody(content);

    for (const auto &token: mlc::seg::TokenizeFunctionBody(content)) {
        std::cout << token << std::endl;
    }

}
