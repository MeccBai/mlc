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

    // 1. 跳过函数头（假设已经处理过，或者 _source 就是主体）
    // ... 原有逻辑 ...

    while (cursor < length) {
        skipSpace();
        if (cursor >= length || _source[cursor] == '}') break;

        const size_t start = cursor;
        int brace_depth = 0;
        int paren_depth = 0;

        // 识别当前片段是不是控制流（加上 else）
        auto checkAt = [&](size_t pos, std::string_view word) {
            if (pos + word.length() <= length && _source.substr(pos, word.length()) == word) {
                size_t next = pos + word.length();
                return (next == length || !std::isalnum(static_cast<unsigned char>(_source[next])));
            }
            return false;
        };

        const bool isControl = checkAt(cursor, "if") || checkAt(cursor, "for") ||
                               checkAt(cursor, "while") || checkAt(cursor, "switch") ||
                               checkAt(cursor, "else");

        while (cursor < length) {
            const char c = _source[cursor];

            // 字符串处理（略，保持你原来的）
            if (c == '"') { /* ... skip ... */ }

            if (c == '(') paren_depth++;
            else if (c == ')') paren_depth--;
            else if (c == '{') brace_depth++;
            else if (c == '}') {
                brace_depth--;
                if (brace_depth == 0 && paren_depth == 0) {
                    cursor++; // 吞掉 '}'
                    // 💥 关键：如果是 if/else 等控制流，吞完 '}' 必须立刻收刀！
                    if (isControl) break;
                    // 只有在非控制流的大括号块里，才继续往后看（比如匿名块后的其他内容）
                    continue;
                }
            }
            else if (c == ';' && brace_depth == 0 && paren_depth == 0) {
                cursor++; // 吞掉 ';'
                break; // 普通语句结束
            }

            cursor++;

            // 💡 针对 if(1) int a; 这种没有大括号的特殊情况
            if (isControl && brace_depth == 0 && paren_depth == 0 && c != ')') {
                // 如果在控制流里，且已经处理完了第一个语句（遇到了分号）
                // 此时 cursor 已经在上面被自增过了，这里直接 break
                if (_source[cursor-1] == ';') break;
            }
        }
        fragments.emplace_back(_source.substr(start, cursor - start));
    }
    return fragments;
}