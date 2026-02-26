import std;

import Prepare;
import Token;
import Compiler;
import Parser;

int main() {
    std::ifstream file("example/main.mc");
    const auto code = std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    std::println("{}",code);

    auto source = mlc::prepare::Prepare(code);
    const auto result = mlc::seg::TopTokenize(source);


    mlc::parser::AbstractSyntaxTree ast(result);

    return 0;
}
