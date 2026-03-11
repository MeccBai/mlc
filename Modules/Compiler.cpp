//
// Created by Administrator on 2026/2/20.
//

module Compiler;

import std;
import Token;

namespace seg = mlc::seg;
namespace ast = mlc::ast;
using size_t = std::size_t;


inline bool isIdentifierChar(const char c) { return std::isalnum(c) || c == '_'; }

auto seg::TopTokenize(const std::string_view _source) -> std::vector<TokenStatement> {
    std::vector<TokenStatement> fragments;
    size_t cursor = 0;
    const size_t length = _source.length();

    auto skipSpaces = [&]() {
        while (cursor < length && std::isspace(static_cast<unsigned char>(_source[cursor]))) {
            cursor++;
        }
    };

    auto matchWord = [&](const std::string_view word) -> bool {
        if (cursor + word.length() > length)
            return false;
        if (_source.substr(cursor, word.length()) != word)
            return false;
        if (cursor + word.length() < length && isIdentifierChar(_source[cursor + word.length()]))
            return false;
        return true;
    };

    auto skipBlock = [&](const char open, const char close) -> bool {
        if (cursor < length && _source[cursor] == open) {
            int depth = 1;
            cursor++;
            while (cursor < length && depth > 0) {
                if (_source[cursor] == open) {
                    depth++;
                } else if (_source[cursor] == close) {
                    depth--;
                }
                cursor++;
            }
            return depth == 0;
        }
        return false;
    };

    auto findSemicolon = [&]() {
        while (cursor < length) {
            if (_source[cursor] == ';') {
                cursor++; // 消耗分号
                break;
            } else if (_source[cursor] == '{') {
                skipBlock('{', '}');
            } else {
                cursor++;
            }
        }
    };

    while (cursor < length) {
        skipSpaces();
        if (cursor >= length)
            break;

        // 1. 记录原始位置以备不测，但标记 export 剥离后的起点
        bool isExported = false;
        if (matchWord("export")) {
            isExported = true;
            cursor += 6; // 跳过 "export"
            skipSpaces();
        }

        const size_t contentStart = cursor; // 这是剔除 export 后的内容起点
        auto type = ast::GlobalStateType::VariableDeclaration; // 默认

        // 2. 依次判定主体类型
        if (matchWord("import")) {
            type = ast::GlobalStateType::ImportFile;
            findSemicolon();
        }
        else if (matchWord("struct")) {
            type = ast::GlobalStateType::StructDefinition;
            findSemicolon();
        }
        else if (matchWord("enum")) {
            type = ast::GlobalStateType::EnumDefinition;
            findSemicolon();
        }
        else {
            // 3. 进入函数 vs 变量的探测逻辑
            bool isFunction = false;
            size_t probe = cursor;
            while (probe < length) {
                if (_source[probe] == '(') {
                    isFunction = true;
                    break;
                }
                // 如果在遇到左括号前先遇到了结束符，则判定为变量
                if (_source[probe] == ';' || _source[probe] == '{') {
                    isFunction = false;
                    break;
                }
                probe++;
            }

            if (isFunction) {
                cursor = probe; // 移动到 '('
                skipBlock('(', ')');
                skipSpaces();
                if (cursor < length && _source[cursor] == ';') {
                    type = ast::GlobalStateType::FunctionDeclaration;
                    cursor++; // 消耗分号
                } else if (cursor < length && _source[cursor] == '{') {
                    type = ast::GlobalStateType::FunctionDefinition;
                    skipBlock('{', '}');
                } else {
                    // 兜底处理
                    findSemicolon();
                    type = ast::GlobalStateType::VariableDeclaration;
                }
            } else {
                // 纯变量声明
                type = ast::GlobalStateType::VariableDeclaration;
                findSemicolon();
            }
        }
        const auto source = std::string(_source.substr(contentStart, cursor - contentStart));
        fragments.emplace_back(TokenStatement{type, source, isExported});
    }
    return fragments;
}

auto seg::TokenizeFunctionBody(const std::string_view _source) -> std::vector<std::string_view> {
    std::vector<std::string_view> fragments;
    size_t cursor = 0;
    const size_t length = _source.length();

    auto skipSpace = [&]() {
        while (cursor < length && std::isspace(static_cast<unsigned char>(_source[cursor])))
            cursor++;
    };

    auto matchWord = [&](std::string_view word) {
        if (cursor + word.length() <= length && _source.substr(cursor, word.length()) == word) {
            size_t next = cursor + word.length();
            if (next == length || !std::isalnum(static_cast<unsigned char>(_source[next]))) return true;
        }
        return false;
    };

    while (cursor < length) {
        skipSpace();
        if (cursor >= length || _source[cursor] == '}') break;

        const size_t start = cursor;

        // 1. 处理带括号和块的控制流: if, while, for, switch
        if (matchWord("if") || matchWord("while") || matchWord("for") || matchWord("switch") || matchWord("else")) {
            // 扫描到块结束
            int brace_depth = 0;
            bool found_first_brace = false;

            while (cursor < length) {
                char c = _source[cursor++];
                if (c == '{') {
                    brace_depth++;
                    found_first_brace = true;
                } else if (c == '}') {
                    brace_depth--;
                    if (found_first_brace && brace_depth == 0) break; // 核心：捕获匹配的闭括号
                }
            }
        }
        // 2. 特殊处理 do { ... } while (...);
        else if (matchWord("do")) {
            while (cursor < length && _source[cursor++] != ';'); // 直接扫到末尾分号
        }
        // 3. 普通语句: int a = 1; 或 函数调用;
        else {
            int brace_depth = 0;
            while (cursor < length) {
                char c = _source[cursor++];
                if (c == '{') brace_depth++;
                else if (c == '}') brace_depth--;

                if (c == ';' && brace_depth == 0) break; // 语句结束
            }
        }

        fragments.emplace_back(_source.substr(start, cursor - start));
    }
    return fragments;
}