//
// Created by Administrator on 2026/2/21.
//

module;
#include <cstdio>
export module aux;

import std;

export class ErrorInfo {
    std::string path;
    explicit ErrorInfo(std::string _path) : path(std::move(_path)) {}
    void ErrorPrintln(const std::string& msg) const {
        const auto info = std::format("Error in file {} : {}",path,msg);
        std::fprintf(stderr,"%s",info.c_str());
    }
    template<class... Args>
    void ErrorPrintln(std::format_string<Args...> fmt, Args&&... args) {
        auto info = std::format("Error in file {} : ", path);
        std::println(stderr,"{} : {}",info, std::vformat(fmt, std::make_format_args(args...)));
    }

};

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