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
            i64 a[10];
            a[0]=i64(100);
        }
    )";

    const std::string content = mlc::prepare::Prepare(source);
    std::print("{}",content);

    mlc::parser::AbstractSyntaxTree ast(mlc::seg::TopTokenize(content));


}
