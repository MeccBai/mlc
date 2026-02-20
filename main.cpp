import std;

import Process;
import Token;
import Compiler;

int main() {
    std::ifstream file("function.c");
    const std::string code = "extern int a(void);";
    const std::string result = mlc::PreProcess(code);

    auto tokens =  mlc::seg::TopTokenize(result);

    for (const auto& token : tokens) {
        std::cout << "Type: " << static_cast<int>(token.type) << ", Content: [" << token.content << "]" << std::endl;
    }

    std::cout << ": [" << result << "]" << std::endl;
    // 输出: [void main() { int a = 1; }]
    return 0;
}