//
// Created by Administrator on 2026/3/9.
//
module Builder;

import Parser;
import std;
import aux;
import TaskFlow;

namespace build = mlc::builder;
using astClass = mlc::parser::AbstractSyntaxTree;
using requireGraphType = std::unordered_map<std::size_t, std::unordered_set<std::string>>;

void build::RequireScaner::scan(const fs::path &_entryPath, const std::size_t _currentLevel,std::shared_ptr<buildNode>& _node) {
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
                for (const auto &[lvl, files] : requireGraph) {
                    if (lvl > oldLevel && !files.empty()) {
                        ErrorPrintln("🚨 Circular import detected!\nLevel {} is empty, but higher levels still have files.", oldLevel);
                        for (const auto &f : files) {
                            ErrorPrintln(" -> Suspicious node at level {}: {}", lvl, f);
                        }
                        std::exit(-1);
                    }
                }
            }
            const auto oldParent = node->parent.lock();
            const auto pos = std::ranges::find(oldParent->childs,node);
            _node = node;
            if (pos == oldParent->childs.end()) {
                ErrorPrintln("🚨 Circular import detected!\nLevel {} is empty, but higher levels still have files.", oldLevel);
                for (const auto &[lvl, files] : requireGraph) {
                    if (lvl == oldLevel && !files.empty()) {
                        ErrorPrintln(" -> Suspicious node at level {}: {}", lvl,files);
                        std::exit(-1);
                    }
                }
            }
            oldParent->childs.erase(pos);
            return;
        }
    }
    else {
        requireMap[pathStr].first = _currentLevel;
        requireMap[pathStr].second = _node;
        requireGraph[_currentLevel].insert(pathStr);
    }
    for (const auto [index,path] : std::views::zip(std::views::iota(0),paths)) {
        auto childNode = std::make_shared<buildNode>();
        childNode->parent = _node;
        _node->childs.insert(childNode);
        scan(path, _currentLevel + 1, childNode);
    }

}

void build::RequireScaner::mergeBuildPlan(const std::shared_ptr<buildNode>& _node, const std::size_t _depth) {
    const bool hasChildren = !_node->childs.empty();

    for (const auto& child : _node->childs) {
        mergeBuildPlan(child, _depth + 1);
    }
    std::size_t currentLevel = 0;
    if (hasChildren) {
        for (const auto& child : _node->childs) {
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
void build::RequireScaner::scan(const importNode &_entryPath,std::size_t _currentLevel,std::shared_ptr<buildNode>& _node) {
    const auto pathStr = _entryPath.path.string();
    _node->self = pathStr;
    const auto paths = _entryPath.imports ;
    auto nodeTemp = std::shared_ptr<buildNode>();
    if (requireMap.contains(pathStr)) {
        if (auto [oldLevel,node] = requireMap[pathStr]; _currentLevel > oldLevel) {
            if (requireGraph.contains(oldLevel)) {
                requireGraph[oldLevel].erase(pathStr);
            }
            requireMap[pathStr].first = _currentLevel;
            requireGraph[_currentLevel].insert(pathStr);
            if (requireGraph[oldLevel].empty()) {
                for (const auto &[lvl, files] : requireGraph) {
                    if (lvl > oldLevel && !files.empty()) {
                        ErrorPrintln("🚨 Circular import detected!\nLevel {} is empty, but higher levels still have files.", oldLevel);
                        for (const auto &f : files) {
                            ErrorPrintln(" -> Suspicious node at level {}: {}", lvl, f);
                        }
                        std::exit(-1);
                    }
                }
            }
            const auto oldParent = node->parent.lock();
            const auto pos = std::ranges::find(oldParent->childs,node);
            _node = node;
            oldParent->childs.erase(pos);
            return;
        }
    }
    else {
        requireMap[pathStr].first = _currentLevel;
        requireMap[pathStr].second = _node;
        requireGraph[_currentLevel].insert(pathStr);
    }
    for (const auto [index,path] : std::views::zip(std::views::iota(0),paths)) {
        auto childNode = std::make_shared<buildNode>();
        childNode->parent = _node;
        _node->childs.insert(childNode);
        scan(*path, _currentLevel + 1, childNode);
    }

}

build::RequireScaner::RequireScaner(importNode &_import) {
    root = std::make_shared<buildNode>();
    scan(_import,0,root);
}

#endif
build::RequireScaner::RequireScaner(const std::filesystem::path &_entryPath) {
    root = std::make_shared<buildNode>();
    scan(_entryPath, 0,root);
    mergeBuildPlan(root,0);
}


void build::Build(const build::buildPlanType &_buildPlan) {

    tf::Executor executor(8);
    tf::Taskflow taskflow;

    for (const auto &level: _buildPlan) {
        taskflow.for_each(level.begin(),level.end(), [](const std::string &filePath) {;
            BuildFile(filePath);
        });
    }


}

void build::BuildFile(const std::string &_filePath) {

}
