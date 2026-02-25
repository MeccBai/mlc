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
            Two$ t_ptr;
            i8 result;

            // 测试：深层成员访问与赋值
            t_ptr->b->a = 100;

            // 测试：在 switch 里处理数组索引和字符
            switch(t_ptr->b->b->b->a) {
                case 'd': { // 看看字符常量还崩不崩
                    return 1;
                }
                default: {
                    return 0;
                }
            }
        }
    )";

    const std::string content = mlc::prepare::Prepare(source);
    std::print("{}",content);

    mlc::parser::AbstractSyntaxTree ast(mlc::seg::TopTokenize(content));


}
