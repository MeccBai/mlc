import std;

import Prepare;
import Token;
import Compiler;
import Parser;
import aux;
import Generator;
import Builder;

void PrintUsage() {
    std::println("Usage: mlc <input.mc>");
    std::println("Don't include other requires in the command line, mlc will automatically resolve them.");
    std::println("Example: mlc src/main.mc");
    std::println("Then you will see a output executable in the src/BuildOutput folder.");
}

int main(const int argc, char* argv[]) {
    DisableOutputBuffering();

    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    const auto inputPath = std::filesystem::path(argv[1]);

    const auto sysLibPath = std::filesystem::path(argv[0]).parent_path() / "lib";

    mlc::parser::AbstractSyntaxTree::SetSysLibPath(sysLibPath);

    if (!std::filesystem::exists(inputPath)) {
        std::println("Error: File '{}' does not exist.", inputPath.string());
        return 1;
    }
    const mlc::builder::RequireScaner scaner(inputPath);
    const auto& buildPlan = scaner.GetBuildPlan();
    mlc::builder::Build(buildPlan,inputPath.parent_path());

    return 0;
}
