//
// Created by Administrator on 2026/2/21.
//

import Parser;
import std;
import Prepare;
import Compiler;

int main() {
    const std::string source = R"(
        struct One { i32 a; Two $b;};
        struct Two { i32 a; One $b; };
        i32 main() {
            One a;
            a.a = 100;
            return;
            return 0;
        }
        void func(i32 x, i32 y) {

        }
    )";

    const std::string content = mlc::prepare::Prepare(source);
    std::print("{}",content);

    mlc::parser::AbstractSyntaxTree ast(mlc::seg::TopTokenize(content));


}
