//
// Created by Administrator on 2026/2/21.
//

import Parser;
import std;
import Prepare;
import Compiler;
import aux;

int main() {
    DisableOutputBuffering() ;
    const std::string source = R"(
        struct One { i32 a; Two $b;};
        struct Two { i32 a; One $b;};
        i32 main() {
            One a=10;
            a.a = 100+5*3;
            Two$bx=@(a.b);
            bx->b=@a;
            bx->b=@a+10;
            func(a);
            return 0;
        }
        void func(One a) {}
    )";

    const std::string content = mlc::prepare::Prepare(source);
    std::print("{}",content);

    mlc::parser::AbstractSyntaxTree ast(mlc::seg::TopTokenize(content));


}
