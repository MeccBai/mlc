//
// Created by Administrator on 2026/2/20.
//

module Compiler;

import std;
import Token;

namespace seg = mlc::seg;
namespace ast = mlc::ast;
using size_t = std::size_t;


inline bool isIdentifierChar(char c) { return std::isalnum(c) || c == '_'; }


auto seg::TopTokenize(const std::string_view _source) -> std::vector<TokenStatement> {
    std::vector<TokenStatement> fragments;
    size_t cursor = 0;
    const size_t length = _source.length();

    auto skipSpaces = [&]() {
        while (cursor < length && std::isspace(_source[cursor])) {
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
            int depth = 0;
            depth = 1;
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
                cursor++; // Consume semicolon
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

        const size_t start = cursor;
        ast::GlobalStatement type = ast::GlobalStatement::VariableDeclaration; // Default

        if (matchWord("typedef")) {
            type = ast::GlobalStatement::Typedef;
            findSemicolon();
        } else if (matchWord("struct")) {
            type = ast::GlobalStatement::StructDefinition;
            findSemicolon();
        } else if (matchWord("enum")) {
            type = ast::GlobalStatement::EnumDefinition;
            findSemicolon();
        } else {
            matchWord("static");
            matchWord("extern");
            bool isFunction = false;
            size_t probe = cursor;
            while (probe < length) {
                if (_source[probe] == '(') {
                    isFunction = true;
                    break;
                }
                if (_source[probe] == ';' || _source[probe] == '{') {
                    isFunction = false;
                    break;
                }
                probe++;
            }

            if (isFunction) {
                cursor = probe;
                if (!skipBlock('(', ')')) {
                }
                skipSpaces();
                if (cursor < length && _source[cursor] == ';') {
                    type = ast::GlobalStatement::FunctionDeclaration;
                    cursor++; // Consume ';'
                } else if (cursor < length && _source[cursor] == '{') {
                    type = ast::GlobalStatement::FunctionDefinition;
                    skipBlock('{', '}');
                } else {
                    findSemicolon();
                    type = ast::GlobalStatement::VariableDeclaration;
                }
            } else {
                type = ast::GlobalStatement::VariableDeclaration;
                findSemicolon();
            }
        }

        fragments.push_back({type, _source.substr(start, cursor - start)});
    }

    return fragments;
}

auto seg::TokenizeFunctionBody(std::string_view _source) -> std::vector<std::string_view> {
    std::vector<std::string_view> fragments;
    size_t cursor = 0;
    const size_t length = _source.length();

    auto isIdChar = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
    auto skipSpace = [&]() { while (cursor < length && std::isspace(static_cast<unsigned char>(_source[cursor]))) cursor++; };

    // 1. 切出函数头
    skipSpace();
    const size_t header_start = cursor;
    while (cursor < length && _source[cursor] != '{') cursor++;
    if (cursor < length) {
        fragments.push_back(_source.substr(header_start, cursor - header_start));
        cursor++; // 跳过 '{'
    }

    // 2. 循环切分语句块
    while (cursor < length) {
        skipSpace();
        if (cursor >= length || _source[cursor] == '}') break;

        const size_t start = cursor;
        int brace_depth = 0, paren_depth = 0;

        auto checkWord = [&](std::string_view word) {
            if (cursor + word.length() <= length && _source.substr(cursor, word.length()) == word) {
                size_t next = cursor + word.length();
                return (next == length || !isIdChar(_source[next]));
            }
            return false;
        };

        const bool is_control = checkWord("if") || checkWord("for") || checkWord("while") || checkWord("switch");

        while (cursor < length) {
            char c = _source[cursor];

            // 依旧保留字符串屏蔽，防止字符串里有 ';' 或 '}' 干扰
            if (c == '"' || c == '\'') {
                char q = c; cursor++;
                while (cursor < length && _source[cursor] != q) {
                    if (_source[cursor] == '\\') cursor++;
                    cursor++;
                }
                if (cursor < length) cursor++;
                continue;
            }

            if (c == '(') paren_depth++;
            else if (c == ')') paren_depth--;
            else if (c == '{') brace_depth++;
            else if (c == '}') {
                if (brace_depth == 0) break;
                brace_depth--;
            }

            cursor++;

            if (paren_depth == 0 && brace_depth == 0) {
                if (c == ';') break; // 普通语句
                if (is_control && c == '}') { // 控制块
                    size_t pre_else = cursor;
                    skipSpace();
                    if (checkWord("else")) continue; // 连带 else 一起切
                    cursor = pre_else;
                    break;
                }
            }
        }
        fragments.push_back(_source.substr(start, cursor - start));
    }
    return fragments;
}