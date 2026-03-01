import std;

import Prepare;
import Token;
import Compiler;
import Parser;
import aux;

int main() {
    DisableOutputBuffering();

    std::ifstream file("example/struct.mc");
    const auto code = std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    std::println("{}",code);

    auto source = mlc::prepare::Prepare(code);
    const auto result = mlc::seg::TopTokenize(source);

    mlc::parser::AbstractSyntaxTree ast(result);

    return 0;
}
