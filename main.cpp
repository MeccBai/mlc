import std;

import Prepare;
import Token;
import Compiler;

int main() {
    std::ifstream file("example/example1.c");
    const auto code = std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    const auto result = mlc::prepare::Prepare(code);

    for (const auto tokens =  mlc::seg::TopTokenize(result); const auto&[type, content] : tokens) {
        std::println("Type:[{}], Content:[{}]", static_cast<int>(type), content);
        for (auto seg = mlc::seg::TokenizeFunctionBody(content); const auto& line : seg) {
             std::println("Statement:[{}]",line);
        }

    }
    return 0;
}