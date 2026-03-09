//
// Created by Administrator on 2026/2/21.
//

module;
#include <cstdio>
export module aux;

import std;

export template<class... Args>
void ErrorPrintln(std::format_string<Args...> fmt, Args&&... args) {
    // 使用 std::vprint_unicode 或 std::vformat 将参数一次性填入模板
    std::println(stderr, fmt, std::forward<Args>(args)...);
}

export void ErrorPrintln(const std::string& msg) {
    std::fprintf(stderr,"%s\n", msg.c_str());
}

export void DisableOutputBuffering() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);
}