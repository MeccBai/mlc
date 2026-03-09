import std;

import Prepare;
import Token;
import Compiler;
import Parser;
import aux;
import Generator;


void PrintUsage() {
    std::println("Usage: mlc <input.mc> [-o <output.exe>]");
}


int main(int argc, char* argv[]) {
    DisableOutputBuffering();

    namespace fs = std::filesystem;

    const std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        PrintUsage();
        return 1;
    }

    fs::path inputPath;
    fs::path exePath = "output.exe"; // 默认输出名

    for (int i = 0; i < args.size(); ++i) {
        if (args[i] == "-o" && i + 1 < args.size()) {
            exePath = args[++i];
        } else if (inputPath.empty()) {
            inputPath = args[i];
        }
    }

    if (inputPath.empty() || !fs::exists(inputPath)) {
        std::println("❌ Error: Input file not found: {}", inputPath.string());
        return 1;
    }

    fs::path llPath = inputPath;
    llPath.replace_extension(".ll");

    std::ifstream file(inputPath);
    const auto code = std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

    auto sourceCode = mlc::prepare::Prepare(code);
    const auto result = mlc::seg::TopTokenize(sourceCode);

    mlc::parser::AbstractSyntaxTree ast(result,inputPath.parent_path());
    auto ir = mlc::ir::gen::IRGenerator::GenerateIR(ast);

    std::ofstream out(llPath);
    out << ir;
    out.close();

    //std::println("{}",ir);

    const auto compileCmd = std::format(R"(clang "{}" -o "{}" -Wno-override-module -mconsole)",
                                         llPath.string(), exePath.string());

    std::println("🚀 Compiling {} -> {}...", inputPath.string(), exePath.string());

    const auto res = std::system(compileCmd.c_str());

    if (res == 0) {
        std::println("🎉 Done!");
        fs::remove(llPath);
    } else {
        std::println("❌ Backend compilation failed.");
    }

    return res;
}