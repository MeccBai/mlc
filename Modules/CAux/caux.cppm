//
// Created by Administrator on 2026/2/21.
//

module;
#include <cstdio>
export module aux;

import std;

export template<class... Args>
void ErrorPrintln(Args&&... args) {
    // 既然你是报错，直接打印出所有内容，甚至不需要手动传 format
    // 或者直接这样写：
    (std::println(stderr, "{}", std::forward<Args>(args)), ...); // 换行
}