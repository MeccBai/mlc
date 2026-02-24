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
            i32 a = 1;
            do {
                i32 a = 2; // 这里的 a 应该遮蔽外层的
                i32 b = a; // b 应该等于 2
                func(a, b);
            } while(a > 0); // 这里的 a 应该绑定回最外层的 1
        }
        void func(i32 x, i32 y) {

        }
    )";

    const std::string content = mlc::prepare::Prepare(source);
    mlc::parser::AbstractSyntaxTree ast(mlc::seg::TopTokenize(content));


}
