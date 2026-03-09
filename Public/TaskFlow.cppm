//
// Created by Administrator on 2026/3/9.
//

module;
#include <bit>
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>
#include <taskflow/algorithm/transform.hpp>
export module TaskFlow;

export namespace tf {
    using tf::Executor;
    using tf::Taskflow;
    using tf::Task;
    using tf::Subflow;
}