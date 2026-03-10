//
// Created by Administrator on 2026/3/9.
//

export module Builder;

import std;

export namespace mlc::builder {
    namespace fs = std::filesystem;

    struct importNode {
        fs::path path;
        std::vector<importNode *> imports{};
    };

    using buildPlanType = std::vector<std::vector<std::string> >;

    class RequireScaner {
        struct buildNode {
            std::string self;
            std::weak_ptr<buildNode> parent;
            std::unordered_set<std::shared_ptr<buildNode> > childs;
        };

        using requireGraphType = std::unordered_map<std::size_t, std::unordered_set<std::string> >;
        using requireMapType = std::unordered_map<std::string, std::pair<std::size_t, std::shared_ptr<buildNode> > >;


        requireGraphType requireGraph;
        requireMapType requireMap;
        buildPlanType buildPlan;

        std::shared_ptr<buildNode> root;

        void scan(const fs::path &_entryPath, std::size_t _currentLevel, std::shared_ptr<buildNode> &_node);

        void mergeBuildPlan(const std::shared_ptr<buildNode> &_node, std::size_t _depth);

    public:
        explicit RequireScaner(const fs::path &_entryPath);

        [[nodiscard]] const buildPlanType &GetBuildPlan() const {
            return buildPlan;
        }

        //explicit RequireScaner(importNode &_import);

        //void scan(const importNode &_entryPath, std::size_t _currentLevel, std::shared_ptr<buildNode> &_node);
    };


    void Build(const buildPlanType &_buildPlan);

    fs::path BuildFile(const std::string &_filePath,const fs::path & _output);
}
