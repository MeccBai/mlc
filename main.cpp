import std;

import Prepare;
import Token;
import Compiler;
import Parser;
import aux;
import Generator;
import Builder;

void PrintUsage() {
    std::println("Usage: mlc <input.mc> [-o <output.exe>]");
}


int main(const int argc, char* argv[]) {
    DisableOutputBuffering();

    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    const auto inputPath = std::filesystem::path(argv[1]);

    if (!std::filesystem::exists(inputPath)) {
        std::println("Error: File '{}' does not exist.", inputPath.string());
        return 1;
    }
    const mlc::builder::RequireScaner scaner(inputPath);
    const auto& buildPlan = scaner.GetBuildPlan();
    mlc::builder::Build(buildPlan);

    return 0;
}
