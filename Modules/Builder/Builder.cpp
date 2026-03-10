//
// Created by Administrator on 2026/3/9.
//
module Builder;

import Parser;
import std;
import aux;
import TaskFlow;
import Compiler;
import Prepare;
import Generator;

constexpr std::uint64_t MHash(const std::string_view str) {
    std::uint64_t hash = 0xcbf29ce484222325ULL;
    for (const char c: str) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

namespace build = mlc::builder;
using astClass = mlc::parser::AbstractSyntaxTree;
using GenClass = mlc::ir::gen::IRGenerator;

void build::RequireScaner::scan(const fs::path &_entryPath, const std::size_t _currentLevel,
                                std::shared_ptr<buildNode> &_node) {
    const auto pathStr = _entryPath.string();
    _node->self = pathStr;
    const auto paths = astClass::GetImportPaths(_entryPath);
    auto nodeTemp = std::shared_ptr<buildNode>();
    if (requireMap.contains(pathStr)) {
        if (auto [oldLevel,node] = requireMap[pathStr]; _currentLevel > oldLevel) {
            if (requireGraph.contains(oldLevel)) {
                requireGraph[oldLevel].erase(pathStr);
            }
            requireMap[pathStr].first = _currentLevel;
            requireGraph[_currentLevel].insert(pathStr);
            if (requireGraph[oldLevel].empty()) {
                for (const auto &[lvl, files]: requireGraph) {
                    if (lvl > oldLevel && !files.empty()) {
                        ErrorPrintln(
                            "🚨 Circular import detected!\nLevel {} is empty, but higher levels still have files.",
                            oldLevel);
                        for (const auto &f: files) {
                            ErrorPrintln(" -> Suspicious node at level {}: {}", lvl, f);
                        }
                        std::exit(-1);
                    }
                }
            }
            const auto oldParent = node->parent.lock();
            const auto pos = std::ranges::find(oldParent->childs, node);
            _node = node;
            if (pos == oldParent->childs.end()) {
                ErrorPrintln("🚨 Circular import detected!\nLevel {} is empty, but higher levels still have files.",
                             oldLevel);
                for (const auto &[lvl, files]: requireGraph) {
                    if (lvl == oldLevel && !files.empty()) {
                        ErrorPrintln(" -> Suspicious node at level {}: {}", lvl, files);
                        std::exit(-1);
                    }
                }
            }
            oldParent->childs.erase(pos);
            return;
        }
    } else {
        requireMap[pathStr].first = _currentLevel;
        requireMap[pathStr].second = _node;
        requireGraph[_currentLevel].insert(pathStr);
    }
    for (const auto [index,path]: std::views::zip(std::views::iota(0), paths)) {
        auto childNode = std::make_shared<buildNode>();
        childNode->parent = _node;
        _node->childs.insert(childNode);
        scan(path, _currentLevel + 1, childNode);
    }
}

void build::RequireScaner::mergeBuildPlan(const std::shared_ptr<buildNode> &_node, const std::size_t _depth) {
    const bool hasChildren = !_node->childs.empty();

    for (const auto &child: _node->childs) {
        mergeBuildPlan(child, _depth + 1);
    }
    std::size_t currentLevel = 0;
    if (hasChildren) {
        for (const auto &child: _node->childs) {
            currentLevel = std::max(currentLevel, requireMap[child->self].first + 1);
        }
    }
    if (buildPlan.size() <= currentLevel) {
        buildPlan.resize(currentLevel + 1);
    }
    buildPlan[currentLevel].emplace_back(_node->self);
    requireMap[_node->self].first = currentLevel;
}

#if 0
void build::RequireScaner::scan(const importNode &_entryPath, std::size_t _currentLevel,
                                std::shared_ptr<buildNode> &_node) {
    const auto pathStr = _entryPath.path.string();
    _node->self = pathStr;
    const auto paths = _entryPath.imports;
    auto nodeTemp = std::shared_ptr<buildNode>();
    if (requireMap.contains(pathStr)) {
        if (auto [oldLevel,node] = requireMap[pathStr]; _currentLevel > oldLevel) {
            if (requireGraph.contains(oldLevel)) {
                requireGraph[oldLevel].erase(pathStr);
            }
            requireMap[pathStr].first = _currentLevel;
            requireGraph[_currentLevel].insert(pathStr);
            if (requireGraph[oldLevel].empty()) {
                for (const auto &[lvl, files]: requireGraph) {
                    if (lvl > oldLevel && !files.empty()) {
                        ErrorPrintln(
                            "🚨 Circular import detected!\nLevel {} is empty, but higher levels still have files.",
                            oldLevel);
                        for (const auto &f: files) {
                            ErrorPrintln(" -> Suspicious node at level {}: {}", lvl, f);
                        }
                        std::exit(-1);
                    }
                }
            }
            const auto oldParent = node->parent.lock();
            const auto pos = std::ranges::find(oldParent->childs, node);
            _node = node;
            oldParent->childs.erase(pos);
            return;
        }
    } else {
        requireMap[pathStr].first = _currentLevel;
        requireMap[pathStr].second = _node;
        requireGraph[_currentLevel].insert(pathStr);
    }
    for (const auto [index,path]: std::views::zip(std::views::iota(0), paths)) {
        auto childNode = std::make_shared<buildNode>();
        childNode->parent = _node;
        _node->childs.insert(childNode);
        scan(*path, _currentLevel + 1, childNode);
    }
}

build::RequireScaner::RequireScaner(importNode &_import) {
    root = std::make_shared<buildNode>();
    scan(_import, 0, root);
}

#endif
build::RequireScaner::RequireScaner(const std::filesystem::path &_entryPath) {
    root = std::make_shared<buildNode>();
    scan(_entryPath, 0, root);
    mergeBuildPlan(root, 0);
}


void build::Build(const buildPlanType &_buildPlan) {
    tf::Executor executor(8);
    tf::Taskflow taskflow;

    std::vector<std::vector<fs::path> > paths;
    const fs::path outputDir = "BuildOutput";

    paths.reserve(_buildPlan.size());
    paths.resize(_buildPlan.size());
    for (const auto &[outPaths,level]: std::views::zip(paths, _buildPlan)) {
        outPaths.reserve(level.size());
        outPaths.resize(level.size());
        taskflow.transform(level.begin(), level.end(), outPaths.begin(),
                           [outputDir](const std::string &filePath) {
                               const auto buildOutput = fs::path(filePath).parent_path() / outputDir ;
                               return BuildFile(filePath,  buildOutput);
                           });
        executor.run(taskflow).wait();
        taskflow.clear();
    }

    auto linkCommand = std::string("clang ");
    linkCommand.reserve(_buildPlan.size() * 10);

    for (const auto &levelPaths: paths) {
        for (const auto &path: levelPaths) {
            linkCommand += "\"" + path.string() + "\" ";
        }
    }
    linkCommand += "-o " + (outputDir / "output.exe").string();
    std::println("Linking with command: {}", linkCommand);
    std::system(linkCommand.c_str());
}

build::fs::path build::BuildFile(const std::string &_filePath, const fs::path &_output) {
    auto filePath = fs::path(_filePath);
    std::ifstream ifs(_filePath);
    auto outputName = _output / fs::path(_filePath).stem();
    const auto outputLLPath = outputName.replace_extension(".ll");
    const auto hashPath =  outputName.replace_extension(".hash");
    const auto content = std::string(std::istreambuf_iterator(ifs), std::istreambuf_iterator<char>());
    if (fs::exists(_output) && !fs::is_directory(_output)) {
        ErrorPrintln("🚨 Output path {} exists and is not a directory!", _output.string());
        std::exit(-1);
    }
    if (!fs::exists(_output)) {
        fs::create_directories(_output);
    }
    auto hash = MHash(content);
    if (fs::exists(hashPath)) {
        std::ifstream hashFile(hashPath);
        if (std::string hashContent((std::istreambuf_iterator(hashFile)), std::istreambuf_iterator<char>()); hashContent == std::to_string(hash) && fs::exists(outputLLPath)) {
            std::println("✅ Cache hit for '{}', skipping build.", _filePath);
            return outputLLPath;
        }
    }
    else {
        std::ofstream hashFile(hashPath);
        hashFile << hash;
        hashFile.close();
    }
    const auto source = seg::TopTokenize(prepare::Prepare(content));
    astClass ast(source, filePath.parent_path());
    auto ir = GenClass::GenerateIR(ast);


    std::ofstream ofs(outputLLPath);
    ofs << ir;
    ofs.close();
    return outputLLPath;
}
